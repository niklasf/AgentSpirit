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

#ifndef AS_GRAMMAR_TERM_PARSER_H_
#define AS_GRAMMAR_TERM_PARSER_H_

#define BOOST_RESULT_OF_USE_TR1
#define BOOST_SPIRIT_USE_PHOENIX_V3 1
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/qi_char_class.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/phoenix/fusion/at.hpp>
#include <boost/phoenix/function/adapt_callable.hpp>

#include "../term.h"
#include "unsigned_double_parser.h"
#include "functor_parser.h"

BOOST_FUSION_ADAPT_STRUCT(as::BeliefAtom,
    (std::string, functor)
    (std::vector<as::Term>, terms))

namespace as {
namespace grammar {

BOOST_PHOENIX_ADAPT_CALLABLE(pow, as::OpPow, 2)
BOOST_PHOENIX_ADAPT_CALLABLE(positive, as::OpPositive, 1)
BOOST_PHOENIX_ADAPT_CALLABLE(negative, as::OpNegative, 1)
BOOST_PHOENIX_ADAPT_CALLABLE(div, as::OpDiv, 2)
BOOST_PHOENIX_ADAPT_CALLABLE(mod, as::OpMod, 2)
BOOST_PHOENIX_ADAPT_CALLABLE(multiply, as::OpMultiply, 2)
BOOST_PHOENIX_ADAPT_CALLABLE(divide, as::OpDivide, 2)
BOOST_PHOENIX_ADAPT_CALLABLE(plus, as::OpPlus, 2)
BOOST_PHOENIX_ADAPT_CALLABLE(minus, as::OpMinus, 2)
BOOST_PHOENIX_ADAPT_CALLABLE(lt, as::OpLt, 2)
BOOST_PHOENIX_ADAPT_CALLABLE(lte, as::OpLte, 2)
BOOST_PHOENIX_ADAPT_CALLABLE(unifies, as::OpUnify, 2)
BOOST_PHOENIX_ADAPT_CALLABLE(deconstruct, as::OpDeconstruct, 2)
BOOST_PHOENIX_ADAPT_CALLABLE(eq, as::OpEq, 2)
BOOST_PHOENIX_ADAPT_CALLABLE(neq, as::OpNeq, 2)
BOOST_PHOENIX_ADAPT_CALLABLE(not_, as::OpNot, 1)
BOOST_PHOENIX_ADAPT_CALLABLE(and_, as::OpAnd, 2)
BOOST_PHOENIX_ADAPT_CALLABLE(or_, as::OpOr, 2)

namespace qi = boost::spirit::qi;
namespace phoenix = boost::phoenix;

template <typename Iterator, typename Skipper>
class TermParser : public qi::grammar<Iterator, as::Term(), Skipper> {
public:
    TermParser() : TermParser::base_type(expr_, "term") {
        using qi::alnum;
        using qi::lit;
        using qi::hex;
        using qi::upper;
        using qi::char_;
        using qi::bool_;
        using qi::_val;
        using qi::_1;
        using qi::_2;
        using qi::_a;
        using qi::_b;
        using qi::lexeme;

        using phoenix::at_c;

        variable_ = lexeme[char_('_') | *char_('_') >> upper >> *(alnum | char_('_'))];

        belief_atom_ = functor_            [at_c<0>(_val) = _1]
                    >> -(    '('
                          >> (expr_ % ',') [at_c<1>(_val) = _1]
                          >  ')'
                        );

        string_ = lexeme['"' >> *(escaped_char_ | "\\x" >> qi::hex | char_ - '"') >> '"'];

        escaped_char_.add("\\a", '\a');
        escaped_char_.add("\\b", '\b');
        escaped_char_.add("\\f", '\f');
        escaped_char_.add("\\n", '\n');
        escaped_char_.add("\\r", '\r');
        escaped_char_.add("\\t", '\t');
        escaped_char_.add("\\v", '\v');
        escaped_char_.add("\\\\", '\\');
        escaped_char_.add("\\\'", '\'');
        escaped_char_.add("\\\"", '\"');

        list_ = '['
             >> -(
                   expr_ % ','
                 )
             >> ']';

        atom_ = variable_            [_val = phoenix::construct<as::Variable>(_1)]
              | udouble_             [_val = _1]
              | bool_                [_val = _1]
              | string_              [_val = _1]
              | list_                [_val = _1]
              | belief_atom_         [_val = _1]
              | ('(' >> expr_ > ')') [_val = _1];

        power_ = atom_              [_val = _1]
              >> *( "**" >> factor_ [_val = pow(_val, _1)]
                  );

        factor_ = ('-' >> factor_ ) [_val = negative(_1)]
                | ('+' >> factor_ ) [_val = positive(_1)]
                | power_            [_val = _1];

        product_ = factor_               [_val = _1]
                >> *( ('*' >> factor_)   [_val = multiply(_val, _1)]
                    | ('/' >> factor_)   [_val = divide(_val, _1)]
                    | ("div" >> factor_) [_val = div(_val, _1)]
                    | ("mod" >> factor_) [_val = mod(_val, _1)]
                    );

        arith_expr_ = product_             [_val = _1]
                   >> *( ('+' >> product_) [_val = plus(_val, _1)]
                       | ('-' >> product_) [_val = minus(_val, _1)]
                       );

        less_ = lexeme['<' >> !lit('-')];

        comparison_ = arith_expr_                [_val = _1, _a = _1, _b = true]
                   >> *( ("<=" >> arith_expr_)   [_val = _b = and_(_b, lte(_a, _1)), _a = _1]
                       | (">=" >> arith_expr_)   [_val = _b = and_(_b, lte(_1, _a)), _a = _1]
                       | ("\\==" >> arith_expr_) [_val = _b = and_(_b, neq(_a, _1)), _a = _1]
                       | ("==" >> arith_expr_)   [_val = _b = and_(_b, eq(_a, _1)), _a = _1]
                       | ("=.." >> arith_expr_)  [_val = _b = and_(_b, deconstruct(_a, _1)), _a = _1]
                       | ('=' >> arith_expr_)    [_val = _b = and_(_b, unifies(_a, _1)), _a = _1]
                       | (less_ >> arith_expr_)  [_val = _b = and_(_b, lt(_a, _1)), _a = _1]
                       | ('>' >> arith_expr_)    [_val = _b = and_(_b, lt(_1, _a)), _a = _1]
                       );

        not_expr_ = ("not" >> not_expr_) [_val = not_(_1)]
                  | comparison_          [_val = _1];

        and_expr_ = not_expr_             [_val = _1]
                 >> *( '&' >> not_expr_ ) [_val = and_(_val, _1)];

        expr_ = and_expr_             [_val = _1]
             >> *( '|' >> and_expr_ ) [_val = or_(_val, _1)];

        variable_.name("variable");
        string_.name("string");
        list_.name("list");
        atom_.name("atom");
        belief_atom_.name("belief_atom");
        power_.name("power");
        factor_.name("factor");
        product_.name("product");
        arith_expr_.name("arith_expr");
        comparison_.name("comparison");
        not_expr_.name("not_expr");
        and_expr_.name("and_expr");
        expr_.name("expr");
    }

private:
    qi::rule<Iterator, std::string(), Skipper> variable_;
    qi::rule<Iterator, std::string(), Skipper> string_;
    qi::symbols<char const, char const> escaped_char_;
    qi::rule<Iterator, std::vector<as::Term>(), Skipper> list_;
    qi::rule<Iterator, as::BeliefAtom(), Skipper> belief_atom_;
    qi::rule<Iterator, as::Term(), Skipper> atom_;
    qi::rule<Iterator, as::Term(), Skipper> power_;
    qi::rule<Iterator, as::Term(), Skipper> factor_;
    qi::rule<Iterator, as::Term(), Skipper> product_;
    qi::rule<Iterator, as::Term(), Skipper> arith_expr_;
    qi::rule<Iterator> less_;
    qi::rule<Iterator, as::Term(), Skipper, qi::locals<as::Term, as::Term> > comparison_;
    qi::rule<Iterator, as::Term(), Skipper> not_expr_;
    qi::rule<Iterator, as::Term(), Skipper> and_expr_;
    qi::rule<Iterator, as::Term(), Skipper> expr_;
    FunctorParser<Iterator> functor_;
    UnsignedDoubleParser<Iterator> udouble_;
};

}  // namespace grammar
}  // namespace as

#endif  // AS_GRAMMAR_TERM_PARSER_H_
