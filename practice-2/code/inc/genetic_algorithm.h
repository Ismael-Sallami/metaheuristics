#pragma once

#include "mh.h"
#include "ea_common.h"
#include <vector>

enum class GAScheme {
    AGG,
    AGE
};

enum class CrossoverType {
    ARITHMETIC,
    BLX
};

enum class MutationType {
    TRANSFER,
    GAUSSIAN
};

class GeneticAlgorithm : public MH<double> {
private:
    GAScheme m_scheme;
    CrossoverType m_crossover_type;

    int m_pop_size;
    double m_pc;
    double m_pm_indiv;
    double m_blx_alpha;
    double m_mutation_ratio;
    MutationType m_mutation_type;
    double m_gaussian_sigma;

    std::vector<double> m_last_convergence;

    void crossover_pair(
        const tSolution<double> &p1,
        const tSolution<double> &p2,
        tSolution<double> &c1,
        tSolution<double> &c2
    ) const;

    ResultMH<double> run_agg(Problem<double> &problem, int maxevals);
    ResultMH<double> run_age(Problem<double> &problem, int maxevals);

public:
    GeneticAlgorithm(
        GAScheme scheme,
        CrossoverType crossover_type,
        int pop_size,
        double pc,
        double pm_indiv,
        double blx_alpha,
        double mutation_ratio,
        MutationType mutation_type = MutationType::TRANSFER,
        double gaussian_sigma = 0.02
    );

    ResultMH<double> optimize(Problem<double> &problem, int maxevals) override;

    const std::vector<double> &last_convergence() const { return m_last_convergence; }
};
