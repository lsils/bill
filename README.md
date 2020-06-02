[![Actions Status](https://github.com/lsils/bill/workflows/Linux%20CI/badge.svg)](https://github.com/lsils/bill/actions)
[![Actions Status](https://github.com/lsils/bill/workflows/MacOS%20CI/badge.svg)](https://github.com/lsils/bill/actions)
[![Actions Status](https://github.com/lsils/bill/workflows/Windows%20CI/badge.svg)](https://github.com/lsils/bill/actions)
[![Coverage Status](https://coveralls.io/repos/github/lsils/bill/badge.svg?branch=master)](https://coveralls.io/github/lsils/bill?branch=master)
[![Documentation Status](https://readthedocs.org/projects/bill/badge/?version=latest)](https://bill.readthedocs.io/en/latest/?badge=latest)
[![License](https://img.shields.io/badge/license-MIT-000000.svg)](https://opensource.org/licenses/MIT)

<img src="https://cdn.jsdelivr.net/gh/lsils/bill@master/bill.svg" width="64" height="64" align="left" style="margin-right: 12pt" />
bill is a header-only reasoning library.  The library serves as an integration layer for various reasoning engines (such as decision diagrams or satisfiability solvers) and provides unified interfaces for them.

[Read the full documentation.](http://bill.readthedocs.io/en/latest/?badge=latest)

## Example

The following code snippet uses a SAT solver to prove De Morgan's law for propositional logic.

```c++
#include <bill/sat/solver.hpp>
#include <bill/sat/tseytin.hpp>

solver<solvers::ghack> solver;
auto const a = lit_type(solver.add_variable(), lit_type::polarities::positive);
auto const b = lit_type(solver.add_variable(), lit_type::polarities::positive);

auto const t0 = add_tseytin_and(solver, a, b);
auto const t1 = ~add_tseytin_or(solver, ~a, ~b);
auto const t2 = add_tseytin_xor(solver, t0, t1);
solver.add_clause(t2);

CHECK(solver.solve() == result::states::unsatisfiable);
```

## Installation requirements

A modern compiler is required to build *bill*.  We are continously testing with Clang 6.0.1, GCC 7.3.0, and GCC 8.2.0.  More information can be found in the [documentation](http://bill.readthedocs.io/en/latest/installation.html).

## EPFL logic sythesis libraries

bill is part of the [EPFL logic synthesis](https://lsi.epfl.ch/page-138455-en.html) libraries.  The other libraries and several examples on how to use and integrate the libraries can be found in the [logic synthesis tool showcase](https://github.com/lsils/lstools-showcase).
