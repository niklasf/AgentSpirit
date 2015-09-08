// This file is part of the AgentSpirit AgentSpeak interpreter.
// Copyright (C) 2015  Niklas Fiekas <niklas.fiekas@tu-clausthal.de>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef AS_RUNTIME_LOGICAL_CONSEQUENCE_H_
#define AS_RUNTIME_LOGICAL_CONSEQUENCE_H_

#include <iostream>
#include <memory>

#include <boost/iterator/filter_iterator.hpp>
#include <boost/variant.hpp>

#include "../agent.h"
#include "../term.h"
#include "../unification.h"
#include "environment.h"

namespace as {
namespace runtime {

/**
 * \brief Yields a dynamic list of unifiers.
 */
class UnifierGeneratorInterface {
public:
    virtual ~UnifierGeneratorInterface() { }

    /**
     * \brief Requests the next unifier.
     *
     * \return true, if another unifier is available.
     */
    virtual bool Next() = 0;

    /**
     * \brief Gets the current unifier.
     *
     * This is only valid if Next() has been called at least once and if the
     * latest call to Next() returned true. Otherwise the behaviour is
     * undefined.
     */
    virtual as::Unifier Current() const = 0;
};

template <typename T>
class UnifierGeneratorImpl : public UnifierGeneratorInterface {
public:
    UnifierGeneratorImpl(const T &term, as::Agent *agent, as::runtime::Environment *env, const as::Unifier &unifier) {
    }

    bool Next() override {
        // TODO: Assert not called?
        // TODO: Implement specializations for all valid contexts.
        return false;
    }

    as::Unifier Current() const override {
        return as::Unifier();
    }
};

class LogicalConsequenceSubstituted : public boost::static_visitor<UnifierGeneratorInterface *> {
public:
    LogicalConsequenceSubstituted(as::Agent *agent, as::runtime::Environment *env, const as::Unifier &unifier)
        : agent_(agent), env_(env), unifier_(unifier) {
    }

    template <typename T>
    UnifierGeneratorInterface *operator()(const T &term) const {
        return new UnifierGeneratorImpl<T>(term, agent_, env_, unifier_);
    }

private:
    as::Agent *agent_;
    as::runtime::Environment *env_;
    as::Unifier unifier_;
};

class IsRelevant {
public:
    IsRelevant(const std::string &functor, size_t arity)
        : functor_(functor), arity_(arity) {
    }

    bool operator()(const as::BeliefAtom &atom) {
        return atom.functor == functor_ && atom.terms.size() == arity_;
    }

private:
    std::string functor_;
    size_t arity_;
};

template <>
class UnifierGeneratorImpl<bool> : public UnifierGeneratorInterface {
public:
    UnifierGeneratorImpl(const bool &term, as::Agent *agent, as::runtime::Environment *env, const as::Unifier &unifier)
        : term_(term), unifier_(unifier) {
    }

    bool Next() override {
        if (term_) {
            term_ = false;
            return true;
        } else {
            return false;
        }
    }

    as::Unifier Current() const override {
        return unifier_;
    }

private:
    bool term_;
    as::Unifier unifier_;
};

template <>
class UnifierGeneratorImpl<as::BeliefAtom> : public UnifierGeneratorInterface {
public:
    UnifierGeneratorImpl(const BeliefAtom &term, as::Agent *agent, as::runtime::Environment *env, const as::Unifier &unifier)
        : term_(term), agent_(agent), env_(env), unifier_(unifier),
          action_(env->GetAction(agent, term, unifier)),
          beliefs_it_(IsRelevant(term.functor, term.terms.size()), agent->beliefs.begin(), agent->beliefs.end()),
          beliefs_end_(IsRelevant(term.functor, term.terms.size()), agent->beliefs.end(), agent->beliefs.end()) {
    }

    bool Next() override {
        // Execute actions.
        if (action_) {
            return action_->Next();
        }

        // Unify with belief base.
        while (beliefs_it_ != beliefs_end_) {
            as::Term term = unifier_(term_);

            current_unifier_ = unifier_;
            bool matches = as::Unify(term_, *beliefs_it_, current_unifier_);

            beliefs_it_++;

            if (matches) {
                return true;
            }
        }

        // TODO: Also apply rules.

        return false;
    }

