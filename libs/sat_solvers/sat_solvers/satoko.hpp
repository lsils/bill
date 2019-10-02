#pragma once

/*** solver_api.cpp ***/
//===--- solver_api.h -------------------------------------------------------===
//
//                     satoko: Satisfiability solver
//
// This file is distributed under the BSD 2-Clause License.
// See LICENSE for details.
//
//===------------------------------------------------------------------------===
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <math.h>

#include <satoko/act_var.h>
#include <satoko/solver.h>
#include <satoko/misc.h>

#include <abc/abc_global.h>
SATOKO_NAMESPACE_IMPL_START

//===------------------------------------------------------------------------===
// Satoko internal functions
//===------------------------------------------------------------------------===
static inline void solver_rebuild_order(solver_t *s)
{
    unsigned var;
    vec_uint_t *vars = vec_uint_alloc(vec_char_size(s->assigns));

    for (var = 0; var < vec_char_size(s->assigns); var++)
        if (var_value(s, var) == SATOKO_VAR_UNASSING)
            vec_uint_push_back(vars, var);
    heap_build(s->var_order, vars);
    vec_uint_free(vars);
}

static inline int clause_is_satisfied(solver_t *s, struct clause *clause)
{
    unsigned i;
    unsigned *lits = &(clause->data[0].lit);
    for (i = 0; i < clause->size; i++)
        if (lit_value(s, lits[i]) == SATOKO_LIT_TRUE)
            return SATOKO_OK;
    return SATOKO_ERR;
}

static inline void solver_clean_stats(solver_t *s)
{
    long n_conflicts_all = s->stats.n_conflicts_all;
    long n_propagations_all = s->stats.n_propagations_all;
    memset(&(s->stats), 0, sizeof(struct satoko_stats));
    s->stats.n_conflicts_all = n_conflicts_all;
    s->stats.n_propagations_all = n_propagations_all;
}

static inline void print_opts(solver_t *s)
{
    printf( "+-[ BLACK MAGIC - PARAMETERS ]-+\n");
    printf( "|                              |\n");
    printf( "|--> Restarts heuristic        |\n");
    printf( "|    * LBD Queue   = %6d    |\n", s->opts.sz_lbd_bqueue);
    printf( "|    * Trail Queue = %6d    |\n", s->opts.sz_trail_bqueue);
    printf( "|    * f_rst       = %6.2f    |\n", s->opts.f_rst);
    printf( "|    * b_rst       = %6.2f    |\n", s->opts.b_rst);
    printf( "|                              |\n");
    printf( "|--> Clause DB reduction:      |\n");
    printf( "|    * First       = %6d    |\n", s->opts.n_conf_fst_reduce);
    printf( "|    * Inc         = %6d    |\n", s->opts.inc_reduce);
    printf( "|    * Special Inc = %6d    |\n", s->opts.inc_special_reduce);
    printf( "|    * Protected (LBD) < %2d    |\n", s->opts.lbd_freeze_clause);
    printf( "|                              |\n");
    printf( "|--> Binary resolution:        |\n");
    printf( "|    * Clause size < %3d       |\n", s->opts.clause_max_sz_bin_resol);
    printf( "|    * Clause lbd  < %3d       |\n", s->opts.clause_min_lbd_bin_resol);
    printf( "+------------------------------+\n\n");
}

static inline void print_stats(solver_t *s)
{
    printf("starts        : %10d\n", s->stats.n_starts);
    printf("conflicts     : %10ld\n", s->stats.n_conflicts);
    printf("decisions     : %10ld\n", s->stats.n_decisions);
    printf("propagations  : %10ld\n", s->stats.n_propagations);
}

//===------------------------------------------------------------------------===
// Satoko external functions
//===------------------------------------------------------------------------===
solver_t * satoko_create()
{
    solver_t *s = satoko_calloc(solver_t, 1);

    satoko_default_opts(&s->opts);
    s->status = SATOKO_OK;
    /* User data */
    s->assumptions = vec_uint_alloc(0);
    s->final_conflict = vec_uint_alloc(0);
    /* Clauses Database */
    s->all_clauses = cdb_alloc(0);
    s->originals = vec_uint_alloc(0);
    s->learnts = vec_uint_alloc(0);
    s->watches = vec_wl_alloc(0);
    /* Activity heuristic */
    s->var_act_inc = VAR_ACT_INIT_INC;
    s->clause_act_inc = CLAUSE_ACT_INIT_INC;
    /* Variable Information */
    s->activity = vec_act_alloc(0);
    s->var_order = heap_alloc(s->activity);
    s->levels = vec_uint_alloc(0);
    s->reasons = vec_uint_alloc(0);
    s->assigns = vec_char_alloc(0);
    s->polarity = vec_char_alloc(0);
    /* Assignments */
    s->trail = vec_uint_alloc(0);
    s->trail_lim = vec_uint_alloc(0);
    /* Temporary data used by Search method */
    s->bq_trail = b_queue_alloc(s->opts.sz_trail_bqueue);
    s->bq_lbd = b_queue_alloc(s->opts.sz_lbd_bqueue);
    s->n_confl_bfr_reduce = s->opts.n_conf_fst_reduce;
    s->RC1 = 1;
    s->RC2 = s->opts.n_conf_fst_reduce;
    /* Temporary data used by Analyze */
    s->temp_lits = vec_uint_alloc(0);
    s->seen = vec_char_alloc(0);
    s->tagged = vec_uint_alloc(0);
    s->stack = vec_uint_alloc(0);
    s->last_dlevel = vec_uint_alloc(0);
    /* Misc temporary */
    s->stamps = vec_uint_alloc(0);
    return s;
}

void satoko_destroy(solver_t *s)
{
    vec_uint_free(s->assumptions);
    vec_uint_free(s->final_conflict);
    cdb_free(s->all_clauses);
    vec_uint_free(s->originals);
    vec_uint_free(s->learnts);
    vec_wl_free(s->watches);
    vec_act_free(s->activity);
    heap_free(s->var_order);
    vec_uint_free(s->levels);
    vec_uint_free(s->reasons);
    vec_char_free(s->assigns);
    vec_char_free(s->polarity);
    vec_uint_free(s->trail);
    vec_uint_free(s->trail_lim);
    b_queue_free(s->bq_lbd);
    b_queue_free(s->bq_trail);
    vec_uint_free(s->temp_lits);
    vec_char_free(s->seen);
    vec_uint_free(s->tagged);
    vec_uint_free(s->stack);
    vec_uint_free(s->last_dlevel);
    vec_uint_free(s->stamps);
    if (s->marks)
        vec_char_free(s->marks);
    satoko_free(s);
}

