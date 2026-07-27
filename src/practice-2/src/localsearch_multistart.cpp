#include "localsearch_multistart.h"
#include "random.hpp"
#include <vector>
#include <algorithm>
#include <utility>

using namespace std;

ResultMH<double> LocalSearchMultiStart::optimize(Problem<double> &problem, int maxevals) {
    size_t n = problem.getSolutionSize();
    auto bounds = problem.getSolutionDomainRange();
    double lo = bounds.first;
    double hi = bounds.second;

    int evals = 0;
    
    // Keep track of the best solution found across all starts
    tSolution<double> global_best_sol;
    tFitness global_best_fitness = -1e9; // Initialize very low

    // Generate the neighborhood (pairs of companies)
    vector<pair<int, int>> neighborhood;
    for (size_t i = 0; i < n; ++i) {
        for (size_t j = 0; j < n; ++j) {
            if (i != j) neighborhood.push_back({i, j});
        }
    }

    // Multi-start loop: repeat while we have evaluations left
    while (evals < maxevals) {
        // Create a random initial solution
        tSolution<double> current_sol = problem.createSolution();
        tFitness current_fitness = problem.fitness(current_sol);
        evals++;

        // Update global best if initial solution is lucky
        if (current_fitness > global_best_fitness) {
            global_best_fitness = current_fitness;
            global_best_sol = current_sol;
        }

        bool improvement_found = true;

        // Local search: keep improving until no better neighbor is found
        while (evals < maxevals && improvement_found) {
            improvement_found = false;
            Random::shuffle(neighborhood);

            for (const auto& pair : neighborhood) {
                if (evals >= maxevals) break;

                int i = pair.first;
                int j = pair.second;

                if (current_sol[i] == 0.0) continue;

                // Transfer 40% from i to j
                double transfer = current_sol[i] * m_ratio;
                const double old_val_i = current_sol[i];
                const double old_val_j = current_sol[j];
                double new_i = old_val_i - transfer;
                double new_j = old_val_j + transfer;

                // Check if new values are valid (within bounds or near zero)
                bool valid_i = (new_i < 1e-8) || (new_i >= lo && new_i <= hi);
                bool valid_j = (new_j < 1e-8) || (new_j >= lo && new_j <= hi);

                if (!valid_i || !valid_j) continue;

                // Apply the change in-place to avoid expensive vector copies
                current_sol[i] = (new_i < 1e-8) ? 0.0 : new_i; 
                current_sol[j] = (new_j < 1e-8) ? 0.0 : new_j;

                tFitness neighbor_fitness = problem.fitness(current_sol);
                evals++;

                // Accept first improvement found
                if (neighbor_fitness > current_fitness) {
                    current_fitness = neighbor_fitness;
                    improvement_found = true;
                    
                    // If this local optimum beats the global best, update it
                    if (current_fitness > global_best_fitness) {
                        global_best_fitness = current_fitness;
                        global_best_sol = current_sol;
                    }
                    break; // Restart neighborhood search from new solution
                } else {
                    // Revert to original state if no improvement
                    current_sol[i] = old_val_i;
                    current_sol[j] = old_val_j;
                }
            }
        }
        // Local search ended because no improvement was found (stuck at local optimum)
        // Main loop will restart with a new random solution
    }

    return ResultMH<double>(global_best_sol, global_best_fitness, evals);
}