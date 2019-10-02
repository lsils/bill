/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*------------------------------------------------------------------------------------------------*/
#pragma once

#include <bill/utils/platforms.hpp>

#if defined(BILL_WINDOWS_PLATFORM)
#pragma warning(push)
#pragma warning( \
    disable : 4018 4127 4189 4200 4242 4244 4245 4305 4365 4388 4389 4456 4457 4459 4514 4552 4571 4583 4619 4623 4625 4626 4706 4710 4711 4774 4820 4820 4996 5026 5027 5039)
#include "sat_solvers/ghack.hpp"
#pragma warning(pop)
#else
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdangling-else"
#pragma GCC diagnostic ignored "-Wreorder"
#pragma GCC diagnostic ignored "-Wsign-compare"
#pragma GCC diagnostic ignored "-Wunused-comparison"
#pragma GCC diagnostic ignored "-Wunused-label"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-value"
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wzero-length-array"
#include "sat_solvers/ghack.hpp"
#include "sat_solvers/glucose.hpp"
#include "sat_solvers/maple.hpp"
#if !defined(BILL_WINDOWS_PLATFORM)
#define LIN64
#define ABC_USE_NAMESPACE pabc
#define ABC_NAMESPACE pabc
#define ABC_USE_NO_READLINE
#define SATOKO_NAMESPACE satoko
#include "sat_solvers/abc.hpp"
#include "sat_solvers/satoko.hpp"
#endif
#pragma GCC diagnostic pop
#endif

#include "types.hpp"

#include <memory>
#include <variant>
#include <vector>

namespace bill {

class result {
public:
	using model_type = std::vector<lbool_type>;
	using clause_type = std::vector<lit_type>;

	enum class states : uint8_t {
		satisfiable,
		unsatisfiable,
		undefined,
		timeout,
		dirty,
	};

	static std::string to_string(states const& state)
	{
		switch (state) {
		case states::satisfiable:
			return "satisfiable";
		case states::unsatisfiable:
			return "unsatisfiable";
		case states::timeout:
			return "timeout";
		case states::dirty:
			return "dirty";
		case states::undefined:
		default:
			return "undefined";
		}
	}

#pragma region Constructors
	result(states state = states::undefined)
	    : state_(state)
	{}

	result(model_type const& model)
	    : state_(states::satisfiable)
	    , data_(model)
	{}

	result(clause_type const& unsat_core)
	    : state_(states::unsatisfiable)
	    , data_(unsat_core)
	{}
#pragma endregion

#pragma region Properties
	inline bool is_satisfiable() const
	{
		return (state_ == states::satisfiable);
	}

	inline bool is_unsatisfiable() const
	{
		return (state_ == states::unsatisfiable);
	}

	inline bool is_undefined() const
	{
		return (state_ == states::undefined);
	}

	inline model_type model() const
	{
		return std::get<model_type>(data_);
	}
#pragma endregion

#pragma region Overloads
	inline operator bool() const
	{
		return (state_ == states::satisfiable);
	}

	inline explicit operator std::string() const
	{
		return result::to_string(state_);
	}
#pragma endregion

private:
	states state_;
	std::variant<model_type, clause_type> data_;
};

enum class solvers {
#if !defined(BILL_WINDOWS_PLATFORM)
	glucose_41,
	maple,
	bsat2,
	bmcg,
	satoko,
#endif
	ghack,
};

template<solvers Solver = solvers::ghack>
class solver;

#if !defined(BILL_WINDOWS_PLATFORM)
template<>
class solver<solvers::glucose_41> {
	using solver_type = Glucose::Solver;

public:
#pragma region Constructors
	solver()
	    : solver_(std::make_unique<solver_type>())
	{}

	/* disallow copying */
	solver(solver<solvers::glucose_41> const&) = delete;
	solver<solvers::glucose_41>& operator=(const solver<solvers::glucose_41>&) = delete;
#pragma endregion

#pragma region Modifiers
	void restart()
	{
		solver_.reset();
		solver_ = std::make_unique<solver_type>();
		state_ = result::states::undefined;
	}