void satoko_default_opts(satoko_opts_t *opts)
{
    memset(opts, 0, sizeof(satoko_opts_t));
    opts->verbose = 0;
    opts->no_simplify = 0;
    /* Limits */
    opts->conf_limit = 0;
    opts->prop_limit  = 0;
    /* Constants used for restart heuristic */
    opts->f_rst = 0.8;
    opts->b_rst = 1.4;
    opts->fst_block_rst   = 10000;
    opts->sz_lbd_bqueue   = 50;
    opts->sz_trail_bqueue = 5000;
    /* Constants used for clause database reduction heuristic */
    opts->n_conf_fst_reduce = 2000;
    opts->inc_reduce = 300;
    opts->inc_special_reduce = 1000;
    opts->lbd_freeze_clause = 30;
    opts->learnt_ratio = 0.5;
    /* VSIDS heuristic */
    opts->var_act_limit = VAR_ACT_LIMIT;
    opts->var_act_rescale = VAR_ACT_RESCALE;
    opts->var_decay = 0.95;
    opts->clause_decay = (clause_act_t) 0.995;
    /* Binary resolution */
    opts->clause_max_sz_bin_resol = 30;
    opts->clause_min_lbd_bin_resol = 6;

    opts->garbage_max_ratio = (float) 0.3;
}

/**
 * TODO: sanity check on configuration options
 */
void satoko_configure(satoko_t *s, satoko_opts_t *user_opts)
{
    assert(user_opts);
    memcpy(&s->opts, user_opts, sizeof(satoko_opts_t));
}

int satoko_simplify(solver_t * s)
{
    unsigned i, j = 0;
    unsigned cref;

    assert(solver_dlevel(s) == 0);
    if (solver_propagate(s) != UNDEF)
        return SATOKO_ERR;
    if (s->n_assigns_simplify == vec_uint_size(s->trail) || s->n_props_simplify > 0)
        return SATOKO_OK;

    vec_uint_foreach(s->originals, cref, i) {
        struct clause *clause = clause_fetch(s, cref);

    if (clause_is_satisfied(s, clause)) {
            clause->f_mark = 1;
            s->stats.n_original_lits -= clause->size;
            clause_unwatch(s, cref);
        } else
            vec_uint_assign(s->originals, j++, cref);
    }
    vec_uint_shrink(s->originals, j);
    solver_rebuild_order(s);
    s->n_assigns_simplify = vec_uint_size(s->trail);
    s->n_props_simplify = s->stats.n_original_lits + s->stats.n_learnt_lits;
    return SATOKO_OK;
}

void satoko_setnvars(solver_t *s, int nvars)
{
    int i;
    for (i = satoko_varnum(s); i < nvars; i++)
        satoko_add_variable(s, 0);
}

int satoko_add_variable(solver_t *s, char sign)
{
    unsigned var = vec_act_size(s->activity);
    vec_wl_push(s->watches);
    vec_wl_push(s->watches);
    vec_act_push_back(s->activity, 0);
    vec_uint_push_back(s->levels, 0);
    vec_char_push_back(s->assigns, SATOKO_VAR_UNASSING);
    vec_char_push_back(s->polarity, sign);
    vec_uint_push_back(s->reasons, UNDEF);
    vec_uint_push_back(s->stamps, 0);
    vec_char_push_back(s->seen, 0);
    heap_insert(s->var_order, var);
    if (s->marks)
        vec_char_push_back(s->marks, 0);
    return var;
}

int satoko_add_clause(solver_t *s, int *lits, int size)
{
    unsigned i, j;
    unsigned prev_lit;
    unsigned max_var;
    unsigned cref;

    qsort((void *) lits, size, sizeof(unsigned), stk_uint_compare);
    max_var = lit2var(lits[size - 1]);
    while (max_var >= vec_act_size(s->activity))
        satoko_add_variable(s, SATOKO_LIT_FALSE);

    vec_uint_clear(s->temp_lits);
    j = 0;
    prev_lit = UNDEF;
    for (i = 0; i < (unsigned)size; i++) {
        if ((unsigned)lits[i] == lit_compl(prev_lit) || lit_value(s, lits[i]) == SATOKO_LIT_TRUE)
            return SATOKO_OK;
        else if ((unsigned)lits[i] != prev_lit && var_value(s, lit2var(lits[i])) == SATOKO_VAR_UNASSING) {
            prev_lit = lits[i];
            vec_uint_push_back(s->temp_lits, lits[i]);
        }
    }

    if (vec_uint_size(s->temp_lits) == 0) {
        s->status = SATOKO_ERR;
        return SATOKO_ERR;
    } if (vec_uint_size(s->temp_lits) == 1) {
        solver_enqueue(s, vec_uint_at(s->temp_lits, 0), UNDEF);
        return (s->status = (solver_propagate(s) == UNDEF));
    }
    if ( 0 ) {
        for ( i = 0; i < vec_uint_size(s->temp_lits); i++ ) {
            int lit = vec_uint_at(s->temp_lits, i);
            printf( "%s%d ", lit&1 ? "!":"", lit>>1 );
        }
        printf( "\n" );
    }
    cref = solver_clause_create(s, s->temp_lits, 0);
    clause_watch(s, cref);
    return SATOKO_OK;
}

void satoko_assump_push(solver_t *s, int lit)
{
    assert(lit2var(lit) < (unsigned)satoko_varnum(s));
    // printf("[Satoko] Push assumption: %d\n", lit);
    vec_uint_push_back(s->assumptions, lit);
    vec_char_assign(s->polarity, lit2var(lit), lit_polarity(lit));
}

void satoko_assump_pop(solver_t *s)
{
    assert(vec_uint_size(s->assumptions) > 0);
    // printf("[Satoko] Pop assumption: %d\n", vec_uint_pop_back(s->assumptions));
    vec_uint_pop_back(s->assumptions);
    solver_cancel_until(s, vec_uint_size(s->assumptions));
}

int satoko_solve(solver_t *s)
{
    int status = SATOKO_UNDEC;

    assert(s);
    solver_clean_stats(s);
    //if (s->opts.verbose)
    //    print_opts(s);
    if (s->status == SATOKO_ERR) {
        printf("Satoko in inconsistent state\n");
        return SATOKO_UNDEC;
    }

    if (!s->opts.no_simplify)
        if (satoko_simplify(s) != SATOKO_OK)
            return SATOKO_UNDEC;

    while (status == SATOKO_UNDEC) {
        status = solver_search(s);
        if (solver_check_limits(s) == 0 || solver_stop(s))
            break;
        if (s->nRuntimeLimit && pabc::Abc_Clock() > s->nRuntimeLimit)
            break;
        if (s->pFuncStop && s->pFuncStop(s->RunId))
            break;
    }
    if (s->opts.verbose)
        print_stats(s);
    
    solver_cancel_until(s, vec_uint_size(s->assumptions));
    return status;
}

int satoko_solve_assumptions(solver_t *s, int * plits, int nlits)
{
    int i, status;
    // printf("\n[Satoko] Solve with assumptions.. (%d)\n", vec_uint_size(s->assumptions));
    // printf("[Satoko]   + Variables: %d\n", satoko_varnum(s));
    // printf("[Satoko]   + Clauses: %d\n", satoko_clausenum(s));
    // printf("[Satoko]   + Trail size: %d\n", vec_uint_size(s->trail));
    // printf("[Satoko]   + Queue head: %d\n", s->i_qhead);
    // solver_debug_check_trail(s);
    for ( i = 0; i < nlits; i++ )
        satoko_assump_push( s, plits[i] );
    status = satoko_solve( s );
    for ( i = 0; i < nlits; i++ )
        satoko_assump_pop( s );
    return status;
}

