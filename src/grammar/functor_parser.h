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

#ifndef AS_GRAMMAR_FUNCTOR_PARSER_H_
#define AS_GRAMMAR_FUNCTOR_PARSER_H_

#include <string>

#define BOOST_RESULT_OF_USE_TR1
#define BOOST_SPIRIT_USE_PHOENIX_V3 1
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/qi_char_class.hpp>

namespace as {
namespace grammar {

namespace qi = boost::spirit::qi;

/**
 * \brief Parses functors.
 *
 * Functors are used in belief atoms or actions or for names of plans.
 * A functor can start with a "~" and contain "_", and alphanumeric characters.
 * The first real character must be a lower case letter.
 */
template <typename Iterator>
class FunctorParser : public qi::grammar<Iterator, std::string()> {
public:
    FunctorParser() : FunctorParser::base_type(start_, "functor") {
        using qi::lower;
        using qi::char_;
        using qi::raw;
        using qi::lit;
        using qi::bool_;

        start_ = raw[-char_('~') >> -char_('.') >> lower >> *(char_("a-zA-Z0-9_") | (char_('.') >> char_("a-zA-Z0-9_")))]
               - lit("not")
               - lit("div")
               - lit("mod")
               - bool_;
    }

private:
    qi::rule<Iterator, std::string()> start_;
};

}  // namespace grammar
}  // namespace as

#endif  // AS_GRAMMAR_FUNCTOR_PARSER_H_
