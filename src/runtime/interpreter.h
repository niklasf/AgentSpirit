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

#ifndef AS_RUNTIME_INTERPRETER_H_
#define AS_RUNTIME_INTERPRETER_H_

#include "../agent.h"
#include "environment.h"

namespace as {
namespace runtime {

/**
 * \brief The AgentSpeak interpreter.
 */
class Interpreter {
public:
    /**
     * \brief Runs one execution step of the given agent in the environment.
     *
     * \return true, if there is more work to do.
     */
    bool Run(Environment *env, as::Agent *agent);
};

}  // namespace runtime
}  // namespace as

#endif  // AS_RUNTIME_INTERPRETER_H_
