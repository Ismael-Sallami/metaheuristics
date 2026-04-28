#pragma once

#include "problem.h"

/**
 * @file gaussian_mutation.h
 * @brief Gaussian mutation operator for real-valued individuals.
 *
 * The function in this header applies a single-point Gaussian perturbation
 * to one component of a solution, then repairs the solution with problem.fix().
 */

/**
 * @brief Mutate one individual using Gaussian noise.
 *
 * The operator evaluates mutation gene by gene.
 * To keep compatibility with the existing GA configuration, the input
 * `pm_indiv` is converted internally to an effective gene probability:
 * `pm_gene = pm_indiv / solution_size`.
 * Each selected gene receives Gaussian noise with standard deviation `sigma`.
 * After mutation, the solution is repaired to satisfy problem constraints.
 *
 * @param solution Solution vector to mutate.
 * @param problem Problem instance used to repair constraints.
 * @param pm_indiv Target mutation budget per individual.
 * @param sigma Standard deviation of Gaussian noise.
 */
void gaussian_mutate_individual(
    tSolution<double> &solution,
    Problem<double> &problem,
    double pm_indiv,
    double sigma
);