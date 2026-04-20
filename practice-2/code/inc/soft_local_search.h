#pragma once

#include "problem.h"
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