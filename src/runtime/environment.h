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

#ifndef AS_RUNTIME_ENVIRONMENT_H_
#define AS_RUNTIME_ENVIRONMENT_H_

#include "../term.h"
#include "../unification.h"
#include "../agent.h"

namespace as {
namespace runtime {

class UnifierGeneratorInterface;

/**
 * \brief The default environment with the standard library of actions.
 *
 * Can be extended to provide scenario specific actions.
 */
class Environment {
public:
    virtual ~Environment() { }

    virtual UnifierGeneratorInterface *GetAction(as::Agent *agent, const as::BeliefAtom &action, const as::Unifier &unifier);
};

}  // namespace runtime
}  // namespace as

#endif  // AS_RUNTIME_ENVIRONMENT_H_