	var_type add_variable()
	{
		return solver_->newVar();
	}

	void add_variables(uint32_t num_variables = 1)
	{
		for (auto i = 0u; i < num_variables; ++i) {
			solver_->newVar();
		}
	}

	auto add_clause(std::vector<lit_type>::const_iterator it,
	                std::vector<lit_type>::const_iterator ie)
	{
		Glucose::vec<Glucose::Lit> literals;
		while (it != ie) {
			literals.push(Glucose::mkLit(it->variable(), it->is_complemented()));
			++it;
		}
		auto const result = solver_->addClause_(literals);
		state_ = result ? result::states::dirty : result::states::unsatisfiable;
		return result;
	}

	auto add_clause(std::vector<lit_type> const& clause)
	{
		return add_clause(clause.begin(), clause.end());
	}

	auto add_clause(lit_type lit)
	{
		auto const result = solver_->addClause(
		    Glucose::mkLit(lit.variable(), lit.is_complemented()));
		state_ = result ? result::states::dirty : result::states::unsatisfiable;
		return result;
	}

	result get_model() const
	{
		assert(state_ == result::states::satisfiable);
		result::model_type model;
		for (auto i = 0; i < solver_->model.size(); ++i) {
			if (solver_->model[i] == Glucose::l_False) {
				model.emplace_back(lbool_type::false_);
			} else if (solver_->model[i] == Glucose::l_True) {
				model.emplace_back(lbool_type::true_);
			} else {
				model.emplace_back(lbool_type::undefined);
			}
		}
		return result(model);
	}

	result get_core() const
	{
		assert(state_ == result::states::unsatisfiable);
		result::clause_type unsat_core;
		for (auto i = 0; i < solver_->conflict.size(); ++i) {
			unsat_core.emplace_back(Glucose::var(solver_->conflict[i]),
			                        Glucose::sign(solver_->conflict[i]) ?
			                            negative_polarity :
			                            positive_polarity);
		}
		return result(unsat_core);
	}

	result get_result() const
	{
		assert(state_ != result::states::dirty);
		if (state_ == result::states::satisfiable) {
			return get_model();
		} else if (state_ == result::states::unsatisfiable) {
			return get_core();
		} else {
			return result();
		}
	}

	result::states solve(std::vector<lit_type> const& assumptions = {},
	                     uint32_t conflict_limit = 0)
	{
		if (state_ != result::states::dirty) {
			return state_;
		}

		assert(solver_->okay() == true);
		if (conflict_limit) {
			solver_->setConfBudget(conflict_limit);
		}

		Glucose::vec<Glucose::Lit> literals;
		for (auto lit : assumptions) {
			literals.push(Glucose::mkLit(lit.variable(), lit.is_complemented()));
		}

		Glucose::lbool state = solver_->solveLimited(literals);
		if (state == Glucose::l_True) {
			state_ = result::states::satisfiable;
		} else if (state == Glucose::l_False) {
			state_ = result::states::unsatisfiable;
		} else {
			state_ = result::states::undefined;
		}
		return state_;
	}
#pragma endregion

#pragma region Properties
	uint32_t num_variables() const
	{
		return solver_->nVars();
	}

	uint32_t num_clauses() const
	{
		return solver_->nClauses();
	}
#pragma endregion

private:
	/*! \brief Backend solver */
	std::unique_ptr<solver_type> solver_;

	/*! \brief Current state of the solver */
	result::states state_ = result::states::undefined;
};
#endif

template<>
class solver<solvers::ghack> {
	using solver_type = GHack::Solver;

public:
#pragma region Constructors
	solver()
	    : solver_(std::make_unique<solver_type>())
	{}

	/* disallow copying */
	solver(solver<solvers::ghack> const&) = delete;
	solver<solvers::ghack>& operator=(const solver<solvers::ghack>&) = delete;
#pragma endregion

#pragma region Modifiers
	void restart()
	{
		solver_.reset();
		solver_ = std::make_unique<solver_type>();
		state_ = result::states::undefined;
	}

