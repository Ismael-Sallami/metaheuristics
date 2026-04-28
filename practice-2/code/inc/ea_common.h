#pragma once

#include <vector>
#include <algorithm>
#include <cmath>
#include "problem.h"
#include "random.hpp"
#include <cassert>

/**
 * @file ea_common.h
 * @brief Shared helper utilities for evolutionary algorithms.
 *
 * This file groups small reusable functions used by GA, AM and DE modules.
 * The helpers here manage common tasks such as:
 * - Population initialization.
 * - Fitness-based comparisons and rankings.
 * - Parent selection.
 * - Crossover operators.
 * - Simple transfer mutation.
 */

/**
 * @brief Basic individual representation for evolutionary algorithms.
 *
 * Each individual stores:
 * - solution: vector of decision variables.
 * - fitness: objective value for that solution.
 */
struct EAIndividual {
    tSolution<double> solution;
    tFitness fitness;
};

/**
 * @brief Returns true when fitness @p a is better than fitness @p b.
 *
 * In this project the objective is maximization, so larger fitness is better.
 */
inline bool ea_better(tFitness a, tFitness b) {
    return a > b;
}

/**
 * @brief Gets the index of the best individual in a population.
 *
 * @param pop Population to inspect.
 * @return Index of the individual with best fitness according to ea_better().
 */
inline int ea_best_index(const std::vector<EAIndividual> &pop) {
    assert(!pop.empty());
    int best = 0; // Assume the first individual is best until we find a better one.
    for (int i = 1; i < static_cast<int>(pop.size()); ++i) {
        if (ea_better(pop[i].fitness, pop[best].fitness)) {
            best = i;
        }
    }
    return best;
}

/**
 * @brief Gets the index of the worst individual in a population.
 *
 * @param pop Population to inspect.
 * @return Index of the individual with worst fitness according to ea_better().
 */
inline int ea_worst_index(const std::vector<EAIndividual> &pop) {
    assert(!pop.empty());
    int worst = 0; // Assume the first individual is worst until we find a worse one.
    for (int i = 1; i < static_cast<int>(pop.size()); ++i) {
        if (ea_better(pop[worst].fitness, pop[i].fitness)) {
            worst = i;
        }
    }
    return worst;
}

/**
 * @brief Repairs and evaluates one individual.
 *
 * The function first calls problem.fix() to enforce feasibility,
 * then computes fitness, and finally increments the evaluation counter.
 *
 * @param problem Optimization problem.
 * @param ind Individual to evaluate.
 * @param evals Evaluation counter (updated by reference).
 */
inline void ea_evaluate_individual(Problem<double> &problem, EAIndividual &ind, unsigned int &evals) {
    assert(!ind.solution.empty());
    problem.fix(ind.solution);
    ind.fitness = problem.fitness(ind.solution);
    ++evals;
}

/**
 * @brief Creates and evaluates an initial random population.
 *
 * @param problem Optimization problem.
 * @param pop_size Number of individuals to generate.
 * @param evals Evaluation counter (updated by reference).
 * @return A fully evaluated population.
 */
inline std::vector<EAIndividual> ea_initialize_population(
    Problem<double> &problem,
    int pop_size,
    unsigned int &evals
) {
    std::vector<EAIndividual> pop;
    pop.reserve(pop_size);

    for (int i = 0; i < pop_size; ++i) {
        EAIndividual ind;
        ind.solution = problem.createSolution();
        ea_evaluate_individual(problem, ind, evals);
        pop.push_back(ind);
    }

    return pop;
}

/**
 * @brief Runs a tournament selection with k=3.
 *
 * Three random indices are sampled and the best one is returned.
 * Sampling is done with replacement.
 *
 * @param pop Population used as tournament pool.
 * @return Index of the tournament winner.
 */
inline int ea_tournament_k3(const std::vector<EAIndividual> &pop) {
    const int n = static_cast<int>(pop.size());
    int a = Random::get<int>(0, n - 1);
    int b = Random::get<int>(0, n - 1);
    int c = Random::get<int>(0, n - 1);

    int best = a;
    if (ea_better(pop[b].fitness, pop[best].fitness)) best = b;
    if (ea_better(pop[c].fitness, pop[best].fitness)) best = c;
    return best;
}

