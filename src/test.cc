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

#define BOOST_TEST_MODULE AgentSpirit
#include <boost/test/included/unit_test.hpp>

#include <sstream>

#include <boost/variant/get.hpp>

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include "term.h"
#include "unification.h"
#include "serialization.h"
#include "grammar/term_parser.h"
#include "grammar/comment_skipper.h"
#include "runtime/logical_consequence.h"

as::Term parse_term(const std::string &str) {
    as::Term t;

    as::grammar::TermParser<std::string::const_iterator, as::grammar::CommentSkipper<std::string::const_iterator> > parser;
    as::grammar::CommentSkipper<std::string::const_iterator> skipper;

    std::string::const_iterator it = str.begin();
    std::string::const_iterator end = str.end();

    boost::spirit::qi::phrase_parse(it, end, parser, skipper, t);
    return t;
}

BOOST_AUTO_TEST_CASE(test_unification) {
    as::Term f_aX = parse_term("f(a, X)");
    as::Term f_ab = parse_term("f(a, true)");
    as::Term X = as::Variable("X");

    {
        as::Unifier unifier;
        BOOST_CHECK(as::Unify(f_aX, f_ab, unifier));
        BOOST_CHECK_EQUAL(unifier.size(), 1);
        BOOST_CHECK_EQUAL(boost::get<bool>(unifier["X"]), true);
    }

    {
        as::Unifier unifier;
        BOOST_CHECK(!as::Unify(f_aX, X, unifier));
    }
}

BOOST_AUTO_TEST_CASE(test_numeric_operators) {
    as::Term one(1.0);
    as::Term two(2.0);

    as::Term expression = as::OpLt()(
        as::OpPlus()(one, two),
        as::OpPow()(one, two));

    BOOST_CHECK_EQUAL(boost::get<bool>(expression), false);
}

BOOST_AUTO_TEST_CASE(test_belief_atom_comparison) {
    as::BeliefAtom left_foo;
    left_foo.functor = "foo";
    left_foo.terms.push_back(as::Term(1.0));
    left_foo.terms.push_back(as::Term(false));

    as::BeliefAtom right_foo;
    right_foo.functor = "foo";
    right_foo.terms.push_back(as::Term(1.0));
    right_foo.terms.push_back(as::Term(true));

    BOOST_CHECK_EQUAL(boost::get<bool>(as::OpEq()(left_foo, right_foo)), false);
    BOOST_CHECK_EQUAL(boost::get<bool>(as::OpLt()(left_foo, right_foo)), true);
    BOOST_CHECK_EQUAL(boost::get<bool>(as::OpLte()(left_foo, right_foo)), true);
    BOOST_CHECK_EQUAL(boost::get<bool>(as::OpLte()(right_foo, left_foo)), false);
}

BOOST_AUTO_TEST_CASE(test_ungrounded_belief_atom_equality) {
    as::BeliefAtom t_of_X;
    t_of_X.functor = "t";
    t_of_X.terms.push_back(as::Variable("X"));

    as::BeliefAtom t_of_five;
    t_of_five.functor = "t";
    t_of_five.terms.push_back(as::Term(5.0));

    as::Term result = as::OpEq()(t_of_X, t_of_five);

    as::BinaryOp<as::OpEq> equality = boost::get<as::BinaryOp<as::OpEq> >(result);
    BOOST_CHECK_EQUAL(boost::get<double>(equality.right), 5.0);
}

BOOST_AUTO_TEST_CASE(test_bool_comparison) {
    as::Term yeah(true);
    as::Term nope(false);

    BOOST_CHECK_EQUAL(boost::get<bool>(as::OpEq()(yeah, yeah)), true);
    BOOST_CHECK_EQUAL(boost::get<bool>(as::OpEq()(yeah, nope)), false);
}

