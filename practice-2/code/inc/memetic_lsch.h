#pragma once

#include "mh.h"
#include "ea_common.h"
#include "genetic_algorithm.h"

#include <vector>

/**
 * @class MemeticLSCh
 * @brief Memetic Algorithm with Local Search Chains (MA-LSCh).
 * Combines a Generational Genetic Algorithm with dynamic Local Search.
 * Chains maintain computational budget across generations for successful solutions.
 */
class MemeticLSCh : public MH<double> {
private:
    // Evolutionary Algorithm Parameters
    CrossoverType m_crossover_type;
    int m_pop_size;
    double m_pc;
    double m_pm_indiv;
    double m_blx_alpha;
    double m_mutation_ratio;

    // Local Search Parameters
    int m_ls_period;        // Generations between LS applications
    int m_base_budget;      // Initial evaluations for a new chain
    int m_max_budget;       // Max evaluations for an active/successful chain
    double m_growth;        // Multiplier for budget on success

    std::vector<double> m_last_convergence;

    /**
     * @brief Applies the selected crossover operator.
     */
    void crossover_pair(
        const tSolution<double> &p1,
        const tSolution<double> &p2,
        tSolution<double> &c1,
        tSolution<double> &c2
    ) const;

public:
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

    // Virtual destructor to prevent memory leaks in derived polymorphic classes
    virtual ~MemeticLSCh() = default;

    ResultMH<double> optimize(Problem<double> &problem, int maxevals) override;

    const std::vector<double> &last_convergence() const { return m_last_convergence; }
};