	var_type add_variable()
	{
		return solver_->newVar();
	}

	void add_variables(uint32_t num_variables = 1)
	{
		for (auto i = 0u; i < num_variables; ++i) {
			solver_->newVar();
		}
	}

	auto add_clause(std::vector<lit_type>::const_iterator it,
	                std::vector<lit_type>::const_iterator ie)
	{
		GHack::vec<GHack::Lit> literals;
		while (it != ie) {
			literals.push(GHack::mkLit(it->variable(), it->is_complemented()));
			++it;
		}
		auto const result = solver_->addClause_(literals);
		state_ = result ? result::states::dirty : result::states::unsatisfiable;
		return result;
	}

	auto add_clause(std::vector<lit_type> const& clause)
	{
		return add_clause(clause.begin(), clause.end());
	}

	auto add_clause(lit_type lit)
	{
		auto const result = solver_->addClause(
		    GHack::mkLit(lit.variable(), lit.is_complemented()));
		state_ = result ? result::states::dirty : result::states::unsatisfiable;
		return result;
	}

	result get_model() const
	{
		assert(state_ == result::states::satisfiable);
		result::model_type model;
		for (auto i = 0; i < solver_->model.size(); ++i) {
			if (solver_->model[i] == GHack::l_False) {
				model.emplace_back(lbool_type::false_);
			} else if (solver_->model[i] == GHack::l_True) {
				model.emplace_back(lbool_type::true_);
			} else {
				model.emplace_back(lbool_type::undefined);
			}
		}
		return result(model);
	}

	result get_core() const
	{
		assert(state_ == result::states::unsatisfiable);
		result::clause_type unsat_core;
		for (auto i = 0; i < solver_->conflict.size(); ++i) {
			unsat_core.emplace_back(GHack::var(solver_->conflict[i]),
			                        GHack::sign(solver_->conflict[i]) ?
			                            negative_polarity :
			                            positive_polarity);
		}
		return result(unsat_core);
	}

	result get_result() const
	{
		assert(state_ != result::states::dirty);
		if (state_ == result::states::satisfiable) {
			return get_model();
		} else if (state_ == result::states::unsatisfiable) {
			return get_core();
		} else {
			return result();
		}
	}

	result::states solve(std::vector<lit_type> const& assumptions = {},
	                     uint32_t conflict_limit = 0)
	{
		if (state_ != result::states::dirty) {
			return state_;
		}

		assert(solver_->okay() == true);
		if (conflict_limit) {
			solver_->setConfBudget(conflict_limit);
		}

		GHack::vec<GHack::Lit> literals;
		for (auto lit : assumptions) {
			literals.push(GHack::mkLit(lit.variable(), lit.is_complemented()));
		}

		GHack::lbool state = solver_->solveLimited(literals);
		if (state == GHack::l_True) {
			state_ = result::states::satisfiable;
		} else if (state == GHack::l_False) {
			state_ = result::states::unsatisfiable;
		} else {
			state_ = result::states::undefined;
		}
		return state_;
	}
#pragma endregion

#pragma region Properties
	uint32_t num_variables() const
	{
		return solver_->nVars();
	}

	uint32_t num_clauses() const
	{
		return solver_->nClauses();
	}
#pragma endregion

private:
	/*! \brief Backend solver */
	std::unique_ptr<solver_type> solver_;

	/*! \brief Current state of the solver */
	result::states state_ = result::states::undefined;
};

#if !defined(BILL_WINDOWS_PLATFORM)
template<>
class solver<solvers::maple> {
	using solver_type = Maple::Solver;

public:
#pragma region Constructors
	solver()
	    : solver_(std::make_unique<solver_type>())
	{}

