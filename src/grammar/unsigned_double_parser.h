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

#ifndef AS_GRAMMAR_UNSIGNED_DOUBLE_PARSER_
#define AS_GRAMMAR_UNSIGNED_DOUBLE_PARSER_

#include <cstdlib>
#include <string>

#define BOOST_RESULT_OF_USE_TR1
#define BOOST_SPIRIT_USE_PHOENIX_V3 1
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/qi_char_class.hpp>
#include <boost/spirit/include/phoenix.hpp>

namespace as {
namespace grammar {

namespace qi = boost::spirit::qi;
namespace phoenix = boost::phoenix;

/**
 * \brief Parses a literal for a double value.
 *
 * Double literals look just like C-style doubles, but with the following
 * differences. This is why boost::spirit::qi::double_ can not be used.
 *
 *  - There can be no leading plus or minus sign.
 *  - After the optional decimal point "." at least one digit is required.
 */
template <typename Iterator>
class UnsignedDoubleParser : public qi::grammar<Iterator, double()> {
public:
    UnsignedDoubleParser() : UnsignedDoubleParser::base_type(start_, "unsigned_double") {
        using qi::_val;
        using qi::_1;
        using qi::raw;
        using qi::digit;
        using qi::char_;

        repr_ = raw[(*digit >> '.' >> +digit | +digit) >> -(char_("eE") >> -char_("+-") >> +digit)];

        start_ = repr_ [_val = phoenix::bind(&UnsignedDoubleParser::ToDouble, this, _1)];
    }

private:
    double ToDouble(const std::string &str) const {
        return std::atof(str.c_str());
    }

    qi::rule<Iterator, std::string()> repr_;
    qi::rule<Iterator, double()> start_;
};

}  // namespace grammar
}  // namespace as

#endif  // AS_GRAMMAR_UNSIGNED_DOUBLE_PARSER_