int satoko_solve_assumptions_limit(satoko_t *s, int * plits, int nlits, int nconflim)
{
    int temp = s->opts.conf_limit, status;
    s->opts.conf_limit = nconflim ? s->stats.n_conflicts + nconflim : 0;
    status = satoko_solve_assumptions(s, plits, nlits);
    s->opts.conf_limit = temp;
    return status;
}
int satoko_minimize_assumptions(satoko_t * s, int * plits, int nlits, int nconflim)
{
    int i, nlitsL, nlitsR, nresL, nresR, status;
    if ( nlits == 1 )
    {
        // since the problem is UNSAT, we try to solve it without assuming the last literal
        // if the result is UNSAT, the last literal can be dropped; otherwise, it is needed
        status = satoko_solve_assumptions_limit( s, NULL, 0, nconflim );
        return (int)(status != SATOKO_UNSAT); // return 1 if the problem is not UNSAT
    }
    assert( nlits >= 2 );
    nlitsL = nlits / 2;
    nlitsR = nlits - nlitsL;
    // assume the left lits
    for ( i = 0; i < nlitsL; i++ )
        satoko_assump_push(s, plits[i]);
    // solve with these assumptions
    status = satoko_solve_assumptions_limit( s, NULL, 0, nconflim );
    if ( status == SATOKO_UNSAT ) // these are enough
    {
        for ( i = 0; i < nlitsL; i++ )
            satoko_assump_pop(s);
        return satoko_minimize_assumptions( s, plits, nlitsL, nconflim );
    }
    // these are not enoguh
    // solve for the right lits
    nresL = nlitsR == 1 ? 1 : satoko_minimize_assumptions( s, plits + nlitsL, nlitsR, nconflim );
    for ( i = 0; i < nlitsL; i++ )
        satoko_assump_pop(s);
    // swap literals
    vec_uint_clear(s->temp_lits);
    for ( i = 0; i < nlitsL; i++ )
        vec_uint_push_back(s->temp_lits, plits[i]);
    for ( i = 0; i < nresL; i++ )
        plits[i] = plits[nlitsL+i];
    for ( i = 0; i < nlitsL; i++ )
        plits[nresL+i] = vec_uint_at(s->temp_lits, i);
    // assume the right lits
    for ( i = 0; i < nresL; i++ )
        satoko_assump_push(s, plits[i]);
    // solve with these assumptions
    status = satoko_solve_assumptions_limit( s, NULL, 0, nconflim );
    if ( status == SATOKO_UNSAT ) // these are enough
    {
        for ( i = 0; i < nresL; i++ )
            satoko_assump_pop(s);
        return nresL;
    }
    // solve for the left lits
    nresR = nlitsL == 1 ? 1 : satoko_minimize_assumptions( s, plits + nresL, nlitsL, nconflim );
    for ( i = 0; i < nresL; i++ )
        satoko_assump_pop(s);
    return nresL + nresR;
}

int satoko_final_conflict(solver_t *s, int **out)
{
    *out = (int *)vec_uint_data(s->final_conflict);
    return vec_uint_size(s->final_conflict);
}

satoko_stats_t * satoko_stats(satoko_t *s)
{
    return &s->stats;
}

satoko_opts_t * satoko_options(satoko_t *s)
{
    return &s->opts;
}

void satoko_bookmark(satoko_t *s)
{
    // printf("[Satoko] Bookmark.\n");
    assert(s->status == SATOKO_OK);
    assert(solver_dlevel(s) == 0);
    s->book_cl_orig = vec_uint_size(s->originals);
    s->book_cl_lrnt = vec_uint_size(s->learnts);
    s->book_vars = vec_char_size(s->assigns);
    s->book_trail = vec_uint_size(s->trail);
    // s->book_qhead = s->i_qhead;
    s->opts.no_simplify = 1;
}

void satoko_unbookmark(satoko_t *s)
{
    // printf("[Satoko] Unbookmark.\n");
    assert(s->status == SATOKO_OK);
    s->book_cl_orig = 0;
    s->book_cl_lrnt = 0;
    s->book_cdb = 0;
    s->book_vars = 0;
    s->book_trail = 0;
    // s->book_qhead = 0;
    s->opts.no_simplify = 0;
}

void satoko_reset(satoko_t *s)
{
    // printf("[Satoko] Reset.\n");
    vec_uint_clear(s->assumptions);
    vec_uint_clear(s->final_conflict);
    cdb_clear(s->all_clauses);
    vec_uint_clear(s->originals);
    vec_uint_clear(s->learnts);
    vec_wl_clean(s->watches);
    vec_act_clear(s->activity);
    heap_clear(s->var_order);
    vec_uint_clear(s->levels);
    vec_uint_clear(s->reasons);
    vec_char_clear(s->assigns);
    vec_char_clear(s->polarity);
    vec_uint_clear(s->trail);
    vec_uint_clear(s->trail_lim);
    b_queue_clean(s->bq_lbd);
    b_queue_clean(s->bq_trail);
    vec_uint_clear(s->temp_lits);
    vec_char_clear(s->seen);
    vec_uint_clear(s->tagged);
    vec_uint_clear(s->stack);
    vec_uint_clear(s->last_dlevel);
    vec_uint_clear(s->stamps);
    s->status = SATOKO_OK;
    s->var_act_inc = VAR_ACT_INIT_INC;
    s->clause_act_inc = CLAUSE_ACT_INIT_INC;
    s->n_confl_bfr_reduce = s->opts.n_conf_fst_reduce;
    s->RC1 = 1;
    s->RC2 = s->opts.n_conf_fst_reduce;
    s->book_cl_orig = 0;
    s->book_cl_lrnt = 0;
    s->book_cdb = 0;
    s->book_vars = 0;
    s->book_trail = 0;
    s->i_qhead = 0;
}

