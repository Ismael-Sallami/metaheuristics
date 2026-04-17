#pragma once

#include <vector>
#include <algorithm>
#include <cmath>
#include "problem.h"
#include "random.hpp"

struct EAIndividual {
    tSolution<double> solution;
    tFitness fitness;
};

inline bool ea_better(tFitness a, tFitness b) {
    // Pr2 follows maximization of fitness.
    return a > b;
}

inline int ea_best_index(const std::vector<EAIndividual> &pop) {
    int best = 0;
    for (int i = 1; i < static_cast<int>(pop.size()); ++i) {
        if (ea_better(pop[i].fitness, pop[best].fitness)) {
            best = i;
        }
    }
    return best;
}

inline int ea_worst_index(const std::vector<EAIndividual> &pop) {
    int worst = 0;
    for (int i = 1; i < static_cast<int>(pop.size()); ++i) {
        if (ea_better(pop[worst].fitness, pop[i].fitness)) {
            worst = i;
        }
    }
    return worst;
}

inline void ea_evaluate_individual(Problem<double> &problem, EAIndividual &ind, unsigned int &evals) {
    problem.fix(ind.solution);
    ind.fitness = problem.fitness(ind.solution);
    ++evals;
}

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
        const double interval = cmax - cmin;
        const double low = cmin - alpha * interval;
        const double high = cmax + alpha * interval;

        c1[i] = Random::get<double>(low, high);
        c2[i] = Random::get<double>(low, high);
    }
}

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
    if (n < 2) { // we need at least 2 variables to transfer between them
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

    problem.fix(sol); // ensure the solution is valid after mutation
}


inline std::vector<int> ea_best_k_indices(const std::vector<EAIndividual> &pop, int k) {
    std::vector<int> idx(pop.size());
    for (int i = 0; i < static_cast<int>(pop.size()); ++i) {
        idx[i] = i; // initialize with indices, in this way it is more efficient to sort in terms of memory and time than sorting the whole population
    }
    
    std::sort(idx.begin(), idx.end(), [&](int a, int b) {
        return ea_better(pop[a].fitness, pop[b].fitness);
    });

    if (k < static_cast<int>(idx.size())) {
        idx.resize(k);
    }

    return idx;
}