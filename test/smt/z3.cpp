/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*------------------------------------------------------------------------------------------------*/
#if defined(BILL_HAS_Z3)

#include "../catch2.hpp"

#include <bill/smt/z3.hpp>
#include <iostream>
#include <vector>

using namespace bill;

TEST_CASE("Simple SAT LP", "[smt/z3]")
{
    using solver_t = z3_smt_solver<false>;

    solver_t solver;
    auto const v1 = solver.add_variable( solver_t::var_types::real );
    solver.add_lp_constraint( {{1,v1}}, 3, solver_t::lp_types::geq ); // v1 >= 3
    solver.add_lp_constraint( {{2,v1}}, 8, solver_t::lp_types::leq ); // 2v1 <= 8 --> v1 <= 4

    auto const res = solver.solve();
    CHECK( res == solver_t::states::satisfiable );
    auto const sol_v1 = solver.get_numeral_variable_value_as_integer( v1 );
    CHECK( sol_v1 >= 3 );
    CHECK( sol_v1 <= 4 );
}

TEST_CASE("Simple SAT ILP", "[smt/z3]")
{
    using solver_t = z3_smt_solver<false>;

    solver_t solver;
    auto const v1 = solver.add_variable( solver_t::var_types::integer );
    solver.add_ilp_constraint( {{1,v1}}, 3, solver_t::lp_types::geq ); // v1 >= 3
    solver.add_ilp_constraint( {{2,v1}}, 8, solver_t::lp_types::leq ); // 2v1 <= 8 --> v1 <= 4

    auto const res = solver.solve();
    CHECK( res == solver_t::states::satisfiable );
    auto const sol_v1 = solver.get_numeral_variable_value_as_integer( v1 );
    CHECK( sol_v1 >= 3 );
    CHECK( sol_v1 <= 4 );
}

TEST_CASE("Simple UNSAT LP", "[smt/z3]")
{
    using solver_t = z3_smt_solver<false>;

    solver_t solver;
    auto const v1 = solver.add_variable( solver_t::var_types::real );
    solver.add_lp_constraint( {{1,v1}}, 3, solver_t::lp_types::geq ); // v1 >= 3
    solver.add_lp_constraint( {{2,v1}}, 2, solver_t::lp_types::leq ); // 2v1 <= 2 --> v1 <= 1

    auto const res = solver.solve();
    CHECK( res == solver_t::states::unsatisfiable );
}

TEST_CASE("Simple UNSAT ILP", "[smt/z3]")
{
    using solver_t = z3_smt_solver<false>;

    solver_t solver;
    auto const v1 = solver.add_variable( solver_t::var_types::integer );
    solver.add_ilp_constraint( {{1,v1}}, 3, solver_t::lp_types::geq ); // v1 >= 3
    solver.add_ilp_constraint( {{1,v1}}, 3, solver_t::lp_types::less ); // v1 < 3

    auto const res = solver.solve();
    CHECK( res == solver_t::states::unsatisfiable );
}

TEST_CASE("Simple optimization LP", "[smt/z3]")
{
    using solver_t = z3_smt_solver<true>;

    solver_t solver;
    auto const v1 = solver.add_variable( solver_t::var_types::real );
    solver.add_lp_constraint( {{2,v1}}, 8, solver_t::lp_types::leq ); // 2v1 <= 8 --> v1 <= 4
    solver.maximize( v1 );

    auto const res = solver.solve();
    CHECK( res == solver_t::states::satisfiable );
    auto const sol_v1 = solver.get_numeral_variable_value_as_integer( v1 );
    CHECK( sol_v1 == 4 );
}

TEST_CASE("Simple optimization ILP", "[smt/z3]")
{
    using solver_t = z3_smt_solver<true>;

    solver_t solver;
    auto const v1 = solver.add_variable( solver_t::var_types::integer );
    solver.add_ilp_constraint( {{2,v1}}, 8, solver_t::lp_types::less ); // 2v1 < 8 --> v1 < 4
    solver.maximize( v1 );

    auto const res = solver.solve();
    CHECK( res == solver_t::states::satisfiable );
    auto const sol_v1 = solver.get_numeral_variable_value_as_integer( v1 );
    CHECK( sol_v1 == 3 );
}

TEST_CASE("LP with cardinality constraints", "[smt/z3]")
{
    using solver_t = z3_smt_solver<false>;

    solver_t solver;
    auto const v1 = solver.add_variable( solver_t::var_types::real );
    auto const b1 = solver.add_lp_condition( {{1,v1}}, 3, solver_t::lp_types::geq ); // v1 >= 3
    auto const b2 = solver.add_lp_condition( {{2,v1}}, 8, solver_t::lp_types::leq ); // 2v1 <= 8 --> v1 <= 4
    auto const b3 = solver.add_lp_condition( {{1,v1}}, 5, solver_t::lp_types::greater ); // v1 > 5
    auto const sum = solver.add_real_cardinality( {b1, b2, b3} );
    solver.add_lp_constraint( {{1,sum}}, 3, solver_t::lp_types::eq );

    auto const res = solver.solve();
    CHECK( res == solver_t::states::unsatisfiable );
}

TEST_CASE("ILP with cardinality constraints and objective", "[smt/z3]")
{
    using solver_t = z3_smt_solver<true>;

    solver_t solver;
    auto const v1 = solver.add_variable( solver_t::var_types::integer );
    auto const b1 = solver.add_ilp_condition( {{1,v1}}, -2, solver_t::lp_types::geq ); // v1 >= -2
    auto const b2 = solver.add_ilp_condition( {{1,v1}}, -1, solver_t::lp_types::leq ); // v1 <= -1
    auto const b3 = solver.add_ilp_condition( {{1,v1}}, 0, solver_t::lp_types::eq ); // v1 == 0
    auto const b4 = solver.add_ilp_condition( {{1,v1}}, 10, solver_t::lp_types::less ); // v1 < 10
    auto const b5 = solver.add_ilp_condition( {{1,v1}}, 0, solver_t::lp_types::geq ); // v1 >= 0
    auto const sum = solver.add_integer_cardinality( {b1, b2, b3, b4, b5} );
    solver.maximize( sum );

    auto const res = solver.solve();
    CHECK( res == solver_t::states::satisfiable );
    auto const sol_v1 = solver.get_numeral_variable_value_as_integer( v1 );
    CHECK( sol_v1 == 0 );
    auto const sol_sum = solver.get_numeral_variable_value_as_integer( sum );
    CHECK( sol_sum == 4 );
}

#endif