void satoko_rollback(satoko_t *s)
{
    unsigned i, cref;
    unsigned n_originals = vec_uint_size(s->originals) - s->book_cl_orig;
    unsigned n_learnts = vec_uint_size(s->learnts) - s->book_cl_lrnt;
    struct clause **cl_to_remove;

    // printf("[Satoko] rollback.\n");
    assert(s->status == SATOKO_OK);
    assert(solver_dlevel(s) == 0);
    if (!s->book_vars) {
        satoko_reset(s);
        return;
    }
    cl_to_remove = satoko_alloc(struct clause *, n_originals + n_learnts);
    /* Mark clauses */
    vec_uint_foreach_start(s->originals, cref, i, s->book_cl_orig)
        cl_to_remove[i] = clause_fetch(s, cref);
    vec_uint_foreach_start(s->learnts, cref, i, s->book_cl_lrnt)
        cl_to_remove[n_originals + i] = clause_fetch(s, cref);
    for (i = 0; i < n_originals + n_learnts; i++) {
        clause_unwatch(s, cdb_cref(s->all_clauses, (unsigned *)cl_to_remove[i]));
        cl_to_remove[i]->f_mark = 1;
    }
    satoko_free(cl_to_remove);
    vec_uint_shrink(s->originals, s->book_cl_orig);
    vec_uint_shrink(s->learnts, s->book_cl_lrnt);
    /* Shrink variable related vectors */
    for (i = s->book_vars; i < 2 * vec_char_size(s->assigns); i++) {
        vec_wl_at(s->watches, i)->size = 0;
        vec_wl_at(s->watches, i)->n_bin = 0;
    }
    // s->i_qhead = s->book_qhead;
    s->watches->size = s->book_vars;
    vec_act_shrink(s->activity, s->book_vars);
    vec_uint_shrink(s->levels, s->book_vars);
    vec_uint_shrink(s->reasons, s->book_vars);
    vec_uint_shrink(s->stamps, s->book_vars);
    vec_char_shrink(s->assigns, s->book_vars);
    vec_char_shrink(s->seen, s->book_vars);
    vec_char_shrink(s->polarity, s->book_vars);
    solver_rebuild_order(s);
    /* Rewind solver and cancel level 0 assignments to the trail */
    solver_cancel_until(s, 0);
    vec_uint_shrink(s->trail, s->book_trail);
    if (s->book_cdb)
        s->all_clauses->size = s->book_cdb;
    s->book_cl_orig = 0;
    s->book_cl_lrnt = 0;
    s->book_vars = 0;
    s->book_trail = 0;
    // s->book_qhead = 0;
}

void satoko_mark_cone(satoko_t *s, int * pvars, int n_vars)
{
    int i;
    if (!solver_has_marks(s))
        s->marks = vec_char_init(satoko_varnum(s), 0);
    for (i = 0; i < n_vars; i++) {
        var_set_mark(s, pvars[i]);
        vec_sdbl_assign(s->activity, pvars[i], 0);
        if (!heap_in_heap(s->var_order, pvars[i]))
            heap_insert(s->var_order, pvars[i]);
    }
}

void satoko_unmark_cone(satoko_t *s, int *pvars, int n_vars)
{
    int i;
    assert(solver_has_marks(s));
    for (i = 0; i < n_vars; i++)
        var_clean_mark(s, pvars[i]);
}

void satoko_write_dimacs(satoko_t *s, char *fname, int wrt_lrnt, int zero_var)
{
    FILE *file;
    unsigned i;
    unsigned n_vars = vec_act_size(s->activity);
    unsigned n_orig = vec_uint_size(s->originals) + vec_uint_size(s->trail);
    unsigned n_lrnts = vec_uint_size(s->learnts);
    unsigned *array;

    assert(wrt_lrnt == 0 || wrt_lrnt == 1);
    assert(zero_var == 0 || zero_var == 1);
    if (fname != NULL)
        file = fopen(fname, "w");
    else
        file = stdout;
    
    if (file == NULL) {
        printf( "Error: Cannot open output file.\n");
        return;
    }
    fprintf(file, "p cnf %d %d\n", n_vars, wrt_lrnt ? n_orig + n_lrnts : n_orig);
    for (i = 0; i < vec_char_size(s->assigns); i++) {
        if ( var_value(s, i) != SATOKO_VAR_UNASSING ) {
            if (zero_var)
                fprintf(file, "%d\n", var_value(s, i) == SATOKO_LIT_FALSE ? -(int)(i) : i);
            else
                fprintf(file, "%d 0\n", var_value(s, i) == SATOKO_LIT_FALSE ? -(int)(i + 1) : i + 1);
        }
    }
    array = vec_uint_data(s->originals);
    for (i = 0; i < vec_uint_size(s->originals); i++)
        clause_dump(file, clause_fetch(s, array[i]), !zero_var);
    
    if (wrt_lrnt) {
        array = vec_uint_data(s->learnts);
        for (i = 0; i < n_lrnts; i++)
            clause_dump(file, clause_fetch(s, array[i]), !zero_var);
    }
    fclose(file);

}

int satoko_varnum(satoko_t *s)
{
    return vec_char_size(s->assigns);
}

int satoko_clausenum(satoko_t *s)
{
    return vec_uint_size(s->originals);
}

int satoko_learntnum(satoko_t *s)
{
    return vec_uint_size(s->learnts);
}

int satoko_conflictnum(satoko_t *s)
{
    return satoko_stats(s)->n_conflicts_all;
}

void satoko_set_stop(satoko_t *s, int * pstop)
{
    s->pstop = pstop;
}

void satoko_set_stop_func(satoko_t *s, int (*fnct)(int))
{
    s->pFuncStop = fnct;
}

void satoko_set_runid(satoko_t *s, int id)
{
    s->RunId = id;
}

int satoko_read_cex_varvalue(satoko_t *s, int ivar)
{
    return satoko_var_polarity(s, ivar) == SATOKO_LIT_TRUE;
}

pabc::abctime satoko_set_runtime_limit(satoko_t* s, pabc::abctime Limit)
{
    pabc::abctime nRuntimeLimit = s->nRuntimeLimit;
    s->nRuntimeLimit = Limit;
    return nRuntimeLimit;
}

char satoko_var_polarity(satoko_t *s, unsigned var)
{
    return vec_char_at(s->polarity, var);
}

ABC_NAMESPACE_IMPL_END

/*** solver.cpp ***/

//===--- solver.c -----------------------------------------------------------===
//
//                     satoko: Satisfiability solver
//
// This file is distributed under the BSD 2-Clause License.
// See LICENSE for details.
//
//===------------------------------------------------------------------------===
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <math.h>

#include <satoko/act_clause.h>
#include <satoko/act_var.h>
#include <satoko/solver.h>
#include <satoko/heap.h>
#include <satoko/mem.h>
#include <satoko/sort.h>

#include <abc/abc_global.h>

SATOKO_NAMESPACE_IMPL_START

//===------------------------------------------------------------------------===
// Lit funtions
//===------------------------------------------------------------------------===
/**
 *  A literal is said to be redundant in a given clause if and only if all
 *  variables in its reason are either present in that clause or (recursevely)
 *  redundant.
 */
