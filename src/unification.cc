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

#include "unification.h"

#include <vector>
#include <string>

namespace {

bool PrepareTerm(const std::unordered_map<std::string, as::Term> &unifier, as::Term &t) {
    switch (t.which()) {
        case as::kBool:
        case as::kDouble:
        case as::kString:
        case as::kBeliefAtom:
        case as::kList:
            return true;

        case as::kVariable:
            {
                as::Variable var = boost::get<as::Variable>(t);
                std::unordered_map<std::string, as::Term>::const_iterator it = unifier.find(var.name);
                if (it != unifier.end()) {
                    t = it->second;
                }
            }
            return true;

        default:
            return false;
    }
}

struct ContainsVariable : boost::static_visitor<bool> {
    explicit ContainsVariable(const std::string &variable_name)
        : variable_name(variable_name) {
    }

    bool operator()(const as::Variable &var) const {
        return var.name == variable_name;
    }

    bool operator()(const std::vector<as::Term> &list) const {
        for (const as::Term &term : list) {
            if (boost::apply_visitor(*this, term)) {
                return true;
            }
        }

        return false;
    }

    bool operator()(const as::BeliefAtom &atom) const {
        std::vector<as::Term>::const_iterator it = atom.terms.begin();

        while (it != atom.terms.end()) {
            if (boost::apply_visitor(*this, *it)) {
                return true;
            }

            ++it;
        }

        return false;
    }

    template <typename T>
    bool operator()(const T &) const {
        return false;
    }

    std::string variable_name;
};

bool UnifyLists(const std::vector<as::Term> &left, const std::vector<as::Term> &right, as::Unifier &unifier) {
    if (left.size() != right.size()) {
        return false;
    }

    std::vector<as::Term>::const_iterator left_it = left.begin();
    std::vector<as::Term>::const_iterator right_it = right.begin();

    // TODO: Think about this optimization and about
    // PrepareTerm and unifiability checks.
    if (left_it != left.end() && right_it != right.end()) {
        if (!Unify(*left_it, *right_it, unifier)) {
            return false;
        }

        ++left_it;
        ++right_it;
    }

    while (left_it != left.end() && right_it != right.end()) {
        as::Term lhs = boost::apply_visitor(unifier, *left_it);
        as::Term rhs = boost::apply_visitor(unifier, *right_it);

        if (!Unify(lhs, rhs, unifier)) {
            return false;
        }

        ++left_it;
        ++right_it;
    }

    return true;
}

}  // namespace


namespace as {

bool Unify(const as::Term &left, const as::Term &right, Unifier &unifier) {
    as::Term lhs = left;
    if (!PrepareTerm(unifier, lhs)) {
        return false;
    }

    Term rhs = right;
    if (!PrepareTerm(unifier, rhs)) {
        return false;
    }

    if (rhs.which() == kVariable && lhs.which() == kVariable) {
        Variable left_variable = boost::get<Variable>(lhs);
        Variable right_variable = boost::get<Variable>(rhs);

        if (left_variable.name == right_variable.name) {
            return true;
        } else if (left_variable.name < right_variable.name) {
            unifier[right_variable.name] = lhs;
            return true;
        } else {
            unifier[left_variable.name] = rhs;
            return true;
        }
    }

    if (rhs.which() == kVariable) {
        as::Variable var = boost::get<Variable>(rhs);

        // Use _ as a wildcard without contains check. Also do not save the
        // value so that it can be unified to different terms in the same
        // expression.
        if (var.name == "_") {
            return true;
        }

        if (boost::apply_visitor(ContainsVariable(var.name), lhs)) {
            return false;
        }

        unifier[var.name] = lhs;
        return true;
    }

    if (lhs.which() == kVariable) {
        as::Variable var = boost::get<as::Variable>(lhs);

        if (var.name == "_") {
            return true;
        }

        if (boost::apply_visitor(ContainsVariable(var.name), rhs)) {
            return false;
        }

        unifier[var.name] = rhs;
        return true;
    }

    if (lhs.which() != rhs.which()) {
        return false;
    }

    if (lhs.which() == kBool) {
        return boost::get<bool>(lhs) == boost::get<bool>(rhs);
    } else if (lhs.which() == kDouble) {
        return boost::get<double>(lhs) == boost::get<double>(rhs);
    } else if (lhs.which() == kString) {
        return boost::get<std::string>(lhs) == boost::get<std::string>(rhs);
    } else if (lhs.which() == kList) {
        return UnifyLists(
            boost::get<std::vector<Term> >(lhs),
            boost::get<std::vector<Term> >(rhs),
            unifier);
    } else {
        BeliefAtom left_atom = boost::get<BeliefAtom>(lhs);
        BeliefAtom right_atom = boost::get<BeliefAtom>(rhs);

        if (left_atom.functor != right_atom.functor) {
            return false;
        }

        return UnifyLists(left_atom.terms, right_atom.terms, unifier);
    }
}

}  // namespace as
