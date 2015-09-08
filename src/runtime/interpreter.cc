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

#include "interpreter.h"

#include <string>
#include <memory>
#include <stdexcept>
#include <algorithm>

#include "../unification.h"
#include "../printer.h"
#include "logical_consequence.h"

namespace as {
namespace runtime {

bool Interpreter::Run(Environment *env, as::Agent *agent) {
    // No intentions left. Done.
    if (agent->intents.empty()) {
        return false;
    }

    // Get the current intention.
    std::deque<as::IntentionFrame> &intent = agent->intents.front();

    // Get the current intention frame. We're done with the current intention
    // if no intention frames remain.
    if (intent.empty()) {
        agent->intents.pop_front();
        return true;
    }
    as::IntentionFrame &frame = intent.front();

    if (frame.external) {
        // TODO: Test goals also may match beliefs, first.

        // Select an appropriate plan for the external event.
        for (const as::Plan &plan : agent->plans) {
            if (plan.trigger_type != frame.trigger_type) {
                continue;
            }

            if (plan.goal_type != frame.goal_type) {
                continue;
            }

            // Unify with the plan head.
            Unifier unifier;
            bool match = as::Unify(plan.trigger, frame.trigger, unifier);
            if (!match) {
                continue;
            }

            // Unify with the context.
            LogicalConsequence consequence(agent, env, unifier);
            std::unique_ptr<as::runtime::UnifierGeneratorInterface> gen(
                boost::apply_visitor(consequence, plan.context));
            if (!gen->Next()) {
                continue;
            }

            frame.unifier = gen->Current();
            frame.body_it = plan.body.begin();
            frame.body_end = plan.body.end();
            frame.external = false;
            break;
        }
    }

    if (frame.external) {
        // TODO: Warning: No plan found! Also emit failure events.
        agent->intents.pop_front();
        if (frame.goal_type == as::Plan::kAchievement) {
            std::cout << frame.trigger.functor << std::endl;
            throw std::runtime_error("no plan found");
        }
        return true;
    }

    if (frame.body_it == frame.body_end) {
        // Bring unifier information back if it is a test goal.
        as::Term completed_trigger(frame.unifier(frame.trigger));

        as::Anonymizer anon;
        completed_trigger = boost::apply_visitor(anon, completed_trigger);

        intent.pop_front();
        if (!intent.empty()) {
            as::Term completed_caller = boost::apply_visitor(intent.front().unifier, (intent.front().body_it - 1)->formula);
            as::Unify(completed_caller, completed_trigger, intent.front().unifier);
        }

        return true;
    }

    switch (frame.body_it->formula_type) {
        case as::BodyFormula::kTerm:
            {
                // Assert logical consequence.
                LogicalConsequence consequence(agent, env, frame.unifier);
                std::unique_ptr<UnifierGeneratorInterface> gen(boost::apply_visitor(consequence, frame.body_it->formula));
                if (gen->Next()) {
                    frame.unifier = gen->Current();
                } else {
                    // TODO: Handle assertion failure.
                    std::cout << "Assertion or action failure!" << std::endl;
                    Term substituted = boost::apply_visitor(frame.unifier, frame.body_it->formula);
                    as::Printer print;
                    boost::apply_visitor(print, substituted);
                    std::cout << std::endl;
                    agent->intents.pop_front();
                    return true;
                }
            }
            break;

        case as::BodyFormula::kReplace:
            {
                as::Unifies wildcard(frame.body_it->formula);
                agent->beliefs.erase(
                    std::remove_if(agent->beliefs.begin(), agent->beliefs.end(), wildcard),
                    agent->beliefs.end());
            }
            // Fall through to kAdd. TODO: Refactor?

        case as::BodyFormula::kAdd:
            {
                as::Term formula = boost::apply_visitor(frame.unifier, frame.body_it->formula);

                if (frame.body_it->formula.which() == as::kBeliefAtom) {
                    as::BeliefAtom belief = boost::get<BeliefAtom>(formula);

                    if (as::IsGround()(belief)) {
                        agent->beliefs.push_back(belief);
                        std::deque<as::IntentionFrame> event;
                        as::IntentionFrame frame(belief);
                        frame.goal_type = as::Plan::kBelief;
                        event.push_front(frame);
                        agent->intents.push_front(event);
                    } else {
                        throw std::runtime_error("only *ground* belief atoms can be added to the belief base");
                    }
                } else {
                    throw std::runtime_error("only belief atoms can be added to the belief base");
                }
            }
            break;

        case as::BodyFormula::kRemove:
            {
                as::Term formula = boost::apply_visitor(frame.unifier, frame.body_it->formula);
                as::Unifies predicate(formula);

                agent->beliefs.erase(
                    std::remove_if(agent->beliefs.begin(), agent->beliefs.end(), predicate),
                    agent->beliefs.end());
            }
            break;

        case as::BodyFormula::kAchieve:
            {
                as::Term formula = boost::apply_visitor(frame.unifier, frame.body_it->formula);
                as::Anonymizer anon;
                formula = boost::apply_visitor(anon, formula);

                if (frame.body_it->formula.which() == as::kBeliefAtom) {
                    as::BeliefAtom atom = boost::get<BeliefAtom>(formula);
                    intent.push_front(as::IntentionFrame(atom));
                } else {
                    throw std::runtime_error("tried to add non belief atom as achievement goal");
                }
            }
            break;

        case as::BodyFormula::kAchieveLater:
            // TODO: Implement.
            throw std::runtime_error("kAchieveLater not implemented");
            break;

        case as::BodyFormula::kTest:
            // TODO Implement.
            throw std::runtime_error("kTest not implemented");
            break;
    }

    frame.body_it++;
    return true;
}

}  // namespace runtime
}  // namespace as
