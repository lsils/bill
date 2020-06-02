#pragma once
#include <bill/sat/solver.hpp>

#if defined(BILL_WINDOWS_PLATFORM) && defined(BILL_HAS_Z3)
#define SOLVER_TYPES bill::solver<bill::solvers::glucose_41>, bill::solver<bill::solvers::ghack>, bill::solver<bill::solvers::bsat2>, bill::solver<bill::solvers::z3>
#elif defined(BILL_WINDOWS_PLATFORM) && !defined(BILL_HAS_Z3)
#define SOLVER_TYPES bill::solver<bill::solvers::glucose_41>, bill::solver<bill::solvers::ghack>, bill::solver<bill::solvers::bsat2>
#elif !defined(BILL_WINDOWS_PLATFORM) && defined(BILL_HAS_Z3)
#define SOLVER_TYPES                                                                                               \
	bill::solver<bill::solvers::glucose_41>, solver<bill::solvers::ghack>, bill::solver<bill::solvers::maple>, \
	    bill::solver<bill::solvers::bsat2>, solver<bill::solvers::bmcg>, bill::solver<bill::solvers::z3>
#else
#define SOLVER_TYPES                                                                                                     \
	bill::solver<bill::solvers::glucose_41>, bill::solver<bill::solvers::ghack>, bill::solver<bill::solvers::maple>, \
	    bill::solver<bill::solvers::bsat2>, bill::solver<bill::solvers::bmcg>
#endif
