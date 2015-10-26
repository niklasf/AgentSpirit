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

#ifndef AS_AGENT_H_
#define AS_AGENT_H_

#include <vector>
#include <deque>

#include "term.h"
#include "unification.h"

namespace as {

/**
 * \brief A data structure for rules of an agent.
 */
struct Rule {
    BeliefAtom lhs;
    Term rhs;
};

/**
 * \brief A data structure for body formulas of an agent.
 */
struct BodyFormula {
    BodyFormula() : formula_type(kTerm), formula(true) {
    }

    enum FormulaType {
        kTerm,
        kTest,
        kAchieve,
        kAchieveLater,
        kAdd,
        kRemove,
        kReplace
    } formula_type;

    Term formula;
};

/**
 * \brief A data structure for plans of an agent.
 */
struct Plan {
    Plan() : context(true) {
    }

    enum TriggerType {
        kAddition,
        kRemoval
    } trigger_type;

    enum GoalType {
        kAchievement,
        kTest,
        kBelief
    } goal_type;

    BeliefAtom trigger;

    Term context;

    std::vector<BodyFormula> body;
};

/**
 * \brief An intention frame of an agent.
 */
struct IntentionFrame {
    explicit IntentionFrame(const BeliefAtom &trigger)
        : trigger_type(Plan::kAddition), goal_type(Plan::kAchievement), trigger(trigger), external(true) {
    }

    IntentionFrame() {
    }

    Plan::TriggerType trigger_type;
    Plan::GoalType goal_type;
    BeliefAtom trigger;
    std::vector<BodyFormula>::const_iterator body_it;
    std::vector<BodyFormula>::const_iterator body_end;
    Unifier unifier;
    bool external;
};

/**
 * \brief A data structure representing an agent and its state.
 */
struct Agent {
    std::string name;

    // TODO: Use a more appropriate container.
    std::vector<BeliefAtom> beliefs;

    std::deque<std::deque<IntentionFrame> > intents;

    std::vector<Rule> rules;

    std::vector<Plan> plans;

    void AddBelief(const BeliefAtom &belief) {
        beliefs.push_back(belief);

        IntentionFrame eventFrame(belief);
        eventFrame.goal_type = Plan::kBelief;

        intents.push_back({ eventFrame });
    }

    void RemoveBeliefs(const as::BeliefAtom &prototype) {
        RemoveBeliefs(as::Term(prototype));
    }

    void RemoveBeliefs(const as::Term &pattern) {
        as::Unifies condition(pattern);

        beliefs.erase(
            std::remove_if(beliefs.begin(), beliefs.end(), condition),
            beliefs.end());
    }
};

}  // namespace as

#endif  // AS_AGENT_H_
