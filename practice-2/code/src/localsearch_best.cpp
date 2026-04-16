#include "localsearch_best.h"
#include "random.hpp"
#include <vector>
#include <algorithm>
#include <utility>
#include <cmath>

using namespace std;

ResultMH<double> LocalSearchBest::optimize(Problem<double> &problem, int maxevals) {
    size_t n = problem.getSolutionSize();
    auto limites = problem.getSolutionDomainRange();
    double lo = limites.first;
    double hi = limites.second;

    tSolution<double> current_sol = problem.createSolution();
    tFitness current_fitness = problem.fitness(current_sol);
    int evals = 1;

    // Generate all possible pairs for the neighborhood
    vector<pair<int, int>> entorno;
    for (size_t i = 0; i < n; ++i) {
        for (size_t j = 0; j < n; ++j) {
            if (i != j) {
                entorno.push_back({i, j});
            }
        }
    }

    bool mejora_encontrada = true;

    while (evals < maxevals && mejora_encontrada) {
        mejora_encontrada = false;

        // Shuffle the neighborhood to avoid bias when hitting evaluation limits
        Random::shuffle(entorno);

        tSolution<double> mejor_vecino_sol = current_sol;
        tFitness mejor_vecino_fitness = INFINITY;

        // Explore the entire neighborhood (or until we run out of evaluations)
        for (const auto& par : entorno) {
            if (evals >= maxevals) break; 

            int i = par.first;
            int j = par.second;

            if (current_sol[i] == 0.0) continue;

            double trasvase = current_sol[i] * m_ratio;
            double nuevo_i = current_sol[i] - trasvase;
            double nuevo_j = current_sol[j] + trasvase;

            bool valido_i = (nuevo_i < 1e-8) || (nuevo_i >= lo && nuevo_i <= hi);
            bool valido_j = (nuevo_j < 1e-8) || (nuevo_j >= lo && nuevo_j <= hi);

            if (!valido_i || !valido_j) continue;

            tSolution<double> neighbor = current_sol;
            neighbor[i] = (nuevo_i < 1e-8) ? 0.0 : nuevo_i; 
            neighbor[j] = (nuevo_j < 1e-8) ? 0.0 : nuevo_j;

            tFitness neighbor_fitness = problem.fitness(neighbor);
            evals++;

            // Keep the best neighbor found in this iteration for minimization.
            if (neighbor_fitness < mejor_vecino_fitness) {
                mejor_vecino_fitness = neighbor_fitness;
                mejor_vecino_sol = neighbor;
            }
        }

        // Check if the best neighbor is better than the current solution.
        if (mejor_vecino_fitness < current_fitness) {
            current_sol = mejor_vecino_sol;
            current_fitness = mejor_vecino_fitness;
            mejora_encontrada = true;
        }
    }

    return ResultMH<double>(current_sol, current_fitness, evals);
}
