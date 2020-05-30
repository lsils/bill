Introduction
============

Synopsis
--------

The C++ library *bill* serves as an integration layer for reasoning engines.  It supports conceptually different reasoning approaches such as decision diagrams or satisfiability solvers and provides simple unified interfaces for integrating them.

Decision Diagrams
-----------------

Satisfiability solving
----------------------

The following code snippet uses a SAT solver to prove De Morgans's law for propositional logic::

  #include <bill/bill.hpp>
  #include <iostream>
  
  /* instantiate solving engine */
  solver<solvers::ghack> solver;
  
  /* construct problem instance */
  auto const a = lit_type(solver.add_variable(), lit_type::polarities::positive);
  auto const b = lit_type(solver.add_variable(), lit_type::polarities::positive);
  
  auto const t0 = add_tseytin_and(solver, a, b);
  auto const t1 = ~add_tseytin_or(solver, ~a, ~b);
  auto const t2 = add_tseytin_xor(solver, t0, t1);
  solver.add_clause(t2);
  
  /* check for counterexamples using SAT */
  auto const result = solver.solve()
  if (result == result::states::unsatisfiable)
  {
    std::cout << "proved" << std::endl;
  }
  else
  {
    std::cout << "refuted" << std::endl;
  }


