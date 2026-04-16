#include "differential_evolution.h"

#include <algorithm>

ResultMH<double> DifferentialEvolution::optimize(Problem<double> &problem, int maxevals) {
    m_last_convergence.clear();

    const int dimension = static_cast<int>(problem.getSolutionSize());
    int pop_size = m_pop_size;
    pop_size = std::max(20, pop_size);
    pop_size = std::min(pop_size, std::max(20, maxevals / 4));

    unsigned int evals = 0;
    auto population = ea_initialize_population(problem, pop_size, evals);

    EAIndividual best_global = population[ea_best_index(population)];

    while (evals < static_cast<unsigned int>(maxevals)) {
        for (int i = 0; i < pop_size; ++i) {
            if (evals >= static_cast<unsigned int>(maxevals)) {
                break;
            }

            int r0 = Random::get<int>(0, pop_size - 1);
            int r1 = Random::get<int>(0, pop_size - 1);
            int r2 = Random::get<int>(0, pop_size - 1);

            while (r0 == i) r0 = Random::get<int>(0, pop_size - 1);
            while (r1 == i || r1 == r0) r1 = Random::get<int>(0, pop_size - 1);
            while (r2 == i || r2 == r0 || r2 == r1) r2 = Random::get<int>(0, pop_size - 1);

            tSolution<double> donor(dimension, 0.0);
            for (int d = 0; d < dimension; ++d) {
                donor[d] = population[r0].solution[d]
                         + m_f * (population[r1].solution[d] - population[r2].solution[d]);
            }
            problem.fix(donor);

            tSolution<double> trial = population[i].solution;
            int j_rand = Random::get<int>(0, dimension - 1);
            for (int d = 0; d < dimension; ++d) {
                if (Random::get<double>(0.0, 1.0) <= m_cr || d == j_rand) {
                    trial[d] = donor[d];
                }
            }

            problem.fix(trial);
            tFitness trial_fit = problem.fitness(trial);
            ++evals;

            if (ea_better(trial_fit, population[i].fitness)) {
                population[i].solution = std::move(trial);
                population[i].fitness = trial_fit;
            }

            int best_now = ea_best_index(population);
            if (ea_better(population[best_now].fitness, best_global.fitness)) {
                best_global = population[best_now];
            }
        }

        m_last_convergence.push_back(best_global.fitness);
    }

    return ResultMH<double>(best_global.solution, best_global.fitness, evals);
}
