/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*------------------------------------------------------------------------------------------------*/

#include "../catch2.hpp"

#include <bill/sat/incremental_totalizer_cardinality.hpp>
#include <bill/sat/solver.hpp>
#include <bill/sat/tseytin.hpp>
#include <iostream>
#include <vector>

using namespace bill;

#if defined(BILL_WINDOWS_PLATFORM)
#define SOLVER_TYPES solver<solvers::glucose_41>, solver<solvers::ghack>
#else
#define SOLVER_TYPES                                                                 \
	solver<solvers::glucose_41>, solver<solvers::ghack>, solver<solvers::maple>, \
	    solver<solvers::bsat2>, solver<solvers::bmcg>
#endif

TEMPLATE_TEST_CASE("Enumerate cardiality-five solutions", "[cardinality][template]", SOLVER_TYPES)
{
	TestType solver;

	auto const n = 5;
	auto const k_max = 5;

	std::vector<lit_type> lits;
	for (auto i = 0; i < n; ++i) {
		auto const v = solver.add_variable();
		lits.emplace_back(v, lit_type::polarities::positive);
	}

	std::vector<std::vector<lit_type>> cls;
	auto tree = create_totalizer(solver, cls, lits, k_max);
	for (auto const& c : cls) {
		solver.add_clause(c);
	}

	std::vector<int> blocked;
	for (auto k = 0; k <= k_max; ++k) {
		// std::cout << "[i] k = " << k << std::endl;
		std::vector<lit_type> assumptions;
		for (auto i = 0; i < k_max; ++i) {
			if (i < k) {
				assumptions.push_back(tree->vars[i]);
			} else {
				assumptions.push_back(~tree->vars[i]);
			}
		}

		while (solver.solve(assumptions) == result::states::satisfiable) {
			auto const result = solver.get_model();
			bill::result::model_type m = result.model();

			/* count number of one-bits */
			auto counter = 0;
			std::vector<lit_type> cl;
			for (const auto& l : lits) {
				if (m[l.variable()] == lbool_type::true_)
					++counter;

				cl.push_back(m[l.variable()] == lbool_type::true_ ? ~l : l);
			}
			solver.add_clause(cl);

			/* number of one-bits must be equal to k */
			CHECK(counter == k);
		}
	}
}

TEMPLATE_TEST_CASE("Increase cardinality n=7, k=3 to cardinality k=6", "[cardinality][template]",
                   SOLVER_TYPES)
{
	TestType solver;

	/* 7 variables, cardinality constraint with at most 3 */
	auto const n = 7;
	auto k_max = 3;
	auto k = 2; /* k must be smaller than k_max */

	std::vector<lit_type> lits;
	for (auto i = 0; i < n; ++i) {
		auto const v = solver.add_variable();
		lits.emplace_back(v, lit_type::polarities::positive);
	}

	/* create cardinality constraint */
	std::vector<std::vector<lit_type>> cls;
	auto tree = create_totalizer(solver, cls, lits, k_max);
	for (const auto& c : cls) {
		solver.add_clause(c);
	}

	/* prepare assumptions */
	std::vector<lit_type> assumptions;
	for (auto i = 0; i < k_max; ++i) {
		assumptions.push_back((i < k) ? tree->vars[i] : ~tree->vars[i]);
	}

	/* enumerate all solutions and block them */
	auto num_k2_solutions = 0;
	while (solver.solve(assumptions) == result::states::satisfiable) {
		++num_k2_solutions;

		auto const result = solver.get_model();
		bill::result::model_type m = result.model();

		std::vector<lit_type> clause;

		auto count = 0;
		for (const auto& l : lits) {
			clause.push_back(m[l.variable()] == lbool_type::true_ ? ~l : l);

			/* count number of ones */
			if (m[l.variable()] == lbool_type::true_) {
				++count;
			}
		}

		/* block */
		solver.add_clause(clause);

		/* in each solution at most k bits are set */
		CHECK(count <= k);
	}

	/* increase the cardinality constraint to k_max = 6 */
	k_max = 6;
	std::vector<std::vector<lit_type>> cls2;
	increase_totalizer(solver, cls2, tree, k_max);
	for (const auto& c : cls2) {
		solver.add_clause(c);
	}

	/* now we increase k = 5 and prepare new assumptions */
	k = 5;
	assumptions.clear();
	for (auto i = 0; i < k_max; ++i) {
		assumptions.push_back((i < k) ? tree->vars[i] : ~tree->vars[i]);
	}

	/* ... and enumerate again */
	auto num_k5_solutions = 0;
	while (solver.solve(assumptions) == result::states::satisfiable) {
		++num_k5_solutions;

		auto const result = solver.get_model();
		bill::result::model_type m = result.model();

		std::vector<lit_type> clause;
		auto count = 0;
		for (const auto& l : lits) {
			clause.push_back(m[l.variable()] == lbool_type::true_ ? ~l : l);

			/* count number of ones */
			if (m[l.variable()] == lbool_type::true_) {
				++count;
			}
		}

		/* block */
		solver.add_clause(clause);

		/* in each solution at most k bits are set */
		CHECK(count <= k);
	}

	CHECK(num_k2_solutions == 29);
	CHECK(num_k5_solutions == 91);
}