	/* disallow copying */
	solver(solver<solvers::maple> const&) = delete;
	solver<solvers::maple>& operator=(const solver<solvers::maple>&) = delete;
#pragma endregion

#pragma region Modifiers
	void restart()
	{
		solver_.reset();
		solver_ = std::make_unique<solver_type>();
		state_ = result::states::undefined;
	}

	var_type add_variable()
	{
		return solver_->newVar();
	}

	void add_variables(uint32_t num_variables = 1)
	{
		for (auto i = 0u; i < num_variables; ++i) {
			solver_->newVar();
		}
	}

	auto add_clause(std::vector<lit_type>::const_iterator it,
	                std::vector<lit_type>::const_iterator ie)
	{
		Maple::vec<Maple::Lit> literals;
		while (it != ie) {
			literals.push(Maple::mkLit(it->variable(), it->is_complemented()));
			++it;
		}
		auto const result = solver_->addClause_(literals);
		state_ = result ? result::states::dirty : result::states::unsatisfiable;
		return result;
	}

	auto add_clause(std::vector<lit_type> const& clause)
	{
		return add_clause(clause.begin(), clause.end());
	}

	auto add_clause(lit_type lit)
	{
		auto const result = solver_->addClause(
		    Maple::mkLit(lit.variable(), lit.is_complemented()));
		state_ = result ? result::states::dirty : result::states::unsatisfiable;
		return result;
	}

	result get_model() const
	{
		assert(state_ == result::states::satisfiable);
		result::model_type model;
		for (auto i = 0; i < solver_->model.size(); ++i) {
			if (solver_->model[i] == Maple::l_False) {
				model.emplace_back(lbool_type::false_);
			} else if (solver_->model[i] == Maple::l_True) {
				model.emplace_back(lbool_type::true_);
			} else {
				model.emplace_back(lbool_type::undefined);
			}
		}
		return result(model);
	}

	result get_core() const
	{
		assert(state_ == result::states::unsatisfiable);
		result::clause_type unsat_core;
		for (auto i = 0; i < solver_->conflict.size(); ++i) {
			unsat_core.emplace_back(Maple::var(solver_->conflict[i]),
			                        Maple::sign(solver_->conflict[i]) ?
			                            negative_polarity :
			                            positive_polarity);
		}
		return result(unsat_core);
	}

	result get_result() const
	{
		assert(state_ != result::states::dirty);
		if (state_ == result::states::satisfiable) {
			return get_model();
		} else if (state_ == result::states::unsatisfiable) {
			return get_core();
		} else {
			return result();
		}
	}

	result::states solve(std::vector<lit_type> const& assumptions = {},
	                     uint32_t conflict_limit = 0)
	{
		if (state_ != result::states::dirty) {
			return state_;
		}

		assert(solver_->okay() == true);
		if (conflict_limit) {
			solver_->setConfBudget(conflict_limit);
		}

		Maple::vec<Maple::Lit> literals;
		for (auto lit : assumptions) {
			literals.push(Maple::mkLit(lit.variable(), lit.is_complemented()));
		}

		Maple::lbool state = solver_->solveLimited(literals);
		if (state == Maple::l_True) {
			state_ = result::states::satisfiable;
		} else if (state == Maple::l_False) {
			state_ = result::states::unsatisfiable;
		} else {
			state_ = result::states::undefined;
		}
		return state_;
	}
#pragma endregion

#pragma region Properties
	uint32_t num_variables() const
	{
		return solver_->nVars();
	}

	uint32_t num_clauses() const
	{
		return solver_->nClauses();
	}
#pragma endregion

private:
	/*! \brief Backend solver */
	std::unique_ptr<solver_type> solver_;

	/*! \brief Current state of the solver */
	result::states state_ = result::states::dirty;
};
#endif

#if !defined(BILL_WINDOWS_PLATFORM)
template<>
class solver<solvers::bsat2> {
	using solver_type = pabc::sat_solver;

public:
#pragma region Constructors
	solver()
	{
		solver_ = pabc::sat_solver_new();
	}