    as::Unifier Current() const override {
        if (action_) {
            return action_->Current();
        }

        return current_unifier_;
    }

private:
    BeliefAtom term_;
    as::Agent *agent_;
    as::runtime::Environment *env_;
    as::Unifier unifier_;

    std::unique_ptr<UnifierGeneratorInterface> action_;
    boost::filter_iterator<IsRelevant, std::vector<as::BeliefAtom>::const_iterator> beliefs_it_;
    boost::filter_iterator<IsRelevant, std::vector<as::BeliefAtom>::const_iterator> beliefs_end_;
    as::Unifier current_unifier_;
};

template <>
class UnifierGeneratorImpl<as::Variable> : public UnifierGeneratorInterface {
public:
    UnifierGeneratorImpl(const as::Variable &term, as::Agent *agent, as::runtime::Environment *env, const as::Unifier &unifier)
        : term_(term),
          agent_(agent), env_(env),
          current_unifier_(unifier),
          tried_true_(false),
          beliefs_it_(agent->beliefs.begin()),
          beliefs_end_(agent->beliefs.end()) {
    }

    bool Next() override {
        if (!tried_true_) {
            current_unifier_[term_.name] = true;
            tried_true_ = true;
            return true;
        }

        while (beliefs_it_ != beliefs_end_) {
            // TODO: Execute actions.
            current_unifier_[term_.name] = *beliefs_it_;

            beliefs_it_++;
            return true;
        }

        return false;
    }

    as::Unifier Current() const override {
        return current_unifier_;
    }

private:
    as::Variable term_;
    as::Agent *agent_;
    as::runtime::Environment *env_;

    std::vector<as::BeliefAtom>::const_iterator beliefs_it_;
    std::vector<as::BeliefAtom>::const_iterator beliefs_end_;
    as::Unifier current_unifier_;
    bool tried_true_;
};

template <>
class UnifierGeneratorImpl<UnaryOp<OpNot> > : public UnifierGeneratorInterface {
public:
    UnifierGeneratorImpl(const UnaryOp<OpNot> &op, as::Agent *agent, as::runtime::Environment *env, const as::Unifier &unifier)
        : gen_(boost::apply_visitor(LogicalConsequenceSubstituted(agent, env, unifier), op.operand)),
          current_unifier_(unifier) {
    }

    bool Next() override {
        return !gen_->Next();
    }

    as::Unifier Current() const override {
        return current_unifier_;
    }

private:
    std::unique_ptr<UnifierGeneratorInterface> gen_;
    as::Unifier current_unifier_;
};

template <>
class UnifierGeneratorImpl<BinaryOp<OpOr> > : public UnifierGeneratorInterface {
public:
    UnifierGeneratorImpl(const BinaryOp<OpOr> &op, as::Agent *agent, as::runtime::Environment *env, const as::Unifier &unifier)
        : left_gen_(boost::apply_visitor(LogicalConsequenceSubstituted(agent, env, unifier), op.left)),
          left_done_(false),
          right_gen_(boost::apply_visitor(LogicalConsequenceSubstituted(agent, env, unifier), op.right)) {
    }

    bool Next() override {
        if (!left_done_) {
            if (left_gen_->Next()) {
                return true;
            } else {
                left_done_ = true;
            }
        }

        return right_gen_->Next();
    }

    as::Unifier Current() const override {
        if (!left_done_) {
            return left_gen_->Current();
        } else {
            return right_gen_->Current();
        }
    }

private:
    std::unique_ptr<UnifierGeneratorInterface> left_gen_;
    bool left_done_;

    std::unique_ptr<UnifierGeneratorInterface> right_gen_;
};

template <>
class UnifierGeneratorImpl<BinaryOp<OpAnd> > : public UnifierGeneratorInterface {
public:
    UnifierGeneratorImpl(const BinaryOp<OpAnd> &op, as::Agent *agent, as::runtime::Environment *env, const as::Unifier &unifier)
        : agent_(agent), env_(env),
          left_gen_(boost::apply_visitor(LogicalConsequenceSubstituted(agent, env, unifier), op.left)),
          right_term_(op.right),
          right_has_more_(false) {
    }

