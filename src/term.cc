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

#include "term.h"

#include <iostream>
#include <stdexcept>

#include <boost/variant/apply_visitor.hpp>

#include "unification.h"

namespace as {

Term OpPositive::operator()(const Term &operand) const {
    switch (operand.which()) {
        case kVariable:
            return UnaryOp<OpPositive>(operand);

        case kDouble:
        case kNegative:
        case kPositive:
        case kPlus:
        case kMinus:
        case kPow:
        case kMultiply:
        case kDivide:
        case kDiv:
        case kMod:
            return operand;

        default:
            throw std::domain_error("invalid operand type for OpPositive");
    }
}

Term OpNegative::operator()(const Term &operand) const {
    switch (operand.which()) {
        case kDouble:
            return -boost::get<double>(operand);

        case kNegative:
            return UnaryOp<OpPositive>(operand);

        case kVariable:
        case kPositive:
        case kPlus:
        case kMinus:
        case kPow:
        case kMultiply:
        case kDivide:
        case kDiv:
        case kMod:
            return UnaryOp<OpNegative>(operand);

        default:
            throw std::domain_error("invalid operand type for OpNegative");
    }
}

Term OpNot::operator()(const Term &operand) const {
    switch (operand.which()) {
        case kBool:
            return !boost::get<bool>(operand);

        case kVariable:
        case kBeliefAtom:
        case kAnd:
        case kOr:
        case kUnify:
        case kDeconstruct:
            return UnaryOp<OpNot>(operand);

        case kNot:
            {
                UnaryOp<OpNot> not_op = boost::get<UnaryOp<OpNot> >(operand);
                if (not_op.operand.which() == kNot) {
                    return not_op.operand;
                }

                return UnaryOp<OpNot>(operand);
            }

        case kEq:
            {
                BinaryOp<OpEq> eq_op = boost::get<BinaryOp<OpEq> >(operand);
                return BinaryOp<OpNeq>(eq_op.left, eq_op.right);
            }

        case kNeq:
            {
                BinaryOp<OpNeq> neq_op = boost::get<BinaryOp<OpNeq> >(operand);
                return BinaryOp<OpEq>(neq_op.left, neq_op.right);
            }

        case kLt:
            {
                BinaryOp<OpLt> lt_op = boost::get<BinaryOp<OpLt> >(operand);
                return BinaryOp<OpLte>(lt_op.right, lt_op.left);
            }

        case kLte:
            {
                BinaryOp<OpLte> lte_op = boost::get<BinaryOp<OpLte> >(operand);
                return BinaryOp<OpLt>(lte_op.right, lte_op.left);
            }

        default:
            throw std::domain_error("invalid operand type for OpNot");
    }
}

Term OpPlus::operator()(const Term &left, const Term &right) const {
    if (left.which() == kDouble && right.which() == kDouble) {
        return boost::get<double>(left) + boost::get<double>(right);
    }

    Term lhs;

    switch (left.which()) {
        case kDouble:
        case kVariable:
        case kNegative:
        case kPlus:
        case kMinus:
        case kPow:
        case kMultiply:
        case kDivide:
        case kDiv:
        case kMod:
            lhs = left;
            break;

        case kPositive:
            {
                UnaryOp<OpPositive> positive_op = boost::get<UnaryOp<OpPositive> >(left);
                lhs = positive_op.operand;
            }
            break;

        default:
            throw std::domain_error("invalid operand types for OpPlus");
    }

    switch (right.which()) {
        case kDouble:
        case kVariable:
        case kPlus:
        case kMinus:
        case kPow:
        case kMultiply:
        case kDivide:
        case kDiv:
        case kMod:
            return BinaryOp<OpPlus>(lhs, right);

        case kPositive:
            {
                UnaryOp<OpPositive> positive_op = boost::get<UnaryOp<OpPositive> >(right);
                return BinaryOp<OpPlus>(lhs, positive_op.operand);
            }

        case kNegative:
            {
                UnaryOp<OpNegative> negative_op = boost::get<UnaryOp<OpNegative> >(right);
                return BinaryOp<OpMinus>(lhs, negative_op.operand);
            }

        default:
            throw std::domain_error("invalid operand types for OpPlus");
    }
}

Term OpMinus::operator()(const Term &left, const Term &right) const {
    if (left.which() == kDouble && right.which() == kDouble) {
        return boost::get<double>(left) - boost::get<double>(right);
    }

    Term lhs;

    switch (left.which()) {
        case kDouble:
        case kVariable:
        case kNegative:
        case kPlus:
        case kMinus:
        case kPow:
        case kMultiply:
        case kDivide:
        case kDiv:
        case kMod:
            lhs = left;
            break;

        case kPositive:
            {
                UnaryOp<OpPositive> positive_op = boost::get<UnaryOp<OpPositive> >(left);
                lhs = positive_op.operand;
            }
            break;

        default:
            throw std::domain_error("invalid operand types for OpMinus");
    }

    switch (right.which()) {
        case kDouble:
        case kVariable:
        case kPlus:
        case kMinus:
        case kPow:
        case kMultiply:
        case kDivide:
        case kDiv:
        case kMod:
            return BinaryOp<OpMinus>(lhs, right);

        case kPositive:
            {
                UnaryOp<OpPositive> positive_op = boost::get<UnaryOp<OpPositive> >(right);
                return BinaryOp<OpMinus>(lhs, positive_op.operand);
            }

        case kNegative:
            {
                UnaryOp<OpNegative> negative_op = boost::get<UnaryOp<OpNegative> >(right);
                return BinaryOp<OpPlus>(lhs, negative_op.operand);
            }

        default:
            throw std::domain_error("invalid operand types for OpMinus");
    }
}

Term OpPow::operator()(const Term &left, const Term &right) const {
    if (left.which() == kDouble && right.which() == kDouble) {
        return std::pow(boost::get<double>(left), boost::get<double>(right));
    }

    Term lhs;

    switch (left.which()) {
        case kDouble:
        case kVariable:
        case kNegative:
        case kPlus:
        case kMinus:
        case kPow:
        case kMultiply:
        case kDivide:
        case kDiv:
        case kMod:
            lhs = left;
            break;

        case kPositive:
            {
                UnaryOp<OpPositive> positive_op = boost::get<UnaryOp<OpPositive> >(left);
                lhs = positive_op.operand;
            }
            break;

        default:
            throw std::domain_error("invalid operand types for OpPow");
    }

    switch (right.which()) {
        case kDouble:
        case kVariable:
        case kPlus:
        case kMinus:
        case kPow:
        case kMultiply:
        case kDivide:
        case kDiv:
        case kMod:
        case kNegative:
            return BinaryOp<OpPow>(lhs, right);

        case kPositive:
            {
                UnaryOp<OpPositive> positive_op = boost::get<UnaryOp<OpPositive> >(right);
                return BinaryOp<OpPow>(lhs, positive_op.operand);
            }

        default:
            throw std::domain_error("invalid operand types for OpPow");
    }
}

Term OpMultiply::operator()(const Term &left, const Term &right) const {
    if (left.which() == kDouble && right.which() == kDouble) {
        return boost::get<double>(left) * boost::get<double>(right);
    }

    Term lhs;

    switch (left.which()) {
        case kDouble:
        case kVariable:
        case kNegative:
        case kPlus:
        case kMinus:
        case kPow:
        case kMultiply:
        case kDivide:
        case kDiv:
        case kMod:
            lhs = left;
            break;

        case kPositive:
            {
                UnaryOp<OpPositive> positive_op = boost::get<UnaryOp<OpPositive> >(left);
                lhs = positive_op.operand;
            }
            break;

        default:
            throw std::domain_error("invalid operand types for OpMultiply");
    }

    switch (right.which()) {
        case kDouble:
        case kVariable:
        case kPlus:
        case kMinus:
        case kPow:
        case kMultiply:
        case kDivide:
        case kDiv:
        case kMod:
        case kNegative:
            return BinaryOp<OpMultiply>(lhs, right);

        case kPositive:
            {
                UnaryOp<OpPositive> positive_op = boost::get<UnaryOp<OpPositive> >(right);
                return BinaryOp<OpMultiply>(lhs, positive_op.operand);
            }

        default:
            throw std::domain_error("invalid operand types for OpMultiply");
    }
}

Term OpDivide::operator()(const Term &left, const Term &right) const {
    if (left.which() == kDouble && right.which() == kDouble) {
        return boost::get<double>(left) / boost::get<double>(right);
    }

    Term lhs;

    switch (left.which()) {
        case kDouble:
        case kVariable:
        case kNegative:
        case kPlus:
        case kMinus:
        case kPow:
        case kMultiply:
        case kDivide:
        case kDiv:
        case kMod:
            lhs = left;
            break;

        case kPositive:
            {
                UnaryOp<OpPositive> positive_op = boost::get<UnaryOp<OpPositive> >(left);
                lhs = positive_op.operand;
            }
            break;

        default:
            throw std::domain_error("invalid operand types for OpDivide");
    }

    switch (right.which()) {
        case kDouble:
        case kVariable:
        case kPlus:
        case kMinus:
        case kPow:
        case kMultiply:
        case kDivide:
        case kDiv:
        case kMod:
        case kNegative:
            return BinaryOp<OpDivide>(lhs, right);

        case kPositive:
            {
                UnaryOp<OpPositive> positive_op = boost::get<UnaryOp<OpPositive> >(right);
                return BinaryOp<OpDivide>(lhs, positive_op.operand);
            }

        default:
            throw std::domain_error("invalid operand types for OpDivide");
    }
}

Term OpDiv::operator()(const Term &left, const Term &right) const {
    if (left.which() == kDouble && right.which() == kDouble) {
        return std::floor(boost::get<double>(left) / boost::get<double>(right));
    }

    Term lhs;

    switch (left.which()) {
        case kDouble:
        case kVariable:
        case kNegative:
        case kPlus:
        case kMinus:
        case kPow:
        case kMultiply:
        case kDivide:
        case kDiv:
        case kMod:
            lhs = left;
            break;

        case kPositive:
            {
                UnaryOp<OpPositive> positive_op = boost::get<UnaryOp<OpPositive> >(left);
                lhs = positive_op.operand;
            }
            break;

        default:
            throw std::domain_error("invalid operand types for OpDiv");
    }

    switch (right.which()) {
        case kDouble:
        case kVariable:
        case kPlus:
        case kMinus:
        case kPow:
        case kMultiply:
        case kDivide:
        case kDiv:
        case kMod:
        case kNegative:
            return BinaryOp<OpDiv>(lhs, right);

        case kPositive:
            {
                UnaryOp<OpPositive> positive_op = boost::get<UnaryOp<OpPositive> >(right);
                return BinaryOp<OpDiv>(lhs, positive_op.operand);
            }

        default:
            throw std::domain_error("invalid operand types for OpDiv");
    }
}

Term OpMod::operator()(const Term &left, const Term &right) const {
    if (left.which() == kDouble && right.which() == kDouble) {
        return std::fmod(boost::get<double>(left), boost::get<double>(right));
    }

    Term lhs;

    switch (left.which()) {
        case kDouble:
        case kVariable:
        case kNegative:
        case kPlus:
        case kMinus:
        case kPow:
        case kMultiply:
        case kDivide:
        case kDiv:
        case kMod:
            lhs = left;
            break;

        case kPositive:
            {
                UnaryOp<OpPositive> positive_op = boost::get<UnaryOp<OpPositive> >(left);
                lhs = positive_op.operand;
            }
            break;

        default:
            throw std::domain_error("invalid operand types for OpMod");
    }

    switch (right.which()) {
        case kDouble:
        case kVariable:
        case kPlus:
        case kMinus:
        case kPow:
        case kMultiply:
        case kDivide:
        case kDiv:
        case kMod:
        case kNegative:
            return BinaryOp<OpMod>(lhs, right);

        case kPositive:
            {
                UnaryOp<OpPositive> positive_op = boost::get<UnaryOp<OpPositive> >(right);
                return BinaryOp<OpMod>(lhs, positive_op.operand);
            }

        default:
            throw std::domain_error("invalid operand types for OpMod");
    }
}

Term OpAnd::operator()(const Term &left, const Term &right) const {
    switch (left.which()) {
        case kBool:
        case kBeliefAtom:
        case kVariable:
        case kNot:
        case kAnd:
        case kOr:
        case kUnify:
        case kDeconstruct:
        case kEq:
        case kNeq:
        case kLt:
        case kLte:
            break;

        default:
            throw std::domain_error("invalid operand types for OpAnd");
    }

    switch (right.which()) {
        case kBool:
        case kBeliefAtom:
        case kVariable:
        case kNot:
        case kAnd:
        case kOr:
        case kUnify:
        case kDeconstruct:
        case kEq:
        case kNeq:
        case kLt:
        case kLte:
            break;

        default:
            throw std::domain_error("invalid operand types for OpAnd");
    }

    if (left.which() == kBool) {
        if (boost::get<bool>(left)) {
            if (right.which() != kVariable) {
                return right;
            }
        } else {
            return false;
        }
    }

    if (right.which() == kBool) {
        if (boost::get<bool>(right)) {
            if (left.which() != kVariable) {
                return left;
            }
        } else {
            return false;
        }
    }

    return BinaryOp<OpAnd>(left, right);
}

Term OpOr::operator()(const Term &left, const Term &right) const {
    switch (left.which()) {
        case kBool:
        case kBeliefAtom:
        case kVariable:
        case kNot:
        case kAnd:
        case kOr:
        case kUnify:
        case kDeconstruct:
        case kEq:
        case kNeq:
        case kLt:
        case kLte:
            break;

        default:
            throw std::domain_error("invalid operand types for OpOr");
    }

    switch (right.which()) {
        case kBool:
        case kBeliefAtom:
        case kVariable:
        case kNot:
        case kAnd:
        case kOr:
        case kUnify:
        case kDeconstruct:
        case kEq:
        case kNeq:
        case kLt:
        case kLte:
            break;

        default:
            throw std::domain_error("invalid operand types for OpOr");
    }

    if (left.which() == kBool) {
        if (!boost::get<bool>(left)) {
            if (right.which() != kVariable) {
                return right;
            }
        } else {
            return true;
        }
    }

    if (right.which() == kBool) {
        if (!boost::get<bool>(right)) {
            if (left.which() != kVariable) {
                return left;
            }
        } else {
            return true;
        }
    }

    return BinaryOp<OpOr>(left, right);
}

Term OpNeq::operator()(const Term &left, const Term &right) const {
    switch (left.which()) {
        case kBool:
            switch (right.which()) {
                case kBool:
                    return boost::get<bool>(left) != boost::get<bool>(right);

                case kVariable:
                case kNot:
                case kAnd:
                case kOr:
                case kUnify:
                case kDeconstruct:
                case kEq:
                case kNeq:
                case kLt:
                case kLte:
                    return BinaryOp<OpNeq>(left, right);

                default:
                    return true;
            }

        case kDouble:
            switch (right.which()) {
                case kDouble:
                    return boost::get<double>(left) != boost::get<double>(right);

                case kVariable:
                case kNegative:
                case kPositive:
                case kPlus:
                case kMinus:
                case kPow:
                case kMultiply:
                case kDivide:
                case kDiv:
                case kMod:
                    return BinaryOp<OpNeq>(left, right);

                default:
                    return true;
            }

        case kString:
            switch (right.which()) {
                case kString:
                    return boost::get<std::string>(left) != boost::get<std::string>(right);

                case kVariable:
                    return BinaryOp<OpNeq>(left, right);

                default:
                    return true;
            }

        case kList:
            switch (right.which()) {
                case kList:
                    {
                        std::vector<Term> left_list = boost::get<std::vector<Term> >(left);
                        std::vector<Term> right_list = boost::get<std::vector<Term> >(right);

                        if (left_list.size() != right_list.size()) {
                            return true;
                        }

                        Term result(false);

                        std::vector<Term>::const_iterator left_it = left_list.begin();
                        std::vector<Term>::const_iterator right_it = right_list.begin();

                        while (left_it != left_list.end() && right_it != right_list.end()) {
                            result = OpOr()(result, OpNeq()(*left_it, *right_it));
                            ++left_it;
                            ++right_it;
                        }

                        return result;
                    }

                case kVariable:
                    return BinaryOp<OpNeq>(left, right);

                default:
                    return true;
            }

        case kBeliefAtom:
            switch (right.which()) {
                case kBeliefAtom:
                    {
                        BeliefAtom left_atom = boost::get<BeliefAtom>(left);
                        BeliefAtom right_atom = boost::get<BeliefAtom>(right);

                        if (left_atom.functor != right_atom.functor) {
                            return true;
                        }

                        if (left_atom.terms.size() != right_atom.terms.size()) {
                            return true;
                        }

                        Term result(false);

                        std::vector<Term>::const_iterator left_it = left_atom.terms.begin();
                        std::vector<Term>::const_iterator right_it = right_atom.terms.begin();

                        while (left_it != left_atom.terms.end() && right_it != right_atom.terms.end()) {
                            result = OpOr()(result, OpNeq()(*left_it, *right_it));
                            ++left_it;
                            ++right_it;
                        }

                        return result;
                    }

                case kVariable:
                    return BinaryOp<OpNeq>(left, right);

                default:
                    return true;
            }

        case kVariable:
            switch (right.which()) {
                case kVariable:
                    {
                        Variable left_var = boost::get<Variable>(left);
                        Variable right_var = boost::get<Variable>(right);
                        if (left_var.name == right_var.name) {
                            return false;
                        }

                        return BinaryOp<OpNeq>(left, right);
                    }

                default:
                    return BinaryOp<OpNeq>(left, right);
            }

        case kPositive:
        case kNegative:
        case kPlus:
        case kMinus:
        case kPow:
        case kMultiply:
        case kDivide:
        case kDiv:
        case kMod:
            switch (right.which()) {
                case kDouble:
                case kVariable:
                case kPositive:
                case kPlus:
                case kMinus:
                case kPow:
                case kMultiply:
                case kDivide:
                case kDiv:
                case kMod:
                    return BinaryOp<OpNeq>(left, right);

                default:
                    return true;
            }

        case kNot:
        case kAnd:
        case kOr:
        case kUnify:
        case kDeconstruct:
        case kEq:
        case kNeq:
        case kLt:
        case kLte:
            switch (right.which()) {
                case kBool:
                case kVariable:
                case kNot:
                case kAnd:
                case kOr:
                case kUnify:
                case kDeconstruct:
                case kEq:
                case kNeq:
                case kLt:
                case kLte:
                    return BinaryOp<OpNeq>(left, right);

                default:
                    return true;
            }

        default:
            return BinaryOp<OpNeq>(left, right);
    }
}

Term OpUnify::operator()(const Term &left, const Term &right) const {
    if (IsUnifiable()(left) && IsUnifiable()(right)) {
        Unifier unifier;
        bool unifies = Unify(left, right, unifier);

        if (!unifies) {
            return false;
        } else if (unifier.empty()) {
            // Apparently it unifies unconditionally.
            return true;
        }
    }

    // Even if a term like X + 1 is not unifiable (i.e. can not be used in
    // unifications) right now, it might be unifiable once Y is known.
    return BinaryOp<OpUnify>(left, right);
}

Term OpDeconstruct::operator()(const Term &left, const Term &right) const {
    switch (left.which()) {
        case kBeliefAtom:
            switch (right.which()) {
                case kList:
                case kVariable:
                    {
                        BeliefAtom atom = boost::get<BeliefAtom>(left);

                        BeliefAtom functor_atom;
                        functor_atom.functor = atom.functor;

                        std::vector<as::Term> deconstructed;
                        deconstructed.reserve(2);
                        deconstructed.push_back(functor_atom);
                        deconstructed.push_back(atom.terms);

                        return OpUnify()(deconstructed, right);
                    }

                default:
                    throw std::domain_error("invalid operand types for OpDeconstruct");
        }

        case kVariable:
            switch (right.which()) {
                case kVariable:
                    break;

                case kList:
                    {
                        std::vector<Term> list = boost::get<std::vector<Term> >(right);
                        if (list.size() != 2) {
                            return false;
                        }

                        switch (list[0].which()) {
                            case kBeliefAtom:
                                {
                                    BeliefAtom atom = boost::get<BeliefAtom>(list[0]);
                                    if (atom.terms.size() > 0) {
                                        return false;
                                    }
                                }
                                break;

                            case kVariable:
                                break;

                            default:
                                return false;
                        }

                        switch (list[1].which()) {
                            case kList:
                            case kVariable:
                                break;

                            default:
                                return false;
                        }
                    }
                    break;

                default:
                    throw std::domain_error("invalid operand types for OpDeconstruct");
            }

            return BinaryOp<OpDeconstruct>(left, right);

        default:
            throw std::domain_error("invalid operand types for OpDeconstruct");
    }
}

Term OpEq::operator()(const Term &left, const Term &right) const {
    return OpNot()(OpNeq()(left, right));
}

Term OpLt::operator()(const Term &left, const Term &right) const {
    switch (left.which()) {
        case kBool:
            switch (right.which()) {
                case kBool:
                    return boost::get<bool>(left) < boost::get<bool>(right);

                case kString:
                    return kBool < kString;

                case kList:
                    return kBool < kList;

                case kBeliefAtom:
                    return kBool < kBeliefAtom;

                case kDouble:
                case kPositive:
                case kNegative:
                case kPlus:
                case kMinus:
                case kPow:
                case kMultiply:
                case kDivide:
                case kDiv:
                case kMod:
                    return kBool < kDouble;

                default:
                    return BinaryOp<OpLt>(left, right);
            }

        case kDouble:
            switch (right.which()) {
                case kDouble:
                    return boost::get<double>(left) < boost::get<double>(right);

                case kString:
                    return kDouble < kString;

                case kList:
                    return kDouble < kList;

                case kBeliefAtom:
                    return kDouble < kBeliefAtom;

                case kBool:
                case kNot:
                case kAnd:
                case kOr:
                case kUnify:
                case kDeconstruct:
                case kEq:
                case kNeq:
                case kLt:
                case kLte:
                    return kDouble < kBool;

                default:
                    return BinaryOp<OpLt>(left, right);
            }

        case kString:
            switch (right.which()) {
                case kString:
                    return boost::get<std::string>(left) < boost::get<std::string>(right);

                case kDouble:
                case kPositive:
                case kNegative:
                case kPlus:
                case kMinus:
                case kPow:
                case kMultiply:
                case kDivide:
                case kDiv:
                case kMod:
                    return kString < kDouble;

                case kBool:
                case kNot:
                case kAnd:
                case kOr:
                case kUnify:
                case kDeconstruct:
                case kEq:
                case kNeq:
                case kLt:
                case kLte:
                    return kString < kBool;

                case kList:
                    return kString < kList;

                default:
                    return BinaryOp<OpLt>(left, right);
            }

        case kList:
            switch (right.which()) {
                case kList:
                    {
                        std::vector<Term> left_list = boost::get<std::vector<Term> >(left);
                        std::vector<Term> right_list = boost::get<std::vector<Term> >(right);

                        if (left_list.size() < right_list.size()) {
                            return true;
                        } else if (left_list.size() > right_list.size()) {
                            return false;
                        }

                        std::vector<Term>::const_iterator left_it = left_list.begin();
                        std::vector<Term>::const_iterator right_it = right_list.begin();

                        Term result(false);
                        Term equal_so_far(true);

                        while (left_it != left_list.end() && right_it != right_list.end()) {
                            result = OpOr()(result, OpAnd()(equal_so_far, OpLt()(*left_it, *right_it)));
                            equal_so_far = OpAnd()(equal_so_far, OpEq()(*left_it, *right_it));

                            ++left_it;
                            ++right_it;
                        }

                        return result;
                    }

                case kBool:
                case kNot:
                case kAnd:
                case kOr:
                case kUnify:
                case kDeconstruct:
                case kEq:
                case kNeq:
                case kLt:
                case kLte:
                    return kList < kBool;

                case kDouble:
                case kPositive:
                case kNegative:
                case kPlus:
                case kMinus:
                case kPow:
                case kMultiply:
                case kDivide:
                case kDiv:
                case kMod:
                    return kList < kDouble;

                case kBeliefAtom:
                    return kList < kBeliefAtom;

                case kString:
                    return kList < kString;

                default:
                    return BinaryOp<OpLt>(left, right);
            }

        case kBeliefAtom:
            switch (right.which()) {
                case kBeliefAtom:
                    {
                        BeliefAtom left_atom = boost::get<BeliefAtom>(left);
                        BeliefAtom right_atom = boost::get<BeliefAtom>(right);

                        if (left_atom.functor < right_atom.functor) {
                            return true;
                        } else if (left_atom.functor > right_atom.functor) {
                            return false;
                        }

                        if (left_atom.terms.size() < right_atom.terms.size()) {
                            return true;
                        } else if (left_atom.terms.size() > right_atom.terms.size()) {
                            return false;
                        }

                        std::vector<Term>::const_iterator left_it = left_atom.terms.begin();
                        std::vector<Term>::const_iterator right_it = right_atom.terms.begin();

                        Term result(false);
                        Term equal_so_far(true);

                        while (left_it != left_atom.terms.end() && right_it != right_atom.terms.end()) {
                            result = OpOr()(result, OpAnd()(equal_so_far, OpLt()(*left_it, *right_it)));
                            equal_so_far = OpAnd()(equal_so_far, OpEq()(*left_it, *right_it));

                            ++left_it;
                            ++right_it;
                        }

                        return result;
                    }

                case kBool:
                case kNot:
                case kAnd:
                case kOr:
                case kUnify:
                case kDeconstruct:
                case kEq:
                case kNeq:
                case kLt:
                case kLte:
                    return kBeliefAtom < kBool;

                case kDouble:
                case kPositive:
                case kNegative:
                case kPlus:
                case kMinus:
                case kPow:
                case kMultiply:
                case kDivide:
                case kDiv:
                case kMod:
                    return kBeliefAtom < kDouble;

                case kString:
                    return kBeliefAtom < kString;

                case kList:
                    return kBeliefAtom < kList;

                default:
                    return BinaryOp<OpLt>(left, right);
            }

        case kVariable:
            switch (right.which()) {
                case kVariable:
                    {
                        Variable left_var = boost::get<Variable>(left);
                        Variable right_var = boost::get<Variable>(right);

                        if (left_var.name == right_var.name) {
                            return false;
                        }

                        return BinaryOp<OpLt>(left, right);
                    }

                default:
                    return BinaryOp<OpLt>(left, right);
            }

        case kPositive:
        case kNegative:
        case kPlus:
        case kMinus:
        case kPow:
        case kMultiply:
        case kDivide:
        case kDiv:
        case kMod:
            switch (right.which()) {
                case kBool:
                case kNot:
                case kAnd:
                case kOr:
                case kUnify:
                case kDeconstruct:
                case kEq:
                case kNeq:
                case kLt:
                case kLte:
                    return kDouble < kBool;

                case kBeliefAtom:
                    return kDouble < kBeliefAtom;

                case kString:
                    return kDouble < kString;

                case kList:
                    return kDouble < kList;

                default:
                    return BinaryOp<OpLt>(left, right);
            }

        case kNot:
        case kAnd:
        case kOr:
        case kUnify:
        case kDeconstruct:
        case kEq:
        case kNeq:
        case kLt:
        case kLte:
            switch (right.which()) {
                case kDouble:
                case kPositive:
                case kNegative:
                case kPlus:
                case kMinus:
                case kPow:
                case kMultiply:
                case kDivide:
                case kDiv:
                case kMod:
                    return kBool < kDouble;

                case kBeliefAtom:
                    return kBool < kBeliefAtom;

                case kString:
                    return kBool < kString;

                case kList:
                    return kBool < kList;

                default:
                    return BinaryOp<OpLt>(left, right);
            }

        default:
            return BinaryOp<OpLt>(left, right);
    }
}

Term OpLte::operator()(const Term &left, const Term &right) const {
    return OpOr()(OpLt()(left, right), OpEq()(left, right));
}

}  // namespace as