	~solver()
	{
		pabc::sat_solver_delete(solver_);
		solver_ = nullptr;
	}

	/* disallow copying */
	solver(solver<solvers::bsat2> const&) = delete;
	solver<solvers::bsat2>& operator=(const solver<solvers::bsat2>&) = delete;
#pragma endregion

#pragma region Modifiers
	void restart()
	{
		pabc::sat_solver_restart(solver_);
		state_ = result::states::undefined;
	}

	var_type add_variable()
	{
		return pabc::sat_solver_addvar(solver_);
	}

	void add_variables(uint32_t num_variables = 1)
	{
		for (auto i = 0u; i < num_variables; ++i) {
			pabc::sat_solver_addvar(solver_);
		}
	}

	auto add_clause(std::vector<lit_type>::const_iterator it,
	                std::vector<lit_type>::const_iterator ie)
	{
		auto counter = 0u;
		while (it != ie) {
			literals[counter++] = pabc::Abc_Var2Lit(it->variable(),
			                                        it->is_complemented());
			++it;
		}
		auto const result = pabc::sat_solver_addclause(solver_, literals, literals + counter);
		state_ = result ? result::states::dirty : result::states::unsatisfiable;
		return result;
	}

	auto add_clause(std::vector<lit_type> const& clause)
	{
		return add_clause(clause.begin(), clause.end());
	}

	auto add_clause(lit_type lit)
	{
		return add_clause(std::vector<lit_type>{lit});
	}

	result get_model() const
	{
		assert(state_ == result::states::satisfiable);
		result::model_type model;
		for (auto i = 0u; i < num_variables(); ++i) {
			auto const value = pabc::sat_solver_var_value(solver_, i);
			if (value == 1) {
				model.emplace_back(lbool_type::true_);
			} else {
				model.emplace_back(lbool_type::false_);
			}
		}
		return result(model);
	}

	result get_result() const
	{
		assert(state_ != result::states::dirty);
		if (state_ == result::states::satisfiable) {
			return get_model();
		} else {
			return result();
		}
	}

	result::states solve(std::vector<lit_type> const& assumptions = {},
	                     uint32_t conflict_limit = 0)
	{
		/* special case: empty solver state */
		if (num_variables() == 0u)
			return result::states::undefined;

		int result;
		if (assumptions.size() > 0u) {
			/* solve with assumptions */
			uint32_t counter = 0u;
			auto it = assumptions.begin();
			while (it != assumptions.end()) {
				literals[counter++] = pabc::Abc_Var2Lit(it->variable(),
				                                        it->is_complemented());
				++it;
			}
			result = pabc::sat_solver_solve(solver_, literals, literals + counter,
			                                conflict_limit, 0, 0, 0);
		} else {
			/* solve without assumptions */
			result = pabc::sat_solver_solve(solver_, 0, 0, conflict_limit, 0, 0, 0);
		}

		if (result == 1) {
			state_ = result::states::satisfiable;
		} else if (result == -1) {
			state_ = result::states::unsatisfiable;
		} else {
			state_ = result::states::undefined;
		}

		return state_;
	}
#pragma endregion

#pragma region Properties
	uint32_t num_variables() const
	{
		return pabc::sat_solver_nvars(solver_);
	}

	uint32_t num_clauses() const
	{
		return pabc::sat_solver_nclauses(solver_);
	}
#pragma endregion

private:
	/*! \brief Backend solver */
	solver_type* solver_ = nullptr;

	/*! \brief Current state of the solver */
	result::states state_ = result::states::undefined;

	/*! \brief Temporary storage for one clause */
	pabc::lit literals[2048];
};
#endif

#if !defined(BILL_WINDOWS_PLATFORM)
template<>
class solver<solvers::bmcg> {
	using solver_type = pabc::bmcg_sat_solver;

public:
#pragma region Constructors
	solver()
	{
		solver_ = pabc::bmcg_sat_solver_start();
	}

