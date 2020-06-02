Change Log
==========

v0.1 (not yet released)
-----------------------

* Solver interfaces:
    - Generic solver interface (`solver`)
    - MapleSAT (`solver<solvers::maple>`)
    - GHack (`solver<solvers::ghack>`)
    - Glucose 4.1 (`solver<solvers::glucose_41>`)
    - ABC bsat2 (`solver<solvers::bsat2>`)
    - Z3 (`solver<solvers::z3>`) `#21 <https://github.com/lsils/bill/pull/21>`_
* Types:
    - Wrapper for variables (`var_type`)
    - Wrapper for literals (`lit_type`)
    - Ternary Boolean type (`lbool_type`)
* Encodings:
    - Tseytin encoding (`add_tseytin_and`, `add_tseytin_or`, `add_tseytin_xor`, `add_tseytin_equals`) `#4 <https://github.com/lsils/bill/pull/4>`_
    - XOR clauses (`add_xor_clause`) `#24 <https://github.com/lsils/bill/pull/24>`_
* Cardinality constraints:
    - At least one (`at_least_one`)
    - At most one pairwise (`at_most_one_pairwise`)
    - Incremental totalizer constraints (`create_totalizer`,`increase_totalizer`,`merge_totalizer`,`extend_totalizer`) `#15 <https://github.com/lsils/bill/pull/15>`_
* Heuristic size-reduction of UNSAT cores:
    - Trim unsatisfiable core (`trim_core`, `trim_core_copy`) `#33 <https://github.com/lsils/bill/pull/33>`_
    - Minimize unsatisifable core (`minimize_core`, `minimize_core_copy`) `#33 <https://github.com/lsils/bill/pull/33>`_
