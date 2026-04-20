#pragma once

#include "mh.h"
#include "ea_common.h"
#include "genetic_algorithm.h"

#include <vector>

/**
 * @file memetic_lsch.h
 * @brief Memetic algorithm with Local Search Chains (MA-LSCh).
 *
 * This variant combines a generational GA cycle with periodic soft local
 * search on top individuals. Each LS chain keeps both:
 * - a persistent local-search state (pause/resume exploration), and
 * - an adaptive local-search budget per individual.
 */

/**
 * @brief Memetic algorithm with resumable local-search chains.
 *
 * Main behavior:
 * - Uses tournament selection, crossover, and transfer mutation.
 * - Applies strict elitism at generation replacement.
 * - Runs local search every `m_ls_period` generations.
 * - Applies local search to the best 10% of population.
 * - Preserves chain state and budget for elite individuals.
 * - Updates each individual's search budget based on LS success.
 */
class MemeticLSCh : public MH<double> {
private:
    /** Crossover operator used in reproduction. */
    CrossoverType m_crossover_type;
    /** Population size. */
    int m_pop_size;
    /** Probability of crossover for one parent pair. */
    double m_pc;
    /** Mutation probability per individual. */
    double m_pm_indiv;
    /** BLX-alpha parameter (used when BLX crossover is selected). */
    double m_blx_alpha;
    /** Transfer ratio used by transfer mutation. */
    double m_mutation_ratio;

    /** Number of generations between local-search phases. */
    int m_ls_period;
    /** Initial local-search budget for a new chain. */
    int m_base_budget;
    /** Maximum allowed local-search budget for one chain. */
    int m_max_budget;
    /** Growth factor applied to budget after successful local search. */
    double m_growth;

    /** Best fitness value recorded after each generation. */
    std::vector<double> m_last_convergence;

    /**
     * @brief Apply the selected crossover operator to one pair of parents.
     */
    void crossover_pair(
        const tSolution<double> &p1,
        const tSolution<double> &p2,
        tSolution<double> &c1,
        tSolution<double> &c2
    ) const;

public:
    /**
     * @brief Build a MA-LSCh optimizer.
     *
     * @param crossover_type Crossover operator.
     * @param pop_size Population size.
     * @param pc Crossover probability.
     * @param pm_indiv Mutation probability per individual.
     * @param blx_alpha BLX-alpha expansion factor.
     * @param mutation_ratio Transfer ratio used in mutation.
     * @param ls_period Generations between local-search applications.
     * @param base_budget Initial LS budget for each chain.
     * @param max_budget Maximum LS budget for each chain.
     * @param growth Budget multiplier applied after LS improvement.
     */
    MemeticLSCh(
        CrossoverType crossover_type,
        int pop_size,
        double pc,
        double pm_indiv,
        double blx_alpha,
        double mutation_ratio,
        int ls_period,
        int base_budget,
        int max_budget,
        double growth
    );

    /** Virtual destructor for safe polymorphic use. */
    virtual ~MemeticLSCh() = default;

    /**
     * @brief Optimize the problem with MA-LSCh.
     *
     * @param problem Problem instance to solve.
     * @param maxevals Maximum number of fitness evaluations.
     * @return Final solution, final fitness, and evaluations consumed.
     */
    ResultMH<double> optimize(Problem<double> &problem, int maxevals) override;

    /**
     * @brief Get convergence values from the last run.
     *
     * @return Best fitness value recorded per generation.
     */
    const std::vector<double> &last_convergence() const { return m_last_convergence; }
};