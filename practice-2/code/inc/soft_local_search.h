#pragma once

#include "problem.h"
#include <utility>
#include <vector>

/**
 * @file soft_local_search.h
 * @brief Soft local search operator used as an intensification step.
 *
 * The function in this file explores transfer-based neighbors.
 * It tries to improve the current solution by moving a small ratio
 * of value from one position to another.
 */

/**
 * @brief Persistent state for chained soft local search.
 *
 * The state stores the neighborhood order and the next index to evaluate,
 * so the search can pause and resume later without restarting from scratch.
 */
struct SoftLSState {
    std::vector<std::pair<int, int>> neighborhood;
    int cursor = 0;
    int dimension = -1;
};

/**
 * @brief Reset a soft local-search chain state.
 */
void reset_soft_local_search_state(SoftLSState &state);

/**
 * @brief Apply a first-improvement soft local search.
 *
 * At each move, the method transfers `ratio` of one component to another.
 * The search stops when it reaches `max_local_evals` evaluations or when
 * no improving move is found in a full neighborhood pass.
 *
 * @param problem Problem used to evaluate neighbors.
 * @param solution Current solution (updated in place if improvements appear).
 * @param fitness Fitness of the current solution (updated with improvements).
 * @param ratio Fraction transferred in each candidate move.
 * @param max_local_evals Maximum number of fitness evaluations allowed.
 * @return Number of evaluations consumed by the local search.
 */
int apply_soft_local_search(
    Problem<double> &problem,
    tSolution<double> &solution,
    tFitness &fitness,
    double ratio,
    int max_local_evals
);

/**
 * @brief Apply chained first-improvement soft local search.
 *
 * Unlike the stateless overload, this version reuses @p state so neighborhood
 * exploration continues from the previous call.
 *
 * @param problem Problem used to evaluate neighbors.
 * @param solution Current solution (updated in place if improvements appear).
 * @param fitness Fitness of the current solution (updated with improvements).
 * @param ratio Fraction transferred in each candidate move.
 * @param max_local_evals Maximum number of fitness evaluations allowed.
 * @param state Persistent local-search chain state.
 * @return Number of evaluations consumed by the local search.
 */
int apply_soft_local_search(
    Problem<double> &problem,
    tSolution<double> &solution,
    tFitness &fitness,
    double ratio,
    int max_local_evals,
    SoftLSState &state
);