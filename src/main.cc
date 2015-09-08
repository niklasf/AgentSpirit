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

#include <iostream>
#include <fstream>
#include <string>

#include "term.h"
#include "agent.h"
#include "printer.h"
#include "runtime/environment.h"
#include "runtime/interpreter.h"
#include "grammar/agent_parser.h"
#include "grammar/comment_skipper.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cout << "// For now exactly one asl file is required!" << std::endl;
        return 1;
    }

    as::grammar::AgentParser<std::string::const_iterator, as::grammar::CommentSkipper<std::string::const_iterator> > parser;
    as::grammar::CommentSkipper<std::string::const_iterator> skipper;

    std::ifstream in(argv[1]);
    std::stringstream buffer;
    buffer << in.rdbuf();
    std::string sample = buffer.str();

    std::string::const_iterator it = sample.begin();
    std::string::const_iterator end = sample.end();

    as::Agent parsed;
    parsed.name = "uno";

    bool pass = boost::spirit::qi::phrase_parse(it, end, parser, skipper, parsed);
    if (pass && it == end) {
        std::cout << "// Parsed:" << std::endl;
        as::Printer()(parsed);
        std::cout << "// **********************************" << std::endl;
    } else {
        std::cout << "*** Parser error! ***" << std::endl;
        return 1;
    }

    as::runtime::Environment env;
    as::runtime::Interpreter interpreter;

    std::vector<as::Agent> agents(atoi(argv[2]), parsed);

    bool more_work;
    do {
        more_work = false;
        for (as::Agent &agent: agents) {
            more_work = interpreter.Run(&env, &agent) || more_work;
        }
    } while (more_work);

    return 0;
}