static inline int lit_is_removable(solver_t* s, unsigned lit, unsigned min_level)
{
    unsigned top = vec_uint_size(s->tagged);

    assert(lit_reason(s, lit) != UNDEF);
    vec_uint_clear(s->stack);
    vec_uint_push_back(s->stack, lit2var(lit));
    while (vec_uint_size(s->stack)) {
        unsigned i;
        unsigned var = vec_uint_pop_back(s->stack);
        struct clause *c = clause_fetch(s, var_reason(s, var));
        unsigned *lits = &(c->data[0].lit);

        assert(var_reason(s, var) != UNDEF);
        if (c->size == 2 && lit_value(s, lits[0]) == SATOKO_LIT_FALSE) {
            assert(lit_value(s, lits[1]) == SATOKO_LIT_TRUE);
            stk_swap(unsigned, lits[0], lits[1]);
        }

        /* Check scan the literals of the reason clause.
         * The first literal is skiped because is the literal itself. */
        for (i = 1; i < c->size; i++) {
            var = lit2var(lits[i]);

            /* Check if the variable has already been seen or if it
             * was assinged a value at the decision level 0. In a
             * positive case, there is no need to look any further */
            if (vec_char_at(s->seen, var) || var_dlevel(s, var) == 0)
                continue;

            /* If the variable has a reason clause and if it was
             * assingned at a 'possible' level, then we need to
             * check if it is recursively redundant, otherwise the
             * literal being checked is not redundant */
            if (var_reason(s, var) != UNDEF && ((1 << (var_dlevel(s, var) & 31)) & min_level)) {
                vec_uint_push_back(s->stack, var);
                vec_uint_push_back(s->tagged, var);
                vec_char_assign(s->seen, var, 1);
            } else {
                vec_uint_foreach_start(s->tagged, var, i, top)
                    vec_char_assign(s->seen, var, 0);
                vec_uint_shrink(s->tagged, top);
                return 0;
            }
        }
    }
    return 1;
}

//===------------------------------------------------------------------------===
// Clause functions
//===------------------------------------------------------------------------===
/* Calculate clause LBD (Literal Block Distance):
 * - It's the number of variables in the final conflict clause that come from
 * different decision levels
 */
static inline unsigned clause_clac_lbd(solver_t *s, unsigned *lits, unsigned size)
{
    unsigned i;
    unsigned lbd = 0;

    s->cur_stamp++;
    for (i = 0; i < size; i++) {
        unsigned level = lit_dlevel(s, lits[i]);
        if (vec_uint_at(s->stamps, level) != s->cur_stamp) {
            vec_uint_assign(s->stamps, level, s->cur_stamp);
            lbd++;
        }
    }
    return lbd;
}

static inline void clause_bin_resolution(solver_t *s, vec_uint_t *clause_lits)
{
    unsigned *lits = vec_uint_data(clause_lits);
    unsigned counter, sz, i;
    unsigned lit;
    unsigned neg_lit = lit_compl(lits[0]);
    struct watcher *w;

    s->cur_stamp++;
    vec_uint_foreach(clause_lits, lit, i)
        vec_uint_assign(s->stamps, lit2var(lit), s->cur_stamp);

    counter = 0;
    watch_list_foreach_bin(s->watches, w, neg_lit) {
        unsigned imp_lit = w->blocker;
        if (vec_uint_at(s->stamps, lit2var(imp_lit)) == s->cur_stamp &&
            lit_value(s, imp_lit) == SATOKO_LIT_TRUE) {
            counter++;
            vec_uint_assign(s->stamps, lit2var(imp_lit), (s->cur_stamp - 1));
        }
    }
    if (counter > 0) {
        sz = vec_uint_size(clause_lits) - 1;
        for (i = 1; i < vec_uint_size(clause_lits) - counter; i++)
            if (vec_uint_at(s->stamps, lit2var(lits[i])) != s->cur_stamp) {
                stk_swap(unsigned, lits[i], lits[sz]);
                i--;
                sz--;
            }
        vec_uint_shrink(clause_lits, vec_uint_size(clause_lits) - counter);
    }
}

static inline void clause_minimize(solver_t *s, vec_uint_t *clause_lits)
{
    unsigned i, j;
    unsigned *lits = vec_uint_data(clause_lits);
    unsigned min_level = 0;
    unsigned clause_size;

    for (i = 1; i < vec_uint_size(clause_lits); i++) {
        unsigned level = lit_dlevel(s, lits[i]);
        min_level |= 1 << (level & 31);
    }

    /* Remove reduntant literals */
    vec_uint_foreach(clause_lits, i, j)
        vec_uint_push_back(s->tagged, lit2var(i));
    for (i = j = 1; i < vec_uint_size(clause_lits); i++)
        if (lit_reason(s, lits[i]) == UNDEF || !lit_is_removable(s, lits[i], min_level))
            lits[j++] = lits[i];
    vec_uint_shrink(clause_lits, j);

    /* Binary Resolution */
    clause_size = vec_uint_size(clause_lits);
    if (clause_size <= s->opts.clause_max_sz_bin_resol &&
        clause_clac_lbd(s, lits, clause_size) <= s->opts.clause_min_lbd_bin_resol)
        clause_bin_resolution(s, clause_lits);
}

static inline void clause_realloc(struct cdb *dest, struct cdb *src, unsigned *cref)
{
    unsigned new_cref;
    struct clause *new_clause;
    struct clause *old_clause = cdb_handler(src, *cref);

    if (old_clause->f_reallocd) {
        *cref = (unsigned) old_clause->size;
        return;
    }
    new_cref = cdb_append(dest, 3 + old_clause->f_learnt + old_clause->size);
    new_clause = cdb_handler(dest, new_cref);
    memcpy(new_clause, old_clause, (3 + old_clause->f_learnt + old_clause->size) * 4);
    old_clause->f_reallocd = 1;
    old_clause->size = (unsigned) new_cref;
    *cref = new_cref;
}

//===------------------------------------------------------------------------===
// Solver internal functions
//===------------------------------------------------------------------------===
static inline unsigned solver_decide(solver_t *s)
{
    unsigned next_var = UNDEF;

    while (next_var == UNDEF || var_value(s, next_var) != SATOKO_VAR_UNASSING) {
        if (heap_size(s->var_order) == 0) {
            next_var = UNDEF;
            return UNDEF;
        }
        next_var = heap_remove_min(s->var_order);
        if (solver_has_marks(s) && !var_mark(s, next_var))
            next_var = UNDEF;
    }
    return var2lit(next_var, satoko_var_polarity(s, next_var));
}

static inline void solver_new_decision(solver_t *s, unsigned lit)
{
    if (solver_has_marks(s) && !var_mark(s, lit2var(lit)))
        return;
    assert(var_value(s, lit2var(lit)) == SATOKO_VAR_UNASSING);
    vec_uint_push_back(s->trail_lim, vec_uint_size(s->trail));
    solver_enqueue(s, lit, UNDEF);
}

/* Calculate Backtrack Level from the learnt clause */
static inline unsigned solver_calc_bt_level(solver_t *s, vec_uint_t *learnt)
{
    unsigned i, tmp;
        unsigned i_max = 1;
    unsigned *lits = vec_uint_data(learnt);
        unsigned max = lit_dlevel(s, lits[1]);

    if (vec_uint_size(learnt) == 1)
        return 0;
    for (i = 2; i < vec_uint_size(learnt); i++) {
        if (lit_dlevel(s, lits[i]) > max) {
            max   = lit_dlevel(s, lits[i]);
            i_max = i;
        }
    }
        tmp         = lits[1];
        lits[1]     = lits[i_max];
        lits[i_max] = tmp;
        return lit_dlevel(s, lits[1]);
}

