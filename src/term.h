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

#ifndef AS_TERM_H_
#define AS_TERM_H_

#include <vector>
#include <string>
#include <cmath>

#include <boost/variant/recursive_variant.hpp>
#include <boost/variant/get.hpp>
#include <boost/variant/apply_visitor.hpp>

namespace as {

/**
 * \brief A variable with a name.
 */
struct Variable {
    Variable() { }

    Variable(const std::string &name)
        : name(name) {
    }

    std::string name;
};

struct BeliefAtom;

template <typename Op>
struct UnaryOp;

template <typename Op>
struct BinaryOp;

struct OpPositive;
struct OpNegative;
struct OpNot;
struct OpPlus;
struct OpMinus;
struct OpPow;
struct OpMultiply;
struct OpDivide;
struct OpDiv;
struct OpMod;
struct OpAnd;
struct OpOr;
struct OpUnify;
struct OpDeconstruct;
struct OpEq;
struct OpNeq;
struct OpLt;
struct OpLte;

typedef boost::make_recursive_variant<
    // Atoms.
    bool,
    double,
    std::string,
    std::vector<boost::recursive_variant_>,
    boost::recursive_wrapper<BeliefAtom>,

    // Variables.
    Variable,

    // Unevaluated operations.
    boost::recursive_wrapper<UnaryOp<OpPositive> >,
    boost::recursive_wrapper<UnaryOp<OpNegative> >,
    boost::recursive_wrapper<UnaryOp<OpNot> >,
    boost::recursive_wrapper<BinaryOp<OpPlus> >,
    boost::recursive_wrapper<BinaryOp<OpMinus> >,
    boost::recursive_wrapper<BinaryOp<OpPow> >,
    boost::recursive_wrapper<BinaryOp<OpMultiply> >,
    boost::recursive_wrapper<BinaryOp<OpDivide> >,
    boost::recursive_wrapper<BinaryOp<OpDiv> >,
    boost::recursive_wrapper<BinaryOp<OpMod> >,
    boost::recursive_wrapper<BinaryOp<OpAnd> >,
    boost::recursive_wrapper<BinaryOp<OpOr> >,
    boost::recursive_wrapper<BinaryOp<OpUnify> >,
    boost::recursive_wrapper<BinaryOp<OpDeconstruct> >,
    boost::recursive_wrapper<BinaryOp<OpEq> >,
    boost::recursive_wrapper<BinaryOp<OpNeq> >,
    boost::recursive_wrapper<BinaryOp<OpLt> >,
    boost::recursive_wrapper<BinaryOp<OpLte> >
  >::type Term;

enum TermType {
    kBool,
    kDouble,
    kString,
    kList,
    kBeliefAtom,
    kVariable,
    kPositive,
    kNegative,
    kNot,
    kPlus,
    kMinus,
    kPow,
    kMultiply,
    kDivide,
    kDiv,
    kMod,
    kAnd,
    kOr,
    kUnify,
    kDeconstruct,
    kEq,
    kNeq,
    kLt,
    kLte
};

/**
 * \brief A belief atom with a functor and a list of terms.
 */
struct BeliefAtom {
    BeliefAtom() { }

    BeliefAtom(const std::string &functor, const std::vector<Term> &terms)
        : functor(functor), terms(terms) {
    }

    std::string functor;
    std::vector<Term> terms;
};

/**
 * \brief A unary operation with its operand.
 */
template <typename Op>
struct UnaryOp {
    UnaryOp() { }

    UnaryOp(const Term &operand)
        : operand(operand) {
    }

    Term operand;
};

/**
 * \brief A binary operation with its operands.
 */
template <typename Op>
struct BinaryOp {
    BinaryOp() { }

    BinaryOp(const Term &left, const Term &right)
        : left(left), right(right) {
    }

    Term left;
    Term right;
};

struct IsValidContext : boost::static_visitor<bool> {
    bool operator()(const bool &) const {
        return true;
    }

    bool operator()(const double &) const {
        return false;
    }

    bool operator()(const std::string &) const {
        return false;
    }

    bool operator()(const std::vector<Term> &) const {
        return false;
    }

    bool operator()(const BeliefAtom &) const {
        return true;
    }

    bool operator()(const Variable &) const {
        // TODO: Investigate whether variables might even be executed as
        // actions when they appear at the root level in a context.
        return true;
    }

    bool operator()(const UnaryOp<OpPositive> &) const {
        return false;
    }

    bool operator()(const UnaryOp<OpNegative> &) const {
        return false;
    }

    bool operator()(const UnaryOp<OpNot> &) const {
        return true;
    }

    bool operator()(const BinaryOp<OpPlus> &) const {
        return false;
    }

