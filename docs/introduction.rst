Introduction
============

Synopsis
--------

The C++ library *bill* serves as an integration layer for reasoning engines.  It supports conceptually different reasoning approaches such as decision diagrams or satisfiability solvers and provides simple unified interfaces for integrating them.

Decision diagrams
-----------------

The following code snippet uses ZDDs to hold a database of words and do to a simple query::

  #include <bill/dd/zdd.hpp>
  #include <iostream>

  // Words on our database
  const char *words[] = {
    "which", "there", "their", "about", "would", "these", "other",
    "words", "could", "write", "first", "water", "after", "where",
    "right", "think", "three", "years", "place", nullptr
  };
  // Each word has five letters, each of the 26 letter might be in one of
  // the five positions.  Hence we need to create a ZDD with 26x5
  // variables:
  bill::zdd_base base(5 * 26);
  auto dict = base.bottom(); // The dictionary starts as an empty family

  // Add the words to the dictionary
  for (uint32_t i = 0; words[i] != nullptr; ++i) {
    auto word = base.top(); // word start with as an empty set 
      for (uint32_t j = 0; j < 5; ++j) {
      uint32_t var = (words[i][j] -  'a') + (26 * j);
      auto zdd_var = base.elementary(var);
      word = base.join(word, zdd_var);
    }
    dict = base.union_(dict, word);
  }
  base.print_sets(dict, std::cout); // Print all the words as sets

  // Configure a query
  char letter = 'h';
  uint8_t position = 1;
  uint32_t var = letter - 'a' + (26 * position);

  // Retrieve all that satisfies the configuration
  auto query = base.nonsupersets(dict, base.elementary(var));
  query = base.difference(dict, query);
  // Print words
  base.foreach_set(query, [](auto const& set) {
    for (uint32_t i = 0; i < 5; ++i) {
      uint32_t letter = (set[i] - (26 * i)) + 'a';
      std::cout << char(letter);
    }
    std::cout << '\n';
    return true;
  });

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

