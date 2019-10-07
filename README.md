[![Build Status](https://travis-ci.org/lsils/bill.svg?branch=master)](https://travis-ci.org/lsils/bill)
[![Build status](https://ci.appveyor.com/api/projects/status/pu4w14anom4lesoj?svg=true)](https://ci.appveyor.com/project/hriener/bill)
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

## EPFL logic sythesis libraries

bill is part of the [EPFL logic synthesis](https://lsi.epfl.ch/page-138455-en.html) libraries.  The other libraries and several examples on how to use and integrate the libraries can be found in the [logic synthesis tool showcase](https://github.com/lsils/lstools-showcase).
