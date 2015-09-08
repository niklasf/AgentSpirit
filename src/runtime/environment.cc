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

#include "environment.h"

#include <iostream>
#include <vector>
#include <stdexcept>

#include "../printer.h"
#include "../unification.h"
#include "logical_consequence.h"

namespace {

struct Printer : public boost::static_visitor<void> {
    void operator()(const std::string &str) const {
        std::cout << str;
    }

    template <typename T>
    void operator()(const T &term) const {
        as::Printer()(term);
    }
};

class PrintAction : public as::runtime::UnifierGeneratorInterface {
public:
    PrintAction(const as::BeliefAtom &action, as::Agent *agent, as::runtime::Environment *env, const as::Unifier &unifier)
        : terms_(action.terms), agent_(agent), env_(env), unifier_(unifier), once_(false) {
    }

    bool Next() override {
        if (!once_) {
            Printer print;

            for (const as::Term &term : terms_) {
                as::Term substituted(boost::apply_visitor(unifier_, term));
                boost::apply_visitor(print, substituted);
            }

            std::cout << std::endl;

            once_ = true;
            return true;
        }

        return false;
    }

    as::Unifier Current() const {
        return unifier_;
    }

private:
    as::Agent *agent_;
    as::runtime::Environment *env_;
    as::Unifier unifier_;

    std::vector<as::Term> terms_;
    bool once_;
};

class MyNameAction : public as::runtime::UnifierGeneratorInterface {
public:
    MyNameAction(const as::BeliefAtom &action, as::Agent *agent, as::runtime::Environment *env, const as::Unifier &unifier)
        : terms_(action.terms), agent_(agent), unifier_(unifier), once_(false) {
    }

    bool Next() override {
        if (!once_) {
            once_ = true;

            if (terms_.size() != 1) {
                throw std::runtime_error(".my_name expects exactly one argument");
            }

            as::Term arg = boost::apply_visitor(unifier_, terms_.front());
            return as::Unify(as::Term(agent_->name), arg, unifier_);
        }

        return false;
    }

    as::Unifier Current() const {
        return unifier_;
    }

private:
    as::Agent *agent_;
    as::Unifier unifier_;

    std::vector<as::Term> terms_;
    bool once_;
};

} // namespace

namespace as {
namespace runtime {

UnifierGeneratorInterface *Environment::GetAction(as::Agent *agent, const as::BeliefAtom &action, const as::Unifier &unifier) {
    if (action.functor == ".print") {
        return new PrintAction(action, agent, this, unifier);
    } else if (action.functor == ".my_name") {
        return new MyNameAction(action, agent, this, unifier);
    } else if (action.functor == ".fail") {
        return new as::runtime::UnifierGeneratorImpl<bool>(false, agent, this, unifier);
    } else {
        return nullptr;
    }
}

}  // namespace runtime
}  // namespace as