/**
 *  Most books and papers explain conflict analysis and the calculation of the
 *  1UIP (first Unique Implication Point) using an implication graph. This
 *  function, however, do not explicity constructs the graph! It inspects the
 *  trail in reverse and figure out which literals should be added to the
 *  to-be-learnt clause using the reasons of each assignment.
 *
 *  cur_lit: current literal being analyzed.
 *  n_paths: number of unprocessed paths from conlfict node to the current
 *           literal being analyzed (cur_lit).
 *
 *  This functions performs a backward BFS (breadth-first search) for 1UIP node.
 *  The trail works as the BFS queue. The counter of unprocessed but seen
 *  variables (n_paths) allows us to identify when 'cur_lit' is the closest
 *  cause of conflict.
 *
 *  When 'n_paths' reaches zero it means there are no unprocessed reverse paths
 *  back from the conflict node to 'cur_lit' - meaning it is the 1UIP decision
 *  variable.
 *
 */
static inline void solver_analyze(solver_t *s, unsigned cref, vec_uint_t *learnt,
                      unsigned *bt_level, unsigned *lbd)
{
    unsigned i;
    unsigned *trail = vec_uint_data(s->trail);
    unsigned idx = vec_uint_size(s->trail) - 1;
    unsigned n_paths = 0;
    unsigned p = UNDEF;
    unsigned var;

    vec_uint_push_back(learnt, UNDEF);
    do {
        struct clause *clause;
        unsigned *lits;
        unsigned j;

        assert(cref != UNDEF);
        clause = clause_fetch(s, cref);
        lits = &(clause->data[0].lit);

        if (p != UNDEF && clause->size == 2 && lit_value(s, lits[0]) == SATOKO_LIT_FALSE) {
            assert(lit_value(s, lits[1]) == SATOKO_LIT_TRUE);
            stk_swap(unsigned, lits[0], lits[1] );
        }

        if (clause->f_learnt)
            clause_act_bump(s, clause);

        if (clause->f_learnt && clause->lbd > 2) {
            unsigned n_levels = clause_clac_lbd(s, lits, clause->size);
            if (n_levels + 1 < clause->lbd) {
                if (clause->lbd <= s->opts.lbd_freeze_clause)
                    clause->f_deletable = 0;
                clause->lbd = n_levels;
            }
        }

        for (j = (p == UNDEF ? 0 : 1); j < clause->size; j++) {
            var = lit2var(lits[j]);
            if (vec_char_at(s->seen, var) || var_dlevel(s, var) == 0)
                continue;
            vec_char_assign(s->seen, var, 1);
            var_act_bump(s, var);
            if (var_dlevel(s, var) == solver_dlevel(s)) {
                n_paths++;
                if (var_reason(s, var) != UNDEF && clause_fetch(s, var_reason(s, var))->f_learnt)
                    vec_uint_push_back(s->last_dlevel, var);
            } else
                vec_uint_push_back(learnt, lits[j]);
        }

        while (!vec_char_at(s->seen, lit2var(trail[idx--])));

        p = trail[idx + 1];
        cref = lit_reason(s, p);
        vec_char_assign(s->seen, lit2var(p), 0);
        n_paths--;
    } while (n_paths > 0);

    vec_uint_data(learnt)[0] = lit_compl(p);
    clause_minimize(s, learnt);
    *bt_level = solver_calc_bt_level(s, learnt);
    *lbd = clause_clac_lbd(s, vec_uint_data(learnt), vec_uint_size(learnt));

    if (vec_uint_size(s->last_dlevel) > 0) {
        vec_uint_foreach(s->last_dlevel, var, i) {
            if (clause_fetch(s, var_reason(s, var))->lbd < *lbd)
                var_act_bump(s, var);
        }
        vec_uint_clear(s->last_dlevel);
    }
    vec_uint_foreach(s->tagged, var, i)
        vec_char_assign(s->seen, var, 0);
    vec_uint_clear(s->tagged);
}

static inline int solver_rst(solver_t *s)
{
    return b_queue_is_valid(s->bq_lbd) &&
           (((long)b_queue_avg(s->bq_lbd) * s->opts.f_rst) > (s->sum_lbd / s->stats.n_conflicts));
}

static inline int solver_block_rst(solver_t *s)
{
    return s->stats.n_conflicts > (int)s->opts.fst_block_rst &&
           b_queue_is_valid(s->bq_lbd) &&
           ((long)vec_uint_size(s->trail) > (s->opts.b_rst * (long)b_queue_avg(s->bq_trail)));
}

static inline void solver_handle_conflict(solver_t *s, unsigned confl_cref)
{
    unsigned bt_level;
    unsigned lbd;
    unsigned cref;

    vec_uint_clear(s->temp_lits);
    solver_analyze(s, confl_cref, s->temp_lits, &bt_level, &lbd);
    s->sum_lbd += lbd;
    b_queue_push(s->bq_lbd, lbd);
    solver_cancel_until(s, bt_level);
    cref = UNDEF;
    if (vec_uint_size(s->temp_lits) > 1) {
        cref = solver_clause_create(s, s->temp_lits, 1);
        clause_watch(s, cref);
    }
    solver_enqueue(s, vec_uint_at(s->temp_lits, 0), cref);
    var_act_decay(s);
    clause_act_decay(s);
}

static inline void solver_analyze_final(solver_t *s, unsigned lit)
{
    unsigned i;

    // printf("[Satoko] Analize final..\n");
    // printf("[Satoko] Conflicting lit: %d\n", lit);
    vec_uint_clear(s->final_conflict);
    vec_uint_push_back(s->final_conflict, lit);
    if (solver_dlevel(s) == 0)
        return;
    vec_char_assign(s->seen, lit2var(lit), 1);
    for (i = vec_uint_size(s->trail); i --> vec_uint_at(s->trail_lim, 0);) {
        unsigned var = lit2var(vec_uint_at(s->trail, i));

        if (vec_char_at(s->seen, var)) {
            unsigned reason = var_reason(s, var);
            if (reason == UNDEF) {
                assert(var_dlevel(s, var) > 0);
                vec_uint_push_back(s->final_conflict, lit_compl(vec_uint_at(s->trail, i)));
            } else {
                unsigned j;
                struct clause *clause = clause_fetch(s, reason);
                for (j = (clause->size == 2 ? 0 : 1); j < clause->size; j++) {
                    if (lit_dlevel(s, clause->data[j].lit) > 0)
                        vec_char_assign(s->seen, lit2var(clause->data[j].lit), 1);
                }
            }
            vec_char_assign(s->seen, var, 0);
        }
    }
    vec_char_assign(s->seen, lit2var(lit), 0);
    // solver_debug_check_unsat(s);
}

