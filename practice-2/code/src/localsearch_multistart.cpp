#include "localsearch_multistart.h"
#include "random.hpp"
#include <vector>
#include <algorithm>
#include <utility>

using namespace std;

ResultMH<double> LocalSearchMultiStart::optimize(Problem<double> &problem, int maxevals) {
    size_t n = problem.getSolutionSize();
    auto limites = problem.getSolutionDomainRange();
    double lo = limites.first;
    double hi = limites.second;

    int evals = 0;
    
    // Keep track of the best solution found across all starts
    tSolution<double> global_best_sol;
    tFitness global_best_fitness = 1e9;

    // Generate the neighborhood (pairs of companies)
    vector<pair<int, int>> entorno;
    for (size_t i = 0; i < n; ++i) {
        for (size_t j = 0; j < n; ++j) {
            if (i != j) entorno.push_back({i, j});
        }
    }

    // Multi-start loop: repeat while we have evaluations left
    while (evals < maxevals) {
        // Create a random initial solution
        tSolution<double> current_sol = problem.createSolution();
        tFitness current_fitness = problem.fitness(current_sol);
        evals++;

        // Update global best if initial solution is lucky
        if (current_fitness < global_best_fitness) {
            global_best_fitness = current_fitness;
            global_best_sol = current_sol;
        }

        bool improvement_found = true;

        // Local search: keep improving until no better neighbor is found
        while (evals < maxevals && improvement_found) {
            improvement_found = false;
            Random::shuffle(entorno);

            for (const auto& par : entorno) {
                if (evals >= maxevals) break;

                int i = par.first;
                int j = par.second;

                if (current_sol[i] == 0.0) continue;

                // Transfer 40% from i to j
                double trasvase = current_sol[i] * m_ratio;
                double nuevo_i = current_sol[i] - trasvase;
                double nuevo_j = current_sol[j] + trasvase;

                // Check if new values are valid (within bounds or near zero)
                bool valido_i = (nuevo_i < 1e-8) || (nuevo_i >= lo && nuevo_i <= hi);
                bool valido_j = (nuevo_j < 1e-8) || (nuevo_j >= lo && nuevo_j <= hi);

                if (!valido_i || !valido_j) continue;

                tSolution<double> neighbor = current_sol;
                neighbor[i] = (nuevo_i < 1e-8) ? 0.0 : nuevo_i; 
                neighbor[j] = (nuevo_j < 1e-8) ? 0.0 : nuevo_j;

                tFitness neighbor_fitness = problem.fitness(neighbor);
                evals++;

                // Accept first improvement found for maximization.
                if (neighbor_fitness > current_fitness) {
                    current_sol = neighbor;
                    current_fitness = neighbor_fitness;
                    improvement_found = true;
                    
                    // If this local optimum beats the global best, update it
                    if (current_fitness > global_best_fitness) {
                        global_best_fitness = current_fitness;
                        global_best_sol = current_sol;
                    }
                    break; // Restart neighborhood search from new solution
                }
            }
        }
        // Local search ended because no improvement was found (stuck at local optimum)
        // Main loop will restart with a new random solution
    }

    return ResultMH<double>(global_best_sol, global_best_fitness, evals);
}