BOOST_AUTO_TEST_CASE(test_logical_consequence) {
    as::Unifier empty_unifier;
    as::Agent empty_agent;
    as::runtime::Environment env;
    as::runtime::LogicalConsequence global_consequence(&empty_agent, &env, empty_unifier);

    {
        std::unique_ptr<as::runtime::UnifierGeneratorInterface> gen(global_consequence(false));
        BOOST_CHECK_EQUAL(gen->Next(), false);
    }

    {
        std::unique_ptr<as::runtime::UnifierGeneratorInterface> gen(global_consequence(true));
        BOOST_CHECK_EQUAL(gen->Next(), true);
        BOOST_CHECK_EQUAL(gen->Next(), false);
    }

    {
        as::BeliefAtom f_X;
        f_X.functor = "f";
        as::Variable X("X");
        f_X.terms.push_back(X);

        std::unique_ptr<as::runtime::UnifierGeneratorInterface> empty_gen(global_consequence(f_X));
        BOOST_CHECK_EQUAL(empty_gen->Next(), false);

        as::Agent agent;
        as::runtime::LogicalConsequence consequence(&agent, &env, empty_unifier);

        as::BeliefAtom f_5;
        f_5.functor = "f";
        f_5.terms.push_back(5.0);
        agent.beliefs.push_back(f_5);

        std::unique_ptr<as::runtime::UnifierGeneratorInterface> gen(consequence(f_X));
        BOOST_CHECK_EQUAL(gen->Next(), true);
        BOOST_CHECK_EQUAL(boost::get<double>(gen->Current()["X"]), 5);
        BOOST_CHECK_EQUAL(gen->Next(), false);
    }

    {
        as::Variable X("X");

        std::unique_ptr<as::runtime::UnifierGeneratorInterface> gen(global_consequence(X));
        BOOST_CHECK_EQUAL(gen->Next(), true);
        BOOST_CHECK_EQUAL(boost::get<bool>(gen->Current()["X"]), true);
        BOOST_CHECK_EQUAL(gen->Next(), false);
    }

    {
        as::Agent agent;
        as::runtime::LogicalConsequence consequence(&agent, &env, empty_unifier);

        as::BeliefAtom f;
        f.functor = "f";
        agent.beliefs.push_back(f);

        as::Variable Y("Y");

        std::unique_ptr<as::runtime::UnifierGeneratorInterface> gen(consequence(Y));
        BOOST_CHECK_EQUAL(gen->Next(), true);
        BOOST_CHECK_EQUAL(boost::get<bool>(gen->Current()["Y"]), true);
        BOOST_CHECK_EQUAL(gen->Next(), true);
        BOOST_CHECK_EQUAL((boost::get<as::BeliefAtom>(gen->Current()["Y"])).functor, "f");
        BOOST_CHECK_EQUAL(gen->Next(), false);
    }
}

BOOST_AUTO_TEST_CASE(test_serialize_term) {
    std::stringstream buffer;

    {
        as::BeliefAtom a;
        a.functor = "a";

        as::BeliefAtom b;
        b.functor = "b";

        std::vector<as::Term> list;
        list.push_back(a);
        list.push_back(b);

        as::BeliefAtom f;
        f.functor = "f";
        f.terms.push_back(2.0);
        f.terms.push_back(true);
        f.terms.push_back(std::string("str"));
        f.terms.push_back(list);

        as::Term term(f);

        boost::archive::text_oarchive oa(buffer);
        oa << term;
    }

    {
        as::Term term;

        boost::archive::text_iarchive ia(buffer);
        ia >> term;

        as::BeliefAtom f(boost::get<as::BeliefAtom>(term));
        BOOST_CHECK_EQUAL(f.functor, "f");
        BOOST_CHECK_EQUAL(f.terms.size(), 4);
        BOOST_CHECK_EQUAL(boost::get<double>(f.terms[0]), 2.0);
        BOOST_CHECK_EQUAL(boost::get<bool>(f.terms[1]), true);
        BOOST_CHECK_EQUAL(boost::get<std::string>(f.terms[2]), "str");

        std::vector<as::Term> list(boost::get<std::vector<as::Term> >(f.terms[3]));
        BOOST_CHECK_EQUAL(list.size(), 2);
        BOOST_CHECK_EQUAL((boost::get<as::BeliefAtom>(list[0])).functor, "a");
        BOOST_CHECK_EQUAL((boost::get<as::BeliefAtom>(list[1])).functor, "b");
    }
}

BOOST_AUTO_TEST_CASE(test_chained_unification) {
    as::Variable A("A");

    as::Variable B("B");

    as::BeliefAtom c;
    c.functor = "c";

    as::Term term = as::OpAnd()(as::OpUnify()(A, B), as::OpUnify()(B, c));

    as::Agent agent;
    as::runtime::Environment env;
    as::Unifier unifier;

    std::unique_ptr<as::runtime::UnifierGeneratorInterface> gen(
        boost::apply_visitor(as::runtime::LogicalConsequence(&agent, &env, unifier), term));
    BOOST_CHECK_EQUAL(gen->Next(), true);

    as::Term A_unif = gen->Current()(A);
    BOOST_CHECK_EQUAL(A_unif.which(), as::kBeliefAtom);

    as::Term B_unif = gen->Current()(B);
    BOOST_CHECK_EQUAL(B_unif.which(), as::kBeliefAtom);
}

BOOST_AUTO_TEST_CASE(test_op_unify) {
    as::Variable X("X");
    as::Variable Y("Y");

    as::Term term = as::OpUnify()(X, as::OpPlus()(Y, as::Term(1.0)));
    BOOST_CHECK_EQUAL(term.which(), as::kUnify);
}

BOOST_AUTO_TEST_CASE(test_ordered_unification) {
    as::Variable X("X");

    std::vector<as::Term> ground_list;
    ground_list.push_back(1.0);
    ground_list.push_back(2.0);

    {
        // In this order the implementation will stumble upon the unification
        // for X + 1.
        std::vector<as::Term> list;
        list.push_back(X);
        list.push_back(as::OpPlus()(X, as::Term(1.0)));

        as::Unifier unifier;
        BOOST_CHECK_EQUAL(Unify(ground_list, list, unifier), true);
    }

    {
        // In this order X - 1 is just not unifiable.
        std::vector<as::Term> list;
        list.push_back(as::OpMinus()(X, as::Term(1.0)));
        list.push_back(X);

        as::Unifier unifier;
        BOOST_CHECK_EQUAL(Unify(ground_list, list, unifier), false);
    }
}
