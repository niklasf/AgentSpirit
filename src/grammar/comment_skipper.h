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

#ifndef AS_GRAMMAR_COMMENT_SKIPPER_H_
#define AS_GRAMMAR_COMMENT_SKIPPER_H_

#define BOOST_RESULT_OF_USE_TR1
#define BOOST_SPIRIT_USE_PHOENIX_V3 1
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/qi_char_class.hpp>

namespace as {
namespace grammar {

namespace qi = boost::spirit::qi;

/**
 * \brief Skips whitespace and comments
 *
 * Skipper for any kind of whitespace and C-style multiline or inline comments.
 */
template <typename Iterator>
class CommentSkipper : public qi::grammar<Iterator> {
public:
    CommentSkipper() : CommentSkipper::base_type::base_type(skip_) {
        using qi::space;
        using qi::char_;

        skip_ = space
              | ("/*" >> *(char_ - "*/") >> "*/")
              | ("//" >> *(char_ - char_("\r\n")));
    }

private:
    qi::rule<Iterator> skip_;
};

}  // namespace grammar
}  // namespace as

#endif  // AS_GRAMMAR_COMMENT_SKIPPER_H_
