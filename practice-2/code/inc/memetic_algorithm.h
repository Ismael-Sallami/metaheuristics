#pragma once

#include "mh.h"
#include "ea_common.h"
#include "genetic_algorithm.h"

#include <vector>

/**
 * @file memetic_algorithm.h
 * @brief Memetic algorithm for portfolio optimization.
 *
 * This algorithm combines a genetic algorithm cycle with a local search stage.
 * Different variants control which individuals receive local search.
 */

/**
 * @brief Variant used to choose local-search candidates.
 */
enum class MemeticVariant {
    /** Apply local search to all individuals. */
    ALL,
    /** Apply local search to random individuals. */
    RAND,
    /** Apply local search to the best individuals. */
    BEST
};

/**
 * @brief Memetic algorithm with GA operators plus periodic local search.
 *
 * The algorithm keeps a convergence history with the best fitness found in
 * each generation.
 */
class MemeticAlgorithm : public MH<double> {
private:
    /** Selection rule for local-search candidates. */
    MemeticVariant m_variant;
    /** Crossover operator used during reproduction. */
    CrossoverType m_crossover_type;

    /** Population size. */
    int m_pop_size;
    /** Probability of applying crossover to a parent pair. */
    double m_pc;
    /** Probability of mutating one individual. */
    double m_pm_indiv;
    /** BLX-alpha expansion factor (used when BLX crossover is selected). */
    double m_blx_alpha;
    /** Transfer ratio used by mutation. */
    double m_mutation_ratio;

    /** Number of generations between local-search phases. */
    int m_ls_period;
    /** Max evaluations allowed per local-search call. */
    int m_ls_max_evals;
    /** Transfer ratio used inside soft local search. */
    double m_ls_ratio;
    /** Probability to select an individual in RAND variant. */
    double m_pls_rand;

    /** Best fitness value recorded after each generation. */
    std::vector<double> m_last_convergence;

    /**
     * Apply the selected crossover operator to one pair of parents.
     */
    void crossover_pair(
        const tSolution<double> &p1,
        const tSolution<double> &p2,
        tSolution<double> &c1,
        tSolution<double> &c2
    ) const;

public:
    /**
     * Build a memetic algorithm with GA and local-search parameters.
     *
     * @param variant Local-search selection variant (ALL, RAND or BEST).
     * @param crossover_type Crossover operator.
     * @param pop_size Population size.
     * @param pc Crossover probability.
     * @param pm_indiv Mutation probability per individual.
     * @param blx_alpha BLX-alpha expansion factor.
     * @param mutation_ratio Transfer ratio used in mutation.
     * @param ls_period Generations between local-search phases.
     * @param ls_max_evals Max evaluations per local-search call.
     * @param ls_ratio Transfer ratio used by local search.
     * @param pls_rand Selection probability in RAND variant.
     */
    MemeticAlgorithm(
        MemeticVariant variant,
        CrossoverType crossover_type,
        int pop_size,
        double pc,
        double pm_indiv,
        double blx_alpha,
        double mutation_ratio,
        int ls_period,
        int ls_max_evals,
        double ls_ratio,
        double pls_rand
    );

    /**
     * Optimize the given problem with a memetic strategy.
     *
     * @param problem Problem instance to solve.
     * @param maxevals Maximum number of fitness evaluations allowed.
     * @return Final solution, final fitness, and evaluations consumed.
     */
    ResultMH<double> optimize(Problem<double> &problem, int maxevals) override;

    /**
     * Get convergence values from the last execution.
     *
     * @return Best fitness recorded per generation.
     */
    const std::vector<double> &last_convergence() const { return m_last_convergence; }
};