static inline void solver_garbage_collect(solver_t *s)
{
    unsigned i;
    unsigned *array;
    struct cdb *new_cdb = cdb_alloc(cdb_capacity(s->all_clauses) - cdb_wasted(s->all_clauses));

    if (s->book_cdb)
        s->book_cdb = 0;

    for (i = 0; i < 2 * vec_char_size(s->assigns); i++) {
        struct watcher *w;
        watch_list_foreach(s->watches, w, i)
            clause_realloc(new_cdb, s->all_clauses, &(w->cref));
    }

    /* Update CREFS */
    for (i = 0; i < vec_uint_size(s->trail); i++)
        if (lit_reason(s, vec_uint_at(s->trail, i)) != UNDEF)
            clause_realloc(new_cdb, s->all_clauses, &(vec_uint_data(s->reasons)[lit2var(vec_uint_at(s->trail, i))]));

    array = vec_uint_data(s->learnts);
    for (i = 0; i < vec_uint_size(s->learnts); i++)
        clause_realloc(new_cdb, s->all_clauses, &(array[i]));

    array = vec_uint_data(s->originals);
    for (i = 0; i < vec_uint_size(s->originals); i++)
        clause_realloc(new_cdb, s->all_clauses, &(array[i]));

    cdb_free(s->all_clauses);
    s->all_clauses = new_cdb;
}

static inline void solver_reduce_cdb(solver_t *s)
{
    unsigned i, limit;
    unsigned n_learnts = vec_uint_size(s->learnts);
    unsigned cref;
    struct clause *clause;
    struct clause **learnts_cls;

    learnts_cls = satoko_alloc(struct clause *, n_learnts);
    vec_uint_foreach_start(s->learnts, cref, i, s->book_cl_lrnt)
        learnts_cls[i] = clause_fetch(s, cref);

    limit = (unsigned)(n_learnts * s->opts.learnt_ratio);

    satoko_sort((void **)learnts_cls, n_learnts,
            (int (*)(const void *, const void *)) clause_compare);

    if (learnts_cls[n_learnts / 2]->lbd <= 3)
        s->RC2 += s->opts.inc_special_reduce;
    if (learnts_cls[n_learnts - 1]->lbd <= 6)
        s->RC2 += s->opts.inc_special_reduce;

    vec_uint_clear(s->learnts);
    for (i = 0; i < n_learnts; i++) {
        clause = learnts_cls[i];
        cref = cdb_cref(s->all_clauses, (unsigned *)clause);
        assert(clause->f_mark == 0);
        if (clause->f_deletable && clause->lbd > 2 && clause->size > 2 && lit_reason(s, clause->data[0].lit) != cref && (i < limit)) {
            clause->f_mark = 1;
            s->stats.n_learnt_lits -= clause->size;
            clause_unwatch(s, cref);
            cdb_remove(s->all_clauses, clause);
        } else {
            if (!clause->f_deletable)
                limit++;
            clause->f_deletable = 1;
            vec_uint_push_back(s->learnts, cref);
        }
    }
    satoko_free(learnts_cls);

    if (s->opts.verbose) {
        printf("reduceDB: Keeping %7d out of %7d clauses (%5.2f %%) \n",
               vec_uint_size(s->learnts), n_learnts,
               100.0 * vec_uint_size(s->learnts) / n_learnts);
        fflush(stdout);
    }
    if (cdb_wasted(s->all_clauses) > cdb_size(s->all_clauses) * s->opts.garbage_max_ratio)
        solver_garbage_collect(s);
}

//===------------------------------------------------------------------------===
// Solver external functions
//===------------------------------------------------------------------------===
unsigned solver_clause_create(solver_t *s, vec_uint_t *lits, unsigned f_learnt)
{
    struct clause *clause;
    unsigned cref;
    unsigned n_words;

    assert(vec_uint_size(lits) > 1);
    assert(f_learnt == 0 || f_learnt == 1);

    n_words = 3 + f_learnt + vec_uint_size(lits);
    cref = cdb_append(s->all_clauses, n_words);
    clause = clause_fetch(s, cref);
    clause->f_learnt = f_learnt;
    clause->f_mark = 0;
    clause->f_reallocd = 0;
    clause->f_deletable = f_learnt;
    clause->size = vec_uint_size(lits);
    memcpy(&(clause->data[0].lit), vec_uint_data(lits), sizeof(unsigned) * vec_uint_size(lits));

    if (f_learnt) {
        vec_uint_push_back(s->learnts, cref);
        clause->lbd = clause_clac_lbd(s, vec_uint_data(lits), vec_uint_size(lits));
        clause->data[clause->size].act = 0;
        s->stats.n_learnt_lits += vec_uint_size(lits);
        clause_act_bump(s, clause);
    } else {
        vec_uint_push_back(s->originals, cref);
        s->stats.n_original_lits += vec_uint_size(lits);
    }
    return cref;
}

void solver_cancel_until(solver_t *s, unsigned level)
{
    unsigned i;

    if (solver_dlevel(s) <= level)
        return;
    for (i = vec_uint_size(s->trail); i --> vec_uint_at(s->trail_lim, level);) {
        unsigned var = lit2var(vec_uint_at(s->trail, i));

        vec_char_assign(s->assigns, var, SATOKO_VAR_UNASSING);
        vec_uint_assign(s->reasons, var, UNDEF);
        if (!heap_in_heap(s->var_order, var))
            heap_insert(s->var_order, var);
    }
    s->i_qhead = vec_uint_at(s->trail_lim, level);
    vec_uint_shrink(s->trail, vec_uint_at(s->trail_lim, level));
    vec_uint_shrink(s->trail_lim, level);
}

unsigned solver_propagate(solver_t *s)
{
    unsigned conf_cref = UNDEF;
    unsigned *lits;
    unsigned neg_lit;
    unsigned n_propagations = 0;

    while (s->i_qhead < vec_uint_size(s->trail)) {
        unsigned p = vec_uint_at(s->trail, s->i_qhead++);
        struct watch_list *ws;
        struct watcher *begin;
        struct watcher *end;
        struct watcher *i, *j;

        n_propagations++;
        watch_list_foreach_bin(s->watches, i, p) {
            if (solver_has_marks(s) && !var_mark(s, lit2var(i->blocker)))
                continue;
            if (var_value(s, lit2var(i->blocker)) == SATOKO_VAR_UNASSING)
                solver_enqueue(s, i->blocker, i->cref);
            else if (lit_value(s, i->blocker) == SATOKO_LIT_FALSE)
                return i->cref;
        }

        ws = vec_wl_at(s->watches, p);
        begin = watch_list_array(ws);
        end = begin + watch_list_size(ws);
        for (i = j = begin + ws->n_bin; i < end;) {
            struct clause *clause;
            struct watcher w;

            if (solver_has_marks(s) && !var_mark(s, lit2var(i->blocker))) {
                *j++ = *i++;
                continue;
            }
            if (lit_value(s, i->blocker) == SATOKO_LIT_TRUE) {
                *j++ = *i++;
                continue;
            }

            clause = clause_fetch(s, i->cref);
            lits = &(clause->data[0].lit);

            // Make sure the false literal is data[1]:
            neg_lit = lit_compl(p);
            if (lits[0] == neg_lit)
                stk_swap(unsigned, lits[0], lits[1]);
            assert(lits[1] == neg_lit);

            w.cref = i->cref;
            w.blocker = lits[0];

            /* If 0th watch is true, then clause is already satisfied. */
            if (lits[0] != i->blocker && lit_value(s, lits[0]) == SATOKO_LIT_TRUE)
                *j++ = w;
            else {
                /* Look for new watch */
                unsigned k;
                for (k = 2; k < clause->size; k++) {
                    if (lit_value(s, lits[k]) != SATOKO_LIT_FALSE) {
                        lits[1] = lits[k];
                        lits[k] = neg_lit;
                        watch_list_push(vec_wl_at(s->watches, lit_compl(lits[1])), w, 0);
                        goto next;
                    }
                }

                *j++ = w;

                /* Clause becomes unit under this assignment */
                if (lit_value(s, lits[0]) == SATOKO_LIT_FALSE) {
                    conf_cref = i->cref;
                    s->i_qhead = vec_uint_size(s->trail);
                    i++;
                    // Copy the remaining watches:
                    while (i < end)
                        *j++ = *i++;
                } else
                    solver_enqueue(s, lits[0], i->cref);
            }
        next:
            i++;
        }

        s->stats.n_inspects += j - watch_list_array(ws);
        watch_list_shrink(ws, j - watch_list_array(ws));
    }
    s->stats.n_propagations += n_propagations;
    s->stats.n_propagations_all += n_propagations;
    s->n_props_simplify -= n_propagations;
    return conf_cref;
}