    bool operator()(const BinaryOp<OpMinus> &) const {
        return false;
    }

    bool operator()(const BinaryOp<OpPow> &) const {
        return false;
    }

    bool operator()(const BinaryOp<OpMultiply> &) const {
        return false;
    }

    bool operator()(const BinaryOp<OpDivide> &) const {
        return false;
    }

    bool operator()(const BinaryOp<OpDiv> &) const {
        return false;
    }

    bool operator()(const BinaryOp<OpMod> &) const {
        return false;
    }

    bool operator()(const BinaryOp<OpAnd> &) const {
        return true;
    }

    bool operator()(const BinaryOp<OpOr> &) const {
        return true;
    }

    bool operator()(const BinaryOp<OpUnify> &) const {
        return true;
    }

    bool operator()(const BinaryOp<OpDeconstruct> &) const {
        return true;
    }

    bool operator()(const BinaryOp<OpEq> &) const {
        return true;
    }

    bool operator()(const BinaryOp<OpNeq> &) const {
        return true;
    }

    bool operator()(const BinaryOp<OpLt> &) const {
        return true;
    }

    bool operator()(const BinaryOp<OpLte> &) const {
        return true;
    }
};

/**
 * \brief Term visitor that checks for groundness.
 */
struct IsGround : boost::static_visitor<bool> {
    bool operator()(const bool &) const {
        return true;
    }

    bool operator()(const double &) const {
        return true;
    }

    bool operator()(const std::string &) const {
        return true;
    }

    bool operator()(const std::vector<Term> &list) const {
        for (const Term &term : list) {
            if (!boost::apply_visitor(*this, term)) {
                return false;
            }
        }

        return true;
    }

    bool operator()(const BeliefAtom &belief) const {
        std::vector<Term>::const_iterator it = belief.terms.begin();

        while (it != belief.terms.end()) {
            if (!boost::apply_visitor(*this, *it)) {
                return false;
            }
            ++it;
        }

        return true;
    }

    template <typename T>
    bool operator()(const T &) const {
        return false;
    }
};

struct IsUnifiable : boost::static_visitor<bool> {
    bool operator()(const Variable &) const {
        return true;
    }

    bool operator()(const std::vector<Term> &list) const {
        for (const Term &term : list) {
            if (!boost::apply_visitor(*this, term)) {
                return false;
            }
        }

        return true;
    }

    bool operator()(const BeliefAtom &belief) const {
        std::vector<Term>::const_iterator it = belief.terms.begin();

        while (it != belief.terms.end()) {
            if (!boost::apply_visitor(*this, *it)) {
                return false;
            }
            ++it;
        }

        return true;
    }

    template <typename T>
    bool operator()(const T &t) const {
        return IsGround()(t);
    }
};

struct OpPositive {
    typedef Term result_type;
    Term operator()(const Term &operand) const;
};

struct OpNegative {
    typedef Term result_type;
    Term operator()(const Term &operand) const;
};

struct OpNot {
    typedef Term result_type;
    Term operator()(const Term &operand) const;
};

struct OpPlus {
    typedef Term result_type;
    Term operator()(const Term &left, const Term &right) const;
};

struct OpMinus {
    typedef Term result_type;
    Term operator()(const Term &left, const Term &right) const;
};

struct OpPow {
    typedef Term result_type;
    Term operator()(const Term &left, const Term &right) const;
};

struct OpMultiply {
    typedef Term result_type;
    Term operator()(const Term &left, const Term &right) const;
};

struct OpDivide {
    typedef Term result_type;
    Term operator()(const Term &left, const Term &right) const;
};

struct OpDiv {
    typedef Term result_type;
    Term operator()(const Term &left, const Term &right) const;
};

struct OpMod {
    typedef Term result_type;
    Term operator()(const Term &left, const Term &right) const;
};

struct OpAnd {
    typedef Term result_type;
    Term operator()(const Term &left, const Term &right) const;
};

struct OpOr {
    typedef Term result_type;
    Term operator()(const Term &left, const Term &right) const;
};

struct OpNeq {
    typedef Term result_type;
    Term operator()(const Term &left, const Term &right) const;
};

struct OpEq {
    typedef Term result_type;
    Term operator()(const Term &left, const Term &right) const;
};

struct OpUnify {
    typedef Term result_type;
    Term operator()(const Term &left, const Term &right) const;
};

struct OpDeconstruct {
    typedef Term result_type;
    Term operator()(const Term &left, const Term &right) const;
};

struct OpLt {
    typedef Term result_type;
    Term operator()(const Term &left, const Term &right) const;
};

struct OpLte {
    typedef Term result_type;
    Term operator()(const Term &left, const Term &right) const;
};

}  // namespace as

#endif  // AS_TERM_H_
