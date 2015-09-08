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

#ifndef AS_UNIFY_H_
#define AS_UNIFY_H_

#include <unordered_map>

#include "term.h"

namespace as {

/**
 * \brief Anonymizes variable names.
 */
class Anonymizer : public boost::static_visitor<as::Term> {
public:
    as::Term operator()(const as::Variable &var) {
        // _ is always anonymous.
        if (var.name == "_") {
            return var;
        }

        // Check if already anonymized.
        std::unordered_map<std::string, as::Variable>::const_iterator it = mapping_.find(var.name);
        if (it != mapping_.end()) {
            return it->second;
        }

        // Generate random identifier.
        auto randchar = []() -> char {
            const char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
            const size_t max_index = sizeof(charset) - 1;
            return charset[rand() % max_index];
        };
        std::string name(64, 0);
        std::generate_n(name.begin(), 10, randchar);
        as::Variable anon_var(name);
        mapping_[var.name] = anon_var;
        return anon_var;
    }

    as::Term operator()(const std::vector<as::Term> &list) {
        std::vector<as::Term> result;
        result.reserve(list.size());

        for (const as::Term &term : list) {
            result.push_back(boost::apply_visitor(*this, term));
        }

        return result;
    }

    as::Term operator()(const as::BeliefAtom &atom) {
        as::BeliefAtom result;
        result.functor = atom.functor;
        result.terms.reserve(atom.terms.size());

        for (const as::Term &term : atom.terms) {
            result.terms.push_back(boost::apply_visitor(*this, term));
        }

        return result;
    }

    template <typename Op>
    as::Term operator()(const as::UnaryOp<Op> &op) {
        return Op()(boost::apply_visitor(*this, op.operand));
    }

    template <typename Op>
    as::Term operator()(const as::BinaryOp<Op> &op) {
        return Op()(
            boost::apply_visitor(*this, op.left),
            boost::apply_visitor(*this, op.right));
    }

    template <typename T>
    as::Term operator()(const T &val) {
        return val;
    }

private:
    std::unordered_map<std::string, as::Variable> mapping_;
};

/**
 * \brief Maps variable names to values.
 *
 * May be used as a visitor to apply the substitutions to a term.
 */
class Unifier : public std::unordered_map<std::string, as::Term>, public boost::static_visitor<as::Term> {
public:
    as::Term operator()(const as::Variable &var) const {
        std::unordered_map<std::string, as::Term>::const_iterator it = find(var.name);
        if (it != end()) {
            return boost::apply_visitor(*this, it->second);
        } else {
            return var;
        }
    }

    as::Term operator()(const std::vector<as::Term> &list) const {
        std::vector<as::Term> result;
        result.reserve(list.size());

        for (const as::Term &term : list) {
            result.push_back(boost::apply_visitor(*this, term));
        }

        return result;
    }

    as::Term operator()(const as::BeliefAtom &atom) const {
        as::BeliefAtom result;
        result.functor = atom.functor;
        result.terms.reserve(atom.terms.size());

        for (const as::Term &term : atom.terms) {
            result.terms.push_back(boost::apply_visitor(*this, term));
        }

        return result;
    }

    template <typename Op>
    as::Term operator()(const as::UnaryOp<Op> &op) const {
        return Op()(boost::apply_visitor(*this, op.operand));
    }

    template <typename Op>
    as::Term operator()(const as::BinaryOp<Op> &op) const {
        return Op()(
            boost::apply_visitor(*this, op.left),
            boost::apply_visitor(*this, op.right));
    }

    template <typename T>
    as::Term operator()(const T &val) const {
        return val;
    }

private:
    // Make private to prevent automatic conversions.
    // TODO: Revisit this decision.
    as::Term operator()(const as::Term &) const;
};

/**
 * \brief Unifies two given terms.
 *
 * \param unifier
 *   A reference to a unifier where substitutions will be stored.
 *
 * \return true, if the unification succeeded.
 */
bool Unify(const Term &left, const Term &right, Unifier &unifier);

class Unifies {
public:
    Unifies(const Term &lhs) : lhs_(lhs) {
    }

    bool operator()(const Term &rhs) {
        Unifier unifier;
        return Unify(lhs_, rhs, unifier);
    }

private:
    Term lhs_;
};

}  // namespace as

#endif  // AS_UNIFY_H_