	~solver()
	{
		pabc::bmcg_sat_solver_stop(solver_);
		solver_ = nullptr;
	}

	/* disallow copying */
	solver(solver<solvers::bmcg> const&) = delete;
	solver<solvers::bmcg>& operator=(const solver<solvers::bmcg>&) = delete;
#pragma endregion

#pragma region Modifiers
	void restart()
	{
		pabc::bmcg_sat_solver_reset(solver_);
		state_ = result::states::undefined;
		variable_counter_ = 0u;
	}

	var_type add_variable()
	{
		variable_counter_++;
		return pabc::bmcg_sat_solver_addvar(solver_);
	}

	void add_variables(uint32_t num_variables = 1)
	{
		for (auto i = 0u; i < num_variables; ++i) {
			pabc::bmcg_sat_solver_addvar(solver_);
		}
		variable_counter_ += num_variables;
	}

	auto add_clause(std::vector<lit_type>::const_iterator it,
	                std::vector<lit_type>::const_iterator ie)
	{
		auto counter = 0u;
		while (it != ie) {
			literals[counter++] = pabc::Abc_Var2Lit(it->variable(),
			                                        it->is_complemented());
			++it;
		}
		auto const result = pabc::bmcg_sat_solver_addclause(solver_, literals, counter);
		state_ = result ? result::states::dirty : result::states::unsatisfiable;
		return result;
	}

	auto add_clause(std::vector<lit_type> const& clause)
	{
		return add_clause(clause.begin(), clause.end());
	}

	auto add_clause(lit_type lit)
	{
		return add_clause(std::vector<lit_type>{lit});
	}

	result get_model() const
	{
		assert(state_ == result::states::satisfiable);
		result::model_type model;
		for (auto i = 0u; i < num_variables(); ++i) {
			auto const value = pabc::bmcg_sat_solver_read_cex_varvalue(solver_, i);
			if (value == 1) {
				model.emplace_back(lbool_type::true_);
			} else {
				model.emplace_back(lbool_type::false_);
			}
		}
		return result(model);
	}

	result get_result() const
	{
		assert(state_ != result::states::dirty);
		if (state_ == result::states::satisfiable) {
			return get_model();
		} else {
			return result();
		}
	}

	result::states solve(std::vector<lit_type> const& assumptions = {},
	                     uint32_t conflict_limit = 0)
	{
		/* special case: empty solver state */
		if (num_variables() == 0u)
			return result::states::undefined;

		if (conflict_limit > 0)
			pabc::bmcg_sat_solver_set_conflict_budget(solver_, conflict_limit);

		int result;
		if (assumptions.size() > 0u) {
			/* solve with assumptions */
			uint32_t counter = 0u;
			auto it = assumptions.begin();
			while (it != assumptions.end()) {
				literals[counter++] = pabc::Abc_Var2Lit(it->variable(),
				                                        it->is_complemented());
				++it;
			}
			result = pabc::bmcg_sat_solver_solve(solver_, literals, counter);
		} else {
			/* solve without assumptions */
			result = pabc::bmcg_sat_solver_solve(solver_, 0, 0);
		}

		if (result == 1) {
			state_ = result::states::satisfiable;
		} else if (result == -1) {
			state_ = result::states::unsatisfiable;
		} else {
			state_ = result::states::undefined;
		}

		return state_;
	}
#pragma endregion

#pragma region Properties
	uint32_t num_variables() const
	{
		return variable_counter_;
	}

	uint32_t num_clauses() const
	{
		return pabc::bmcg_sat_solver_clausenum(solver_);
	}
#pragma endregion

private:
	/*! \brief Backend solver */
	solver_type* solver_ = nullptr;

	/*! \brief Current state of the solver */
	result::states state_ = result::states::undefined;

	/*! \brief Temporary storage for one clause */
	pabc::lit literals[2048];

