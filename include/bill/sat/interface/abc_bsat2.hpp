/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*------------------------------------------------------------------------------------------------*/
#pragma once

#include "common.hpp"
#include "types.hpp"

#include <memory>
#include <variant>
#include <vector>
#include <random>

namespace bill {

template<>
class solver<solvers::bsat2> {
	using solver_type = pabc::sat_solver;

public:
#pragma region Constructors
	solver() : var_ctr_( 1, 0u ), cls_ctr_( 1, 0 )
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
		randomize = false;
		var_ctr_.clear();
		var_ctr_.emplace_back( 0u );
		cls_ctr_.clear();
		cls_ctr_.emplace_back( 0 );
	}

	var_type add_variable()
	{
		++var_ctr_.back();
		return pabc::sat_solver_addvar(solver_);
	}

	void add_variables(uint32_t num_variables = 1)
	{
		var_ctr_.back() += num_variables;
		for (auto i = 0u; i < num_variables; ++i) {
			pabc::sat_solver_addvar(solver_);
		}
	}

	auto add_clause(std::vector<lit_type>::const_iterator it,
	                std::vector<lit_type>::const_iterator ie)
	{
		++cls_ctr_.back();
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
		--cls_ctr_.back(); /* do not count unit clauses */
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

		if ( randomize )
		{
			std::vector<uint32_t> vars;
			for ( auto i = 0u; i < num_variables(); ++i )
			{
				if ( random() % 2 )
				{
					vars.push_back( i );
				}
			}
			pabc::sat_solver_set_polarity( solver_, (int*)(const_cast<uint32_t*>(vars.data())), vars.size() );
		}

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
		return var_ctr_.back();
		//return pabc::sat_solver_nvars(solver_);
	}

	uint32_t num_clauses() const
	{
		return cls_ctr_.back();
		//return pabc::sat_solver_nclauses(solver_);
	}
#pragma endregion

	void push()
	{
		pabc::sat_solver_bookmark(solver_);
		var_ctr_.emplace_back( var_ctr_.back() );
		cls_ctr_.emplace_back( cls_ctr_.back() );
	}

	void pop( uint32_t n = 1u )
	{
		assert( n == 1u && "bsat does not support multiple step pop" ); (void)n;
	    pabc::sat_solver_rollback(solver_);
	    var_ctr_.resize( var_ctr_.size() - 1 );
		cls_ctr_.resize( cls_ctr_.size() - 1 );
	}

	void set_random_phase( uint32_t seed = 0u )
	{
		randomize = true;
		pabc::sat_solver_set_random(solver_, 1);
		random.seed( seed );
	}

private:
	/*! \brief Backend solver */
	solver_type* solver_ = nullptr;

	/*! \brief Current state of the solver */
	result::states state_ = result::states::undefined;

	/*! \brief Temporary storage for one clause */
	pabc::lit literals[2048];

	std::default_random_engine random;
	bool randomize = false;
	std::vector<uint32_t> var_ctr_;
	std::vector<int> cls_ctr_;
};

} // namespace bill