char solver_search(solver_t *s)
{
    s->stats.n_starts++;
    while (1) {
        unsigned confl_cref = solver_propagate(s);
        if (confl_cref != UNDEF) {
            s->stats.n_conflicts++;
            s->stats.n_conflicts_all++;
            if (solver_dlevel(s) == 0)
                return SATOKO_UNSAT;
            /* Restart heuristic */
            b_queue_push(s->bq_trail, vec_uint_size(s->trail));
            if (solver_block_rst(s))
                b_queue_clean(s->bq_lbd);
            solver_handle_conflict(s, confl_cref);
        } else {
            // solver_debug_check_clauses(s);
            /* No conflict */
            unsigned next_lit;

            if (solver_rst(s) || solver_check_limits(s) == 0 || solver_stop(s) || 
                (s->nRuntimeLimit && (s->stats.n_conflicts & 63) == 0 && pabc::Abc_Clock() > s->nRuntimeLimit)) {
                b_queue_clean(s->bq_lbd);
                solver_cancel_until(s, 0);
                return SATOKO_UNDEC;
            }
            if (!s->opts.no_simplify && solver_dlevel(s) == 0)
                satoko_simplify(s);

            /* Reduce the set of learnt clauses */
            if (s->opts.learnt_ratio && vec_uint_size(s->learnts) > 100 &&
                s->stats.n_conflicts >= s->n_confl_bfr_reduce) {
                s->RC1 = (s->stats.n_conflicts / s->RC2) + 1;
                solver_reduce_cdb(s);
                s->RC2 += s->opts.inc_reduce;
                s->n_confl_bfr_reduce = s->RC1 * s->RC2;
            }

            /* Make decisions based on user assumptions */
            next_lit = UNDEF;
            while (solver_dlevel(s) < vec_uint_size(s->assumptions)) {
                unsigned lit = vec_uint_at(s->assumptions, solver_dlevel(s));
                if (lit_value(s, lit) == SATOKO_LIT_TRUE) {
                    vec_uint_push_back(s->trail_lim, vec_uint_size(s->trail));
                } else if (lit_value(s, lit) == SATOKO_LIT_FALSE) {
                    solver_analyze_final(s, lit_compl(lit));
                    return SATOKO_UNSAT;
                } else {
                    next_lit = lit;
                    break;
                }

            }
            if (next_lit == UNDEF) {
                s->stats.n_decisions++;
                next_lit = solver_decide(s);
                if (next_lit == UNDEF) {
                    // solver_debug_check(s, SATOKO_SAT);
                    return SATOKO_SAT;
                }
            }
            solver_new_decision(s, next_lit);
        }
    }
}

//===------------------------------------------------------------------------===
// Debug procedures
//===------------------------------------------------------------------------===
void solver_debug_check_trail(solver_t *s)
{
    unsigned i;
    unsigned *array;
    vec_uint_t *trail_dup = vec_uint_alloc(0);
    fprintf(stdout, "[Satoko] Checking for trail(%u) inconsistencies...\n", vec_uint_size(s->trail));
    vec_uint_duplicate(trail_dup, s->trail);
    vec_uint_sort(trail_dup, 1);
    array = vec_uint_data(trail_dup);
    for (i = 1; i < vec_uint_size(trail_dup); i++) {
        if (array[i - 1] == lit_compl(array[i])) {
            fprintf(stdout, "[Satoko] Inconsistent trail: %u %u\n", array[i - 1], array[i]);
            assert(0);
            return;
        }
    }
    for (i = 0; i < vec_uint_size(trail_dup); i++) {
        if (var_value(s, lit2var(array[i])) != lit_polarity(array[i])) {
            fprintf(stdout, "[Satoko] Inconsistent trail assignment: %u, %u\n", vec_char_at(s->assigns, lit2var(array[i])), array[i]);
            assert(0);
            return;
        }
    }
    fprintf(stdout, "[Satoko] Trail OK.\n");
    vec_uint_print(trail_dup);
    vec_uint_free(trail_dup);

}

void solver_debug_check_clauses(solver_t *s)
{
    unsigned cref, i;

    fprintf(stdout, "[Satoko] Checking clauses (%d)...\n", vec_uint_size(s->originals));
    vec_uint_foreach(s->originals, cref, i) {
        unsigned j;
        struct clause *clause = clause_fetch(s, cref);
        for (j = 0; j < clause->size; j++) {
            if (vec_uint_find(s->trail, lit_compl(clause->data[j].lit))) {
                continue;
            }
            break;
        }
        if (j == clause->size) {
            vec_uint_print(s->trail);
            fprintf(stdout, "[Satoko] FOUND UNSAT CLAUSE]: (%d) ", i);
            clause_print(clause);
            assert(0);
        }
    }
    fprintf(stdout, "[Satoko] All SAT - OK\n");
}

void solver_debug_check(solver_t *s, int result)
{
    unsigned cref, i;
    solver_debug_check_trail(s);
    fprintf(stdout, "[Satoko] Checking clauses (%d)... \n", vec_uint_size(s->originals));
    vec_uint_foreach(s->originals, cref, i) {
        unsigned j;
        struct clause *clause = clause_fetch(s, cref);
        for (j = 0; j < clause->size; j++) {
            if (vec_uint_find(s->trail, clause->data[j].lit)) {
                break;
            }
        }
        if (result == SATOKO_SAT && j == clause->size) {
            fprintf(stdout, "[Satoko] FOUND UNSAT CLAUSE: (%d) ", i);
            clause_print(clause);
            assert(0);
        }
    }
    fprintf(stdout, "[Satoko] All SAT - OK\n");
}

ABC_NAMESPACE_IMPL_END
