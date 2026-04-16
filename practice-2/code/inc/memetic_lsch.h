#pragma once

#include "mh.h"
#include "ea_common.h"
#include "genetic_algorithm.h"

#include <vector>

class MemeticLSCh : public MH<double> {
private:
    CrossoverType m_crossover_type;
    int m_pop_size;
    double m_pc;
    double m_pm_indiv;
    double m_blx_alpha;
    double m_mutation_ratio;

    int m_ls_period;
    int m_base_budget;
    int m_max_budget;
    double m_growth;

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

    ResultMH<double> optimize(Problem<double> &problem, int maxevals) override;
};
