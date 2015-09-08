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

#include "printer.h"

#include <vector>

namespace as {

void Printer::operator()(const bool &operand) const {
    if (operand) {
        std::cout << "true";
    } else {
        std::cout << "false";
    }
}

void Printer::operator()(const double &operand) const {
    std::cout << operand;
}

void Printer::operator()(const std::string &operand) const {
    std::cout << "\"";

    // Output the string with quotes escaped.
    for (std::string::size_type i = 0; i < operand.length(); i++) {
        switch (operand[i]) {
            case '"':
            case '\\':
                std::cout << '\\' << operand[i];
                break;

            default:
                std::cout << operand[i];
        }
    }

    std::cout << "\"";
}

void Printer::operator()(const std::vector<Term> &operand) const {
    std::cout << "[";

    std::vector<Term>::const_iterator it = operand.begin();
    while (it != operand.end()) {
        boost::apply_visitor(*this, *it);
        ++it;

        if (it != operand.end()) {
            std::cout << ", ";
        }
    }

    std::cout << "]";
}

void Printer::operator()(const BeliefAtom &operand) const {
    std::cout << operand.functor;

    if (operand.terms.size() > 0) {
        std::cout << "(";

        std::vector<Term>::const_iterator it = operand.terms.begin();
        while (it != operand.terms.end()) {
            boost::apply_visitor(*this, *it);
            ++it;

            if (it != operand.terms.end()) {
                std::cout << ", ";
            }
        }

        std::cout << ")";
    }
}

void Printer::operator()(const Variable &operand) const {
    std::cout << operand.name;
}

void Printer::operator()(const UnaryOp<OpPositive> &operand) const {
    std::cout << "(+";
    boost::apply_visitor(*this, operand.operand);
    std::cout << ")";
}

void Printer::operator()(const UnaryOp<OpNegative> &operand) const {
    std::cout << "(-";
    boost::apply_visitor(*this, operand.operand);
    std::cout << ")";
}

void Printer::operator()(const UnaryOp<OpNot> &operand) const {
    std::cout << "not ";
    boost::apply_visitor(*this, operand.operand);
}

void Printer::operator()(const BinaryOp<OpPlus> &operand) const {
    std::cout << "(";
    boost::apply_visitor(*this, operand.left);
    std::cout << " + ";
    boost::apply_visitor(*this, operand.right);
    std::cout << ")";
}

void Printer::operator()(const BinaryOp<OpMinus> &operand) const {
    std::cout << "(";
    boost::apply_visitor(*this, operand.left);
    std::cout << " - ";
    boost::apply_visitor(*this, operand.right);
    std::cout << ")";
}

void Printer::operator()(const BinaryOp<OpPow> &operand) const {
    std::cout << "(";
    boost::apply_visitor(*this, operand.left);
    std::cout << "**";
    boost::apply_visitor(*this, operand.right);
    std::cout << ")";
}

void Printer::operator()(const BinaryOp<OpMultiply> &operand) const {
    std::cout << "(";
    boost::apply_visitor(*this, operand.left);
    std::cout << " * ";
    boost::apply_visitor(*this, operand.right);
    std::cout << ")";
}

void Printer::operator()(const BinaryOp<OpDivide> &operand) const {
    std::cout << "(";
    boost::apply_visitor(*this, operand.left);
    std::cout << " / ";
    boost::apply_visitor(*this, operand.right);
    std::cout << ")";
}

void Printer::operator()(const BinaryOp<OpDiv> &operand) const {
    std::cout << "(";
    boost::apply_visitor(*this, operand.left);
    std::cout << " div ";
    boost::apply_visitor(*this, operand.right);
    std::cout << ")";
}

void Printer::operator()(const BinaryOp<OpMod> &operand) const {
    std::cout << "(";
    boost::apply_visitor(*this, operand.left);
    std::cout << " mod ";
    boost::apply_visitor(*this, operand.right);
    std::cout << ")";
}

void Printer::operator()(const BinaryOp<OpAnd> &operand) const {
    std::cout << "(";
    boost::apply_visitor(*this, operand.left);
    std::cout << " & ";
    boost::apply_visitor(*this, operand.right);
    std::cout << ")";
}

void Printer::operator()(const BinaryOp<OpOr> &operand) const {
    std::cout << "(";
    boost::apply_visitor(*this, operand.left);
    std::cout << " | ";
    boost::apply_visitor(*this, operand.right);
    std::cout << ")";
}

void Printer::operator()(const BinaryOp<OpUnify> &operand) const {
    std::cout << "(";
    boost::apply_visitor(*this, operand.left);
    std::cout << " = ";
    boost::apply_visitor(*this, operand.right);
    std::cout << ")";
}

void Printer::operator()(const BinaryOp<OpDeconstruct> &operand) const {
    std::cout << "(";
    boost::apply_visitor(*this, operand.left);
    std::cout << " =.. ";
    boost::apply_visitor(*this, operand.right);
    std::cout << ")";
}

void Printer::operator()(const BinaryOp<OpEq> &operand) const {
    std::cout << "(";
    boost::apply_visitor(*this, operand.left);
    std::cout << " == ";
    boost::apply_visitor(*this, operand.right);
    std::cout << ")";
}

void Printer::operator()(const BinaryOp<OpNeq> &operand) const {
    std::cout << "(";
    boost::apply_visitor(*this, operand.left);
    std::cout << " \\== ";
    boost::apply_visitor(*this, operand.right);
    std::cout << ")";
}

void Printer::operator()(const BinaryOp<OpLt> &operand) const {
    std::cout << "(";
    boost::apply_visitor(*this, operand.left);
    std::cout << " < ";
    boost::apply_visitor(*this, operand.right);
    std::cout << ")";
}

void Printer::operator()(const BinaryOp<OpLte> &operand) const {
    std::cout << "(";
    boost::apply_visitor(*this, operand.left);
    std::cout << " <= ";
    boost::apply_visitor(*this, operand.right);
    std::cout << ")";
}

void Printer::operator()(const Rule &operand) const {
    (*this)(operand.lhs);
    std::cout << " :- ";
    boost::apply_visitor(*this, operand.rhs);
}

void Printer::operator()(const BodyFormula &operand) const {
    switch (operand.formula_type) {
        case BodyFormula::kAchieve:
            std::cout << "!";
            break;
        case BodyFormula::kAchieveLater:
            std::cout << "!!";
            break;
        case BodyFormula::kTest:
            std::cout << "?";
            break;
        case BodyFormula::kAdd:
            std::cout << "+";
            break;
        case BodyFormula::kRemove:
            std::cout << "-";
            break;
        case BodyFormula::kReplace:
            std::cout << "-+";
            break;
        case BodyFormula::kTerm:
            break;
    }

    boost::apply_visitor(*this, operand.formula);
}

void Printer::operator()(const Plan &operand) const {
    switch (operand.trigger_type) {
        case Plan::kAddition:
            std::cout << "+";
            break;
        case Plan::kRemoval:
            std::cout << "-";
            break;
    }

    switch (operand.goal_type) {
        case Plan::kAchievement:
            std::cout << "!";
            break;
        case Plan::kTest:
            std::cout << "?";
            break;
        case Plan::kBelief:
            break;
    }

    (*this)(operand.trigger);

    std::cout << " : ";

    boost::apply_visitor(*this, operand.context);

    std::cout << " <-" << std::endl;

    std::vector<BodyFormula>::const_iterator formula = operand.body.begin();
    while (formula != operand.body.end()) {
        std::cout << "    ";

        (*this)(*formula);

        formula++;
        if (formula != operand.body.end()) {
            std::cout << ";" << std::endl;
        }
    }
}

void Printer::operator()(const Agent &operand) const {
    for (const BeliefAtom &belief : operand.beliefs) {
        (*this)(belief);
        std::cout << "." << std::endl;
    }

    std::cout << std::endl;

    for (const Rule &rule : operand.rules) {
        (*this)(rule);
        std::cout << "." << std::endl;
    }

    std::cout << std::endl;

    // TODO: Maybe print intents for debugging.
    /*for(const BeliefAtom &goal : operand.goals) {
        std::cout << "!";
        (*this)(goal);
        std::cout << "." << std::endl;
    } */

    std::cout << std::endl;

    std::vector<Plan>::const_iterator it = operand.plans.begin();
    while (it != operand.plans.end()) {
        (*this)(*it);
        std::cout << "." << std::endl;

        it++;

        if (it != operand.plans.end()) {
            std::cout << std::endl;
        }
    }
}

}  // namespace as