/**
 * @brief Arithmetic crossover operator.
 *
 * For each gene, a random sigma in [0,1] is sampled and two children are built
 * as convex combinations of parent genes.
 *
 * @param p1 First parent.
 * @param p2 Second parent.
 * @param c1 First child (output).
 * @param c2 Second child (output).
 */
inline void ea_arithmetic_crossover(
    const tSolution<double> &p1,
    const tSolution<double> &p2,
    tSolution<double> &c1,
    tSolution<double> &c2
) {
    const int n = static_cast<int>(p1.size());
    c1.resize(n);
    c2.resize(n);

    for (int i = 0; i < n; ++i) {
        double sigma = Random::get<double>(0.0, 1.0);
        c1[i] = sigma * p1[i] + (1.0 - sigma) * p2[i];
        c2[i] = sigma * p2[i] + (1.0 - sigma) * p1[i];
    }
}

/**
 * @brief BLX-alpha crossover operator.
 *
 * For each gene, children are sampled uniformly from the extended interval:
 * [min(p1,p2) - alpha*I, max(p1,p2) + alpha*I], where I is parent distance.
 *
 * @param p1 First parent.
 * @param p2 Second parent.
 * @param alpha BLX expansion parameter.
 * @param c1 First child (output).
 * @param c2 Second child (output).
 */
inline void ea_blx_crossover(
    const tSolution<double> &p1,
    const tSolution<double> &p2,
    double alpha,
    tSolution<double> &c1,
    tSolution<double> &c2
) {
    const int n = static_cast<int>(p1.size());
    c1.resize(n);
    c2.resize(n);

    for (int i = 0; i < n; ++i) {
        const double cmax = std::max(p1[i], p2[i]);
        const double cmin = std::min(p1[i], p2[i]);
        const double interval = cmax - cmin; // we don't need fabs() since cmax >= cmin
        const double low = cmin - alpha * interval;
        const double high = cmax + alpha * interval;

        c1[i] = Random::get<double>(low, high);
        c2[i] = Random::get<double>(low, high);
    }
}

/**
 * @brief Transfer mutation between two random positions.
 *
 * With probability pm_indiv, the operator moves a fraction of one gene value
 * (transfer_ratio) from a random source position i to a different position j.
 * The result is repaired with problem.fix().
 *
 * @param sol Solution to mutate.
 * @param problem Optimization problem (used for repair).
 * @param pm_indiv Mutation probability per individual.
 * @param transfer_ratio Fraction transferred from i to j.
 */
inline void ea_mutate_transfer(
    tSolution<double> &sol,
    Problem<double> &problem,
    double pm_indiv,
    double transfer_ratio
) {
    if (Random::get<double>(0.0, 1.0) > pm_indiv) {
        return;
    }

    const int n = static_cast<int>(sol.size());
    if (n < 2) { // Not enough genes to transfer, skip mutation.
        return;
    }

    int i = Random::get<int>(0, n - 1);
    int j = Random::get<int>(0, n - 1);
    while (j == i) {
        j = Random::get<int>(0, n - 1);
    }

    const double amount = sol[i] * transfer_ratio;
    sol[i] -= amount;
    sol[j] += amount;

    problem.fix(sol);
}


/**
 * @brief Returns indices of the best k individuals in descending quality order.
 *
 * This function sorts only integer indices, not full individuals, to reduce
 * unnecessary data movement.
 *
 * @param pop Population to rank.
 * @param k Number of top indices requested.
 * @return Vector with the best k indices (or all indices if k is large).
 */
inline std::vector<int> ea_best_k_indices(const std::vector<EAIndividual> &pop, int k) {
    std::vector<int> idx(pop.size());
    for (int i = 0; i < static_cast<int>(pop.size()); ++i) {
        idx[i] = i;
    }

    const int limit = std::min(k, static_cast<int>(idx.size()));
    if (limit > 0) { // Only sort if we need at least one index. Using partial_sort to get the top k without fully sorting the entire population.
        std::partial_sort(idx.begin(), idx.begin() + limit, idx.end(), [&](int a, int b) { 
            return ea_better(pop[a].fitness, pop[b].fitness);
        });
    }
    if (k < static_cast<int>(idx.size())) {
        idx.resize(k);
    }
    

    return idx;
}