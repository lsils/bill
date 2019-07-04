Change Log
==========

v0.1 (not yet released)
-----------------------

* Solver interfaces:
    - Generic solver interface (`solver`)
    - MapleSAT (`solver<solvers::maple>`)
    - GHack (`solver<solvers::ghack>`)
    - Glucose 4.1 (`solver<solvers::glucose_41>`)
* Types:
    - Wrapper for variables (`var_type`)
    - Wrapper for literals (`lit_type`)
    - Ternary Boolean type (`lbool_type`)
* Encodings:
    - Tseytin encoding (`add_tseytin_and`, `add_tseytin_or`, `add_tseytin_xor`, `add_tseytin_equals`) `#4 <https://github.com/lsils/bill/pull/4>`_
* Cardinality constraints:
    - At least one (`at_least_one`)
    - At most one pairwise (`at_most_one_pairwise`)
