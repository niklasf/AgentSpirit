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

#ifndef AS_PRINTER_H_
#define AS_PRINTER_H_

#include "term.h"
#include "agent.h"

namespace as {

/**
 * \brief Visitor to write terms to the standard output
 */
struct Printer : boost::static_visitor<void> {
    void operator()(const bool &operand) const;
    void operator()(const double &operand) const;
    void operator()(const std::string &operand) const;
    void operator()(const std::vector<Term> &operand) const;
    void operator()(const BeliefAtom &operand) const;
    void operator()(const Variable &operand) const;
    void operator()(const UnaryOp<OpPositive> &operand) const;
    void operator()(const UnaryOp<OpNegative> &operand) const;
    void operator()(const UnaryOp<OpNot> &operand) const;
    void operator()(const BinaryOp<OpPlus> &operand) const;
    void operator()(const BinaryOp<OpMinus> &operand) const;
    void operator()(const BinaryOp<OpPow> &operand) const;
    void operator()(const BinaryOp<OpMultiply> &operand) const;
    void operator()(const BinaryOp<OpDivide> &operand) const;
    void operator()(const BinaryOp<OpDiv> &operand) const;
    void operator()(const BinaryOp<OpMod> &operand) const;
    void operator()(const BinaryOp<OpAnd> &operand) const;
    void operator()(const BinaryOp<OpOr> &operand) const;
    void operator()(const BinaryOp<OpUnify> &operand) const;
    void operator()(const BinaryOp<OpDeconstruct> &operand) const;
    void operator()(const BinaryOp<OpEq> &operand) const;
    void operator()(const BinaryOp<OpNeq> &operand) const;
    void operator()(const BinaryOp<OpLt> &operand) const;
    void operator()(const BinaryOp<OpLte> &operand) const;
    void operator()(const Rule &operand) const;
    void operator()(const BodyFormula &operand) const;
    void operator()(const Plan &operand) const;
    void operator()(const Agent &operand) const;
};

}  // namespace as

#endif  // AS_PRINTER_H_