    bool Next() {
        while (right_has_more_ || left_gen_->Next()) {
            if (!right_has_more_) {
                Unifier substitution(left_gen_->Current());
                as::Term right_term_substituted = boost::apply_visitor(substitution, right_term_);
                right_gen_.reset(boost::apply_visitor(LogicalConsequenceSubstituted(agent_, env_, left_gen_->Current()), right_term_substituted));
            }

            right_has_more_ = right_gen_->Next();
            if (right_has_more_) {
                return true;
            }
        }

        return false;
    }

    as::Unifier Current() const override {
        return right_gen_->Current();
    }

private:
    as::Agent *agent_;
    as::runtime::Environment *env_;

    std::unique_ptr<UnifierGeneratorInterface> left_gen_;

    as::Term right_term_;
    std::unique_ptr<UnifierGeneratorInterface> right_gen_;
    bool right_has_more_;
};

template <>
class UnifierGeneratorImpl<BinaryOp<OpUnify> > : public UnifierGeneratorInterface {
public:
    UnifierGeneratorImpl(const BinaryOp<OpUnify> &op, as::Agent *agent, as::runtime::Environment *env, const as::Unifier &unifier)
        : op_(op), unifier_(unifier), once_(false) {
    }

    bool Next() override {
        if (!once_) {
            once_ = true;

            current_unifier_ = unifier_;
            return Unify(op_.left, op_.right, current_unifier_);
        }

        return false;
    }

    as::Unifier Current() const override {
        return current_unifier_;
    }

private:
    as::BinaryOp<OpUnify> op_;
    as::Unifier unifier_;

    as::Unifier current_unifier_;
    bool once_;
};

template <>
class UnifierGeneratorImpl<BinaryOp<OpDeconstruct> > : public UnifierGeneratorInterface {
public:
    UnifierGeneratorImpl(const BinaryOp<OpDeconstruct> &op, as::Agent *agent, as::runtime::Environment *env, const as::Unifier &unifier)
        : op_(op), current_unifier_(unifier), once_(false) {
    }

    bool Next() override {
        if (!once_) {
            once_ = true;

            // If there was more than a variable on the left side this should
            // have been resolved to OpUnify already.
            if (op_.left.which() != as::kVariable) {
                return false;
            }
            as::Variable var = boost::get<as::Variable>(op_.left);

            // Need a list on the right hand side to construct the belief atom.
            if (op_.right.which() != as::kList) {
                return false;
            }
            std::vector<as::Term> list = boost::get<std::vector<as::Term> >(op_.right);
            if (list.size() != 2) {
                return false;
            }

            // First element must be the atom that will be the functor.
            if (list[0].which() != kBeliefAtom) {
                return false;
            }
            as::BeliefAtom atom = boost::get<as::BeliefAtom>(list[0]);
            if (atom.terms.size() > 0) {
                return false;
            }

            // Second element must be the list of terms.
            if (list[1].which() != kList) {
                return false;
            }
            atom.terms = boost::get<std::vector<as::Term> >(list[1]);

            current_unifier_[var.name] = atom;
            return true;
        }

        return false;
    }

    as::Unifier Current() const override {
        return current_unifier_;
    }

private:
    as::BinaryOp<OpDeconstruct> op_;
    as::Unifier current_unifier_;
    bool once_;
};

/**
 * \brief Visitor providing the logical consequence of terms in an environment.
 *
 * An atomic term is a logical consequence if it follows from the current
 * beliefs and rules of the agent in the environment. Actions will be executed
 * to see if they succeed.
 */
class LogicalConsequence : public boost::static_visitor<UnifierGeneratorInterface *> {
public:
    LogicalConsequence(as::Agent *agent, as::runtime::Environment *env, const as::Unifier unifier)
        : impl_(agent, env, unifier), substitution_(unifier) {
    }

    template <typename T>
    UnifierGeneratorInterface *operator()(const T &term) const {
        as::Term substituted = substitution_(term);
        return boost::apply_visitor(impl_, substituted);
    }

private:
    LogicalConsequenceSubstituted impl_;
    Unifier substitution_;
};

}  // namespace runtime
}  // namespace as

#endif  // AS_RUNTIME_LOGICAL_CONSEQUENCE_H_
