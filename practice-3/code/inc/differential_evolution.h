#pragma once

#include "mh.h"
#include "ea_common.h"

#include <vector>

/**
 * @file differential_evolution.h
 * @brief Differential Evolution metaheuristic for continuous optimization.
 *
 * This implementation uses classic DE operators:
 * - Differential mutation with factor F.
 * - Binomial crossover with rate CR.
 * - Deterministic replacement when the trial is better.
 */

/**
 * @brief Differential Evolution algorithm.
 *
 * The class stores the best fitness found after each generation so the
 * convergence curve can be plotted later.
 */
class DifferentialEvolution : public MH<double> {
private:
    /** Scale factor used in differential mutation (F). */
    double m_f;
    /** Crossover rate used in binomial crossover (CR). */
    double m_cr;
    /** Requested population size. */
    int m_pop_size;

    /** Best fitness value recorded at the end of each generation. */
    std::vector<double> m_last_convergence;

public:
    /**
     * Build a Differential Evolution optimizer.
     *
     * @param f Differential mutation scale factor.
     * @param cr Binomial crossover rate.
     * @param pop_size Target population size.
     */
    DifferentialEvolution(double f, double cr, int pop_size)
        : m_f(f), m_cr(cr), m_pop_size(pop_size) {}

    /**
     * Optimize a problem using Differential Evolution.
     *
     * @param problem Problem instance to solve.
     * @param maxevals Maximum number of fitness evaluations.
     * @return Final solution, final fitness, and evaluations consumed.
     */
    ResultMH<double> optimize(Problem<double> &problem, int maxevals) override;

    /**
     * Get convergence values from the last run.
     *
     * @return Best fitness per generation.
     */
    const std::vector<double> &last_convergence() const { return m_last_convergence; }
};