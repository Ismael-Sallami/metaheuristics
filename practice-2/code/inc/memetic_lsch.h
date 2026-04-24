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
 * search on one elite individual. Each LS chain keeps a persistent
 * local-search state (pause/resume exploration) and applies a fixed
 * intensity stretch budget in each LS call.
 */

/**
 * @brief Memetic algorithm with resumable local-search chains.
 *
 * Main behavior:
 * - Uses tournament selection, crossover, and transfer mutation.
 * - Applies strict elitism at generation replacement.
 * - Runs local search every `m_ls_period` generations.
 * - Applies local search to the best individual.
 * - Preserves chain state for cloned/elite individuals.
 * - Uses fixed LS intensity stretch `m_i_str`.
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
    /** Ratio used by soft local-search transfer moves. */
    double m_ls_ratio;
    /** Fixed local-search budget per chain step (Intensity Stretch). */
    int m_i_str;

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
     * @param ls_ratio Transfer ratio used by local search.
     * @param i_str Fixed LS intensity stretch.
     */
    MemeticLSCh(
        CrossoverType crossover_type,
        int pop_size,
        double pc,
        double pm_indiv,
        double blx_alpha,
        double mutation_ratio,
        int ls_period,
        double ls_ratio,
        int i_str
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