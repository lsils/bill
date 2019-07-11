/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*------------------------------------------------------------------------------------------------*/

#include "catch2.hpp"

#include <bill/sat/solver.hpp>
#include <bill/sat/tseytin.hpp>

#include <iostream>
#include <vector>

using namespace bill;

#if defined(BILL_WINDOWS_PLATFORM)
#define SOLVER_TYPES solver<solvers::ghack>
#else
#define SOLVER_TYPES solver<solvers::glucose_41>,solver<solvers::ghack>,solver<solvers::maple>
#endif

TEMPLATE_TEST_CASE("Simple SAT", "[sat][template]", SOLVER_TYPES)
{
	TestType solver;
	auto const r = solver.solve();
	CHECK(r != result::states::unsatisfiable);
}

TEMPLATE_TEST_CASE("Simple UNSAT", "[sat][template]", SOLVER_TYPES)
{
	TestType solver;

	auto const a = lit_type(solver.add_variable(), lit_type::polarities::positive);
	solver.add_clause(a);
	solver.add_clause(~a);

	auto const r = solver.solve();
	CHECK(r == result::states::unsatisfiable);
}

TEMPLATE_TEST_CASE("De Morgan", "[sat][template]", SOLVER_TYPES)
{
	TestType solver;

	auto const a = lit_type(solver.add_variable(), lit_type::polarities::positive);
	auto const b = lit_type(solver.add_variable(), lit_type::polarities::positive);

	auto const t0 = add_tseytin_and(solver, a, b);
	auto const t1 = ~add_tseytin_or(solver, ~a, ~b);
	auto const t2 = add_tseytin_xor(solver, t0, t1);
	solver.add_clause(t2);

	auto const r = solver.solve();
	CHECK(r == result::states::unsatisfiable);
	CHECK(r != result::states::satisfiable);
}

TEMPLATE_TEST_CASE("Incremental", "[sat][template]", SOLVER_TYPES)
{
	TestType solver;

	auto const a = lit_type(solver.add_variable(), lit_type::polarities::positive);
	auto const b = lit_type(solver.add_variable(), lit_type::polarities::positive);

	auto const t0 = add_tseytin_and(solver, a, b);
	auto r = solver.solve();
	CHECK(r == result::states::satisfiable);

	auto const t1 = ~add_tseytin_or(solver, ~a, ~b);
	r = solver.solve();
	CHECK(r == result::states::satisfiable);

	auto const t2 = add_tseytin_xor(solver, t0, t1);
	solver.add_clause(t2);
	r = solver.solve();
	CHECK(r == result::states::unsatisfiable);
}

TEMPLATE_TEST_CASE("Model", "[sat][template]", SOLVER_TYPES)
{
	TestType solver;

	/* a and b have equal values */
	auto const a = lit_type(solver.add_variable(), lit_type::polarities::positive);
	auto const b = lit_type(solver.add_variable(), lit_type::polarities::positive);
	auto const t0 = add_tseytin_equals(solver, a, b);

	auto r = solver.solve({t0});
	CHECK(r == result::states::satisfiable);

	auto m = solver.get_model();
	auto model = m.model();
	CHECK(model.at(t0.variable()) == lbool_type::true_);
	CHECK(model.at(a.variable()) == model.at(b.variable()));

	/* a and b have un-equal values */
	auto const t1 = add_tseytin_xor(solver, a, b);

	r = solver.solve({t1});
	CHECK(r == result::states::satisfiable);

	m = solver.get_model();
	model = m.model();
	CHECK(model.at(t1.variable()) == lbool_type::true_);
	CHECK(model.at(a.variable()) != model.at(b.variable()));

	/* a and b are both true */
	auto const t2 = add_tseytin_and(solver, a, b);

	r = solver.solve({t2});
	CHECK(r == result::states::satisfiable);

	m = solver.get_model();
	model = m.model();
	CHECK(model.at(t2.variable()) == lbool_type::true_);
	CHECK(model.at(a.variable()) == lbool_type::true_);
	CHECK(model.at(b.variable()) == lbool_type::true_);

	/* at least one of a and b is true */
	auto const t3 = add_tseytin_or(solver, a, b);

	r = solver.solve({t3});
	CHECK(r == result::states::satisfiable);

	m = solver.get_model();
	model = m.model();
	CHECK(model.at(t3.variable()) == lbool_type::true_);

	bool const a_or_b_is_true = model.at(a.variable()) == lbool_type::true_
	                            | model.at(b.variable()) == lbool_type::true_;
	CHECK(a_or_b_is_true);
}
