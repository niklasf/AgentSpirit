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

#ifndef AS_GRAMMAR_BELIEF_ATOM_PARSER_H_
#define AS_GRAMMAR_BELIEF_ATOM_PARSER_H_

#define BOOST_RESULT_OF_USE_TR1
#define BOOST_SPIRIT_USE_PHOENIX_V3 1
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/phoenix/fusion/at.hpp>

#include "functor_parser.h"
#include "term_parser.h"

namespace as {
namespace grammar {

namespace qi = boost::spirit::qi;
namespace phoenix = boost::phoenix;

/**
 * \brief Parses belief atoms.
 *
 * Belief atoms have a functor and possibly one more terms directly following
 * in brackets. For example:
 *
 *  - foo
 *  - foo(bar)
 *  - foo(bar, baz)
 */
template <typename Iterator, typename Skipper>
class BeliefAtomParser : public qi::grammar<Iterator, as::BeliefAtom(), Skipper> {
public:
    BeliefAtomParser() : BeliefAtomParser::base_type(belief_atom_, "belief_atom") {
        using qi::_val;
        using qi::_1;
        using phoenix::at_c;

        belief_atom_ = functor_            [at_c<0>(_val) = _1]
                    >> -(    '('
                          >> (term_ % ',') [at_c<1>(_val) = _1]
                          >  ')'
                        );
    }

private:
    qi::rule<Iterator, as::BeliefAtom(), Skipper> belief_atom_;
    TermParser<Iterator, Skipper> term_;
    FunctorParser<Iterator> functor_;
};

}  // namespace grammar
}  // namespace as

#endif  // AS_GRAMMAR_BELIEF_ATOM_PARSER_H_
