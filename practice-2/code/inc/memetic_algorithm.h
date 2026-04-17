#pragma once

#include "mh.h"
#include "ea_common.h"
#include "genetic_algorithm.h"

#include <vector>

enum class MemeticVariant {
    ALL,
    RAND,
    BEST
};

class MemeticAlgorithm : public MH<double> {
private:
    MemeticVariant m_variant;
    CrossoverType m_crossover_type;

    int m_pop_size;
    double m_pc;
    double m_pm_indiv;
    double m_blx_alpha;
    double m_mutation_ratio;

    int m_ls_period;
    int m_ls_max_evals;
    double m_ls_ratio;
    double m_pls_rand;

    std::vector<double> m_last_convergence;

    void crossover_pair(
        const tSolution<double> &p1,
        const tSolution<double> &p2,
        tSolution<double> &c1,
        tSolution<double> &c2
    ) const;

public:
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

    ResultMH<double> optimize(Problem<double> &problem, int maxevals) override;

    const std::vector<double> &last_convergence() const { return m_last_convergence; }
};