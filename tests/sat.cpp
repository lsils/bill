/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*------------------------------------------------------------------------------------------------*/
#include <bill/sat/solver.hpp>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4365)
#pragma warning(disable:4514)
#pragma warning(disable:4571)
#pragma warning(disable:4619)
#pragma warning(disable:4623)
#pragma warning(disable:4625)
#pragma warning(disable:4626)
#pragma warning(disable:4710)
#pragma warning(disable:4711)
#pragma warning(disable:4820)
#pragma warning(disable:5026)
#pragma warning(disable:5027)
#pragma warning(disable:5039)
#include <catch.hpp>
#pragma warning(pop)
#endif

#include <iostream>
#include <vector>

using namespace bill;

template<typename Solver>
lit_type add_tseitin_and(Solver& solver, lit_type const& a, lit_type const& b)
{
	auto const r = solver.add_variable();
	solver.add_clause(std::vector{~a, ~b, lit_type(r, lit_type::polarities::positive)});
	solver.add_clause(std::vector{a, lit_type(r, lit_type::polarities::negative)});
	solver.add_clause(std::vector{b, lit_type(r, lit_type::polarities::negative)});
	return lit_type(r, lit_type::polarities::positive);
}

template<typename Solver>
lit_type add_tseitin_or(Solver& solver, lit_type const& a, lit_type const& b)
{
	auto const r = solver.add_variable();
	solver.add_clause(std::vector{a, b, lit_type(r, lit_type::polarities::negative)});
	solver.add_clause(std::vector{~a, lit_type(r, lit_type::polarities::positive)});
	solver.add_clause(std::vector{~b, lit_type(r, lit_type::polarities::positive)});
	return lit_type(r, lit_type::polarities::positive);
}

template<typename Solver>
lit_type add_tseitin_xor(Solver& solver, lit_type const& a, lit_type const& b)
{
	auto const r = solver.add_variable();
	solver.add_clause(std::vector{~a, ~b, lit_type(r, lit_type::polarities::negative)});
	solver.add_clause(std::vector{~a, b, lit_type(r, lit_type::polarities::positive)});
	solver.add_clause(std::vector{a, ~b, lit_type(r, lit_type::polarities::positive)});
	solver.add_clause(std::vector{a, b, lit_type(r, lit_type::polarities::negative)});
	return lit_type(r, lit_type::polarities::positive);
}

template<typename Solver>
lit_type add_tseitin_equals(Solver& solver, lit_type const& a, lit_type const& b)
{
	auto const r = solver.add_variable();
	solver.add_clause(std::vector{~a, ~b, lit_type(r, lit_type::polarities::positive)});
	solver.add_clause(std::vector{~a, b, lit_type(r, lit_type::polarities::negative)});
	solver.add_clause(std::vector{a, ~b, lit_type(r, lit_type::polarities::negative)});
	solver.add_clause(std::vector{a, b, lit_type(r, lit_type::polarities::positive)});
	return lit_type(r, lit_type::polarities::positive);
}

TEMPLATE_TEST_CASE("Simple SAT", "[sat][template]", solver<solvers::glucose_41>,
                   solver<solvers::ghack>, solver<solvers::maple>)
{
	TestType solver;
	auto const r = solver.solve();
	CHECK(r != result::states::unsatisfiable);
}

TEMPLATE_TEST_CASE("Simple UNSAT", "[sat][template]", solver<solvers::glucose_41>,
                   solver<solvers::ghack>, solver<solvers::maple>)
{
	TestType solver;

	auto const a = lit_type(solver.add_variable(), lit_type::polarities::positive);
	solver.add_clause(a);
	solver.add_clause(~a);

	auto const r = solver.solve();
	CHECK(r == result::states::unsatisfiable);
}

TEMPLATE_TEST_CASE("De Morgan", "[sat][template]", solver<solvers::glucose_41>,
                   solver<solvers::ghack>, solver<solvers::maple>)
{
	TestType solver;

	auto const a = lit_type(solver.add_variable(), lit_type::polarities::positive);
	auto const b = lit_type(solver.add_variable(), lit_type::polarities::positive);

	auto const t0 = add_tseitin_and(solver, a, b);
	auto const t1 = ~add_tseitin_or(solver, ~a, ~b);
	auto const t2 = add_tseitin_xor(solver, t0, t1);
	solver.add_clause(t2);

	auto const r = solver.solve();
	CHECK(r == result::states::unsatisfiable);
	CHECK(r != result::states::satisfiable);
}

TEMPLATE_TEST_CASE("Incremental", "[sat][template]", solver<solvers::glucose_41>,
                   solver<solvers::ghack>, solver<solvers::maple>)
{
	TestType solver;

	auto const a = lit_type(solver.add_variable(), lit_type::polarities::positive);
	auto const b = lit_type(solver.add_variable(), lit_type::polarities::positive);

	auto const t0 = add_tseitin_and(solver, a, b);
	auto r = solver.solve();
	CHECK(r == result::states::satisfiable);

	auto const t1 = ~add_tseitin_or(solver, ~a, ~b);
	r = solver.solve();
	CHECK(r == result::states::satisfiable);

	auto const t2 = add_tseitin_xor(solver, t0, t1);
	solver.add_clause(t2);
	r = solver.solve();
	CHECK(r == result::states::unsatisfiable);
}

TEMPLATE_TEST_CASE("Model", "[sat][template]", solver<solvers::glucose_41>, solver<solvers::ghack>,
                   solver<solvers::maple>)
{
	TestType solver;

	/* a and b have equal values */
	auto const a = lit_type(solver.add_variable(), lit_type::polarities::positive);
	auto const b = lit_type(solver.add_variable(), lit_type::polarities::positive);
	auto const t0 = add_tseitin_equals(solver, a, b);

	auto r = solver.solve({t0});
	CHECK(r == result::states::satisfiable);

	auto m = solver.get_model();
	auto model = m.model();
	CHECK(model.at(t0.variable()) == lbool_type::true_);
	CHECK(model.at(a.variable()) == model.at(b.variable()));

	/* a and b have un-equal values */
	auto const t1 = add_tseitin_xor(solver, a, b);

	r = solver.solve({t1});
	CHECK(r == result::states::satisfiable);

	m = solver.get_model();
	model = m.model();
	CHECK(model.at(t1.variable()) == lbool_type::true_);
	CHECK(model.at(a.variable()) != model.at(b.variable()));

	/* a and b are both true */
	auto const t2 = add_tseitin_and(solver, a, b);

	r = solver.solve({t2});
	CHECK(r == result::states::satisfiable);

	m = solver.get_model();
	model = m.model();
	CHECK(model.at(t2.variable()) == lbool_type::true_);
	CHECK(model.at(a.variable()) == lbool_type::true_);
	CHECK(model.at(b.variable()) == lbool_type::true_);

	/* at least one of a and b is true */
	auto const t3 = add_tseitin_or(solver, a, b);

	r = solver.solve({t3});
	CHECK(r == result::states::satisfiable);

	m = solver.get_model();
	model = m.model();
	CHECK(model.at(t3.variable()) == lbool_type::true_);

	bool const a_or_b_is_true = model.at(a.variable()) == lbool_type::true_
	                            | model.at(b.variable()) == lbool_type::true_;
	CHECK(a_or_b_is_true);
}
