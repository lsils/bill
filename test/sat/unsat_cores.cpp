#include "../catch2.hpp"

#include <bill/sat/solver.hpp>

#if defined(BILL_WINDOWS_PLATFORM)
#define SOLVER_TYPES bill::solver<bill::solvers::glucose_41>, bill::solver<bill::solvers::ghack>
#else
#define SOLVER_TYPES bill::solver<bill::solvers::glucose_41>, bill::solver<bill::solvers::ghack>, bill::solver<bill::solvers::maple>
#endif

#include <bill/sat/unsat_cores.hpp>
#include <vector>

using namespace bill;

TEMPLATE_TEST_CASE("Get UNSAT core", "[sat][template]", SOLVER_TYPES)
{
	TestType solver;

	auto const x1 = lit_type(solver.add_variable(), lit_type::polarities::positive);
	auto const x2 = lit_type(solver.add_variable(), lit_type::polarities::positive);
	auto const x3 = lit_type(solver.add_variable(), lit_type::polarities::positive);
	auto const x4 = lit_type(solver.add_variable(), lit_type::polarities::positive);
	auto const x5 = lit_type(solver.add_variable(), lit_type::polarities::positive);
	auto const x6 = lit_type(solver.add_variable(), lit_type::polarities::positive);
	auto const x7 = lit_type(solver.add_variable(), lit_type::polarities::positive);
	auto const x8 = lit_type(solver.add_variable(), lit_type::polarities::positive);
	auto const x9 = lit_type(solver.add_variable(), lit_type::polarities::positive);

	/* Example CNF from ''On Computing Minimum Unsatisfiable Cores'',
	 * SAT 2004: we use assumption literals 4,...,9 for clause activation */
	solver.add_clause({~x4, x1, ~x3});
	solver.add_clause({~x5, x2});
	solver.add_clause({~x6, ~x2, x3});
	solver.add_clause({~x7, ~x2, ~x3});
	solver.add_clause({~x8, x2, x3});
	solver.add_clause({~x9, ~x1, x2, ~x3});

	auto r = solver.solve({});
	CHECK(r == result::states::satisfiable);

	r = solver.solve({x4, x5, x6, x7, x8, x9});
	CHECK(r == result::states::unsatisfiable);

	auto counter = 0u;
	result::clause_type core = solver.get_core().core();
	for (const auto& l : core) {
		switch (counter++) {
		case 0: {
			CHECK(l.variable() == x7.variable());
		} break;
		case 1: {
			CHECK(l.variable() == x6.variable());
		} break;
		case 2: {
			CHECK(l.variable() == x5.variable());
		} break;
		default:
			/* unreachable */
			CHECK(false);
			break;
		}
	}

	r = solver.solve({x5, x6, x7});
	CHECK(r == result::states::unsatisfiable);

	r = solver.solve({x4, x8, x9});
	CHECK(r == result::states::satisfiable);
}

TEMPLATE_TEST_CASE("UNSAT core extraction", "[sat][template]", SOLVER_TYPES)
{
	TestType solver;

	auto const x1 = lit_type(solver.add_variable(), lit_type::polarities::positive);
	auto const x2 = lit_type(solver.add_variable(), lit_type::polarities::positive);
	auto const x3 = lit_type(solver.add_variable(), lit_type::polarities::positive);
	auto const x4 = lit_type(solver.add_variable(), lit_type::polarities::positive);
	auto const x5 = lit_type(solver.add_variable(), lit_type::polarities::positive);
	auto const x6 = lit_type(solver.add_variable(), lit_type::polarities::positive);
	auto const x7 = lit_type(solver.add_variable(), lit_type::polarities::positive);
	auto const x8 = lit_type(solver.add_variable(), lit_type::polarities::positive);
	auto const x9 = lit_type(solver.add_variable(), lit_type::polarities::positive);

	/* Example CNF from ''On Computing Minimum Unsatisfiable Cores'',
	 * SAT 2004: we use assumption literals 4,...,9 for clause activation */
	solver.add_clause({~x4, x1, ~x3});
	solver.add_clause({~x5, x2});
	solver.add_clause({~x6, ~x2, x3});
	solver.add_clause({~x7, ~x2, ~x3});
	solver.add_clause({~x8, x2, x3});
	solver.add_clause({~x9, ~x1, x2, ~x3});

	CHECK(solver.solve({x4, x5, x6, x7, x8, x9}) == result::states::unsatisfiable); /* UC1 */
	CHECK(solver.solve({x4, x5, x6, x7, x8}) == result::states::unsatisfiable);     /* UC2 */
	CHECK(solver.solve({x4, x5, x6, x7, x9}) == result::states::unsatisfiable);     /* UC3 */
	CHECK(solver.solve({x4, x6, x7, x8, x9}) == result::states::unsatisfiable);     /* UC4 */
	CHECK(solver.solve({x5, x6, x7, x8, x9}) == result::states::unsatisfiable);     /* UC5 */
	CHECK(solver.solve({x4, x5, x6, x7}) == result::states::unsatisfiable);         /* UC6 */
	CHECK(solver.solve({x5, x6, x7, x8}) == result::states::unsatisfiable);         /* UC7 */
	CHECK(solver.solve({x5, x6, x7, x9}) == result::states::unsatisfiable);         /* UC8 */
	CHECK(solver.solve({x5, x6, x7}) == result::states::unsatisfiable);             /* UC9 */
}

