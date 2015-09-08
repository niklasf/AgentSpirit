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

#ifndef AS_GRAMMAR_AGENT_PARSER_H_
#define AS_GRAMMAR_AGENT_PARSER_H_

#include <vector>
#include <deque>

#define BOOST_RESULT_OF_USE_TR1
#define BOOST_SPIRIT_USE_PHOENIX_V3 1
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/phoenix/fusion/at.hpp>

#include "../term.h"
#include "../agent.h"
#include "term_parser.h"
#include "belief_atom_parser.h"

BOOST_FUSION_ADAPT_STRUCT(as::Rule,
    (as::BeliefAtom, lhs)
    (as::Term, rhs))

BOOST_FUSION_ADAPT_STRUCT(as::BodyFormula,
    (as::BodyFormula::FormulaType, formula_type)
    (as::Term, formula))

BOOST_FUSION_ADAPT_STRUCT(as::Plan,
    (as::Plan::TriggerType, trigger_type)
    (as::Plan::GoalType, goal_type)
    (as::BeliefAtom, trigger)
    (as::Term, context)
    (std::vector<as::BodyFormula>, body))

BOOST_FUSION_ADAPT_STRUCT(as::Agent,
    (std::vector<as::BeliefAtom>, beliefs)
    (std::deque<std::deque<as::IntentionFrame> >, intents)
    (std::vector<as::Rule>, rules)
    (std::vector<as::Plan>, plans))

namespace as {
namespace grammar {

namespace qi = boost::spirit::qi;
namespace phoenix = boost::phoenix;

template <typename Iterator, typename Skipper>
class AgentParser : public qi::grammar<Iterator, as::Agent(), Skipper> {
public:
    AgentParser() : AgentParser::base_type(start_, "agent") {
        using qi::lit;
        using qi::_val;
        using qi::_pass;
        using qi::eps;
        using qi::_1;
        using qi::_2;
        using qi::_3;
        using qi::_4;

        using phoenix::at_c;
        using phoenix::val;

        rule_ = belief_atom_ [at_c<0>(_val) = _1, phoenix::bind(&AgentParser::CheckIsUnifiable, this, _1, _pass)]
             >> ":-"
             >> term_        [at_c<1>(_val) = _1, phoenix::bind(&AgentParser::CheckIsValidContext, this, _1, _pass)];

        trigger_type_ = lit('+') [_val = as::Plan::kAddition]
                      | lit('-') [_val = as::Plan::kRemoval];

        goal_type_ = lit('!') [_val = as::Plan::kAchievement]
                   | lit('?') [_val = as::Plan::kTest]
                   | eps      [_val = as::Plan::kBelief];

        formula_type_ = lit('?')  [_val = as::BodyFormula::kTest]
                      | lit("!!") [_val = as::BodyFormula::kAchieveLater]
                      | lit('!')  [_val = as::BodyFormula::kAchieve]
                      | lit('+')  [_val = as::BodyFormula::kAdd]
                      | lit("-+") [_val = as::BodyFormula::kReplace]
                      | lit('-')  [_val = as::BodyFormula::kRemove]
                      | eps       [_val = as::BodyFormula::kTerm];

        // TODO: Most formula types require a variable or belief atom. Terms
        // are either actions or boolean expressions. Investigate requirements
        // for the -+ operator.
        body_formula_ = formula_type_ [at_c<0>(_val) = _1]
                     >> term_         [at_c<1>(_val) = _1];

        plan_ = trigger_type_                   [at_c<0>(_val) = _1]
             >> goal_type_                      [at_c<1>(_val) = _1]
             >> belief_atom_                    [at_c<2>(_val) = _1]
             >> ( ':' >> term_                  [at_c<3>(_val) = _1, phoenix::bind(&AgentParser::CheckIsValidContext, this, _1, _pass)]
                | eps                           [at_c<3>(_val) = true]
                )
             >> ( "<-" >> (body_formula_ % ';') [at_c<4>(_val) = _1]
                | eps                           [phoenix::push_back(at_c<4>(_val), as::BodyFormula())]
                );

        start_ = *( ( rule_               [phoenix::push_back(at_c<2>(_val), _1)]
                    | belief_atom_        [phoenix::bind(&AgentParser::AddBelief, this, _val, _1, _pass)]
                    | '!' >> belief_atom_ [phoenix::bind(&AgentParser::AddGoal, this, _val, _1)]
                    | plan_               [phoenix::push_back(at_c<3>(_val), _1)]
                    )
                    > '.'
                  );

        rule_.name("rule");
        trigger_type_.name("trigger_type");
        goal_type_.name("goal_type");
        formula_type_.name("formula_type");
        body_formula_.name("body_formula");
        plan_.name("plan");

        qi::on_error<qi::fail>(start_,
            std::cout
                << val("Error! Expecting ")
                << _4
                << val(" here: \"")
                << phoenix::construct<std::string>(_3, _2)
                << val("\"")
                << std::endl);
    }

private:
    void AddBelief(as::Agent &val, const as::BeliefAtom &belief, bool &pass) {
        if (as::IsGround()(belief)) {
            // Just add the base belief.
            val.beliefs.push_back(belief);
        } else if (as::IsUnifiable()(belief)) {
            // If there is an ungrounded belief, this is actually an implicit
            // rule, assigning true to the term for all unifications.
            as::Rule implicit_rule;
            implicit_rule.lhs = belief;
            implicit_rule.rhs = true;
            val.rules.push_back(implicit_rule);
        } else {
            pass = false;
        }
    }

    void CheckIsUnifiable(const as::Term &lhs, bool &pass) {
        if (!boost::apply_visitor(as::IsUnifiable(), lhs)) {
            pass = false;
        }
    }

    void CheckIsValidContext(const as::Term &context, bool &pass) {
        if (!boost::apply_visitor(as::IsValidContext(), context)) {
            pass = false;
        }
    }

    void AddGoal(as::Agent &val, const BeliefAtom &trigger) {
        val.intents.push_back(std::deque<as::IntentionFrame>());
        val.intents.back().push_back(as::IntentionFrame(trigger));
    }

    qi::rule<Iterator, as::Rule(), Skipper> rule_;
    qi::rule<Iterator, as::Plan::TriggerType(), Skipper> trigger_type_;
    qi::rule<Iterator, as::Plan::GoalType(), Skipper> goal_type_;
    qi::rule<Iterator, as::BodyFormula::FormulaType(), Skipper> formula_type_;
    qi::rule<Iterator, as::BodyFormula(), Skipper> body_formula_;
    qi::rule<Iterator, as::Plan(), Skipper> plan_;
    qi::rule<Iterator, as::Agent(), Skipper> start_;
    TermParser<Iterator, Skipper> term_;
    BeliefAtomParser<Iterator, Skipper> belief_atom_;
};

}  // namespace grammar
}  // namespace as

#endif  // AS_GRAMMAR_AGENT_PARSER_H_