	/*! \brief Count the number of variables */
	uint32_t variable_counter_ = 0u;
};
#endif

#if !defined(BILL_WINDOWS_PLATFORM)
template<>
class solver<solvers::satoko> {
public:
	using solver_type = satoko::satoko_t;

#pragma region Constructors
	solver()
	{
		solver_ = satoko::satoko_create();
	}

	~solver()
	{
		satoko::satoko_destroy(solver_);
		solver_ = nullptr;
	}

	/* disallow copying */
	solver(solver<solvers::satoko> const&) = delete;
	solver<solvers::satoko>& operator=(const solver<solvers::satoko>&) = delete;
#pragma endregion

#pragma region Modifiers
	void restart()
	{
		satoko::satoko_reset(solver_);
		state_ = result::states::undefined;
	}

	var_type add_variable()
	{
		return satoko::satoko_add_variable(solver_, 0);
	}

	void add_variables(uint32_t num_variables = 1)
	{
		for (auto i = 0u; i < num_variables; ++i) {
			satoko::satoko_add_variable(solver_, 0);
		}
	}

	auto add_clause(std::vector<lit_type>::const_iterator it,
	                std::vector<lit_type>::const_iterator ie)
	{
		auto counter = 0u;
		while (it != ie) {
			literals[counter++] = pabc::Abc_Var2Lit(it->variable(),
			                                        it->is_complemented());
			++it;
		}
		auto const result = satoko::satoko_add_clause(solver_, literals, counter);
		state_ = result ? result::states::dirty : result::states::unsatisfiable;
		return result;
	}

	auto add_clause(std::vector<lit_type> const& clause)
	{
		return add_clause(clause.begin(), clause.end());
	}

	auto add_clause(lit_type lit)
	{
		return add_clause(std::vector<lit_type>{lit});
	}

	result get_model() const
	{
		assert(state_ == result::states::satisfiable);
		result::model_type model;
		for (auto i = 0u; i < num_variables(); ++i) {
			auto const value = satoko::satoko_read_cex_varvalue(solver_, i);
			if (value == 1) {
				model.emplace_back(lbool_type::true_);
			} else {
				model.emplace_back(lbool_type::false_);
			}
		}
		return result(model);
	}

	result get_result() const
	{
		assert(state_ != result::states::dirty);
		if (state_ == result::states::satisfiable) {
			return get_model();
		} else {
			return result();
		}
	}

	result::states solve(std::vector<lit_type> const& assumptions = {},
	                     uint32_t conflict_limit = 0)
	{
		/* special case: empty solver state */
		if (num_variables() == 0u)
			return result::states::undefined;

		int result;
		if (assumptions.size() > 0u) {
			/* solve with assumptions */
			uint32_t counter = 0u;
			auto it = assumptions.begin();
			while (it != assumptions.end()) {
				literals[counter++] = pabc::Abc_Var2Lit(it->variable(),
				                                        it->is_complemented());
				++it;
			}
			result = satoko::satoko_solve_assumptions_limit(solver_, literals, counter,
			                                                conflict_limit);
		} else {
			/* solve without assumptions */
			result = satoko::satoko_solve_assumptions_limit(solver_, 0, 0,
			                                                conflict_limit);
		}

		if (result == satoko::SATOKO_SAT) {
			state_ = result::states::satisfiable;
		} else if (result == satoko::SATOKO_UNSAT) {
			state_ = result::states::unsatisfiable;
		} else {
			state_ = result::states::undefined;
		}

		return state_;
	}
#pragma endregion

#pragma region Properties
	uint32_t num_variables() const
	{
		return satoko::satoko_varnum(solver_);
	}

	uint32_t num_clauses() const
	{
		return satoko::satoko_clausenum(solver_);
	}
#pragma endregion

private:
	/*! \brief Backend solver */
	solver_type* solver_ = nullptr;

	/*! \brief Current state of the solver */
	result::states state_ = result::states::undefined;

	/*! \brief Temporary storage for one clause */
	pabc::lit literals[2048];

	/*! \brief Count the number of variables */
	uint32_t variable_counter_ = 0u;
};
#endif

} // namespace bill