TEMPLATE_TEST_CASE("Trim UNSAT core", "[sat][template]", SOLVER_TYPES)
{
	TestType solver;

	auto const x1 = lit_type(solver.add_variable(), lit_type::polarities::positive);
	auto const x2 = lit_type(solver.add_variable(), lit_type::polarities::positive);
	auto const x3 = lit_type(solver.add_variable(), lit_type::polarities::positive);
	auto const x4 = lit_type(solver.add_variable(), lit_type::polarities::positive);
	auto const x5 = lit_type(solver.add_variable(), lit_type::polarities::positive);
	auto const x6 = lit_type(solver.add_variable(), lit_type::polarities::positive);
	auto const x7 = lit_type(solver.add_variable(), lit_type::polarities::positive);
	auto const x8 = lit_type(solver.add_variable(), lit_type::polarities::positive);
	auto const x9 = lit_type(solver.add_variable(), lit_type::polarities::positive);

	/* Example CNF from ''On Computing Minimum Unsatisfiable Cores'',
	 * SAT 2004: we use assumption literals 4,...,9 for clause activation */
	solver.add_clause({~x4, x1, ~x3});
	solver.add_clause({~x5, x2});
	solver.add_clause({~x6, ~x2, x3});
	solver.add_clause({~x7, ~x2, ~x3});
	solver.add_clause({~x8, x2, x3});
	solver.add_clause({~x9, ~x1, x2, ~x3});

	auto r = solver.solve({x4, x5, x6, x7, x8, x9});
	CHECK(r == result::states::unsatisfiable);

	/* start with a huge core */
	result::clause_type core = {x4, x5, x6, x7, x8, x9};
	auto const original_core_size = core.size();

	/* trim the core */
	trim_core(solver, core);
	CHECK(original_core_size >= core.size());
}

TEMPLATE_TEST_CASE("Minimize UNSAT core", "[sat][template]", SOLVER_TYPES)
{
	TestType solver;

	auto const x1 = lit_type(solver.add_variable(), lit_type::polarities::positive);
	auto const x2 = lit_type(solver.add_variable(), lit_type::polarities::positive);
	auto const x3 = lit_type(solver.add_variable(), lit_type::polarities::positive);
	auto const x4 = lit_type(solver.add_variable(), lit_type::polarities::positive);
	auto const x5 = lit_type(solver.add_variable(), lit_type::polarities::positive);
	auto const x6 = lit_type(solver.add_variable(), lit_type::polarities::positive);
	auto const x7 = lit_type(solver.add_variable(), lit_type::polarities::positive);
	auto const x8 = lit_type(solver.add_variable(), lit_type::polarities::positive);
	auto const x9 = lit_type(solver.add_variable(), lit_type::polarities::positive);

	/* Example CNF from ''On Computing Minimum Unsatisfiable Cores'',
	 * SAT 2004: we use assumption literals 4,...,9 for clause activation */
	solver.add_clause({~x4, x1, ~x3});
	solver.add_clause({~x5, x2});
	solver.add_clause({~x6, ~x2, x3});
	solver.add_clause({~x7, ~x2, ~x3});
	solver.add_clause({~x8, x2, x3});
	solver.add_clause({~x9, ~x1, x2, ~x3});

	auto const r = solver.solve({x4, x5, x6, x7, x8, x9});
	CHECK(r == result::states::unsatisfiable);

	result::clause_type core = {x4, x5, x6, x7, x8, x9};
	auto const original_core_size = core.size();

	minimize_core(solver, core);
	CHECK(original_core_size >= core.size());
}
