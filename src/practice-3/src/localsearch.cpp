#include "localsearch.h"
#include "random.hpp"
#include <vector>
#include <algorithm>
#include <utility>

using namespace std;

ResultMH<double> LocalSearch::optimize(Problem<double> &problem, int maxevals) {
    tSolution<double> current_sol = problem.createSolution();
    return optimize(problem, maxevals, current_sol);
}

ResultMH<double> LocalSearch::optimize(Problem<double> &problem, int maxevals, const tSolution<double>& initial_sol) {
    size_t n = problem.getSolutionSize();
    auto bounds = problem.getSolutionDomainRange();
    double lo = bounds.first;
    double hi = bounds.second;

    tSolution<double> current_sol = initial_sol;
    tFitness current_fitness = problem.fitness(current_sol);
    int evals = 1;

    // Generate the complete neighborhood index (all pairs of assets).
    vector<pair<int, int>> neighborhood;
    for (size_t i = 0; i < n; ++i) {
        for (size_t j = 0; j < n; ++j) {
            if (i != j) {
                neighborhood.push_back({i, j});
            }
        }
    }

    bool improvement_found = true;

    // Iterate until no improvement is found in a full neighborhood scan or evaluation budget is hit.
    while (evals < maxevals && improvement_found) {
        improvement_found = false;

        // Randomize the order of neighborhood exploration to avoid directional bias.
        Random::shuffle(neighborhood); 

        for (const auto& pair : neighborhood) {
            if (evals >= maxevals) break; 

            int i = pair.first;
            int j = pair.second;

            // Skip transfers from empty assets.
            if (current_sol[i] == 0.0) continue;

            double transfer = current_sol[i] * m_ratio;
            double old_val_i = current_sol[i];
            double old_val_j = current_sol[j];
            double new_i = old_val_i - transfer;
            double new_j = old_val_j + transfer;

            // Enforce asset weight constraints.
            bool valid_i = (new_i < 1e-8) || (new_i >= lo && new_i <= hi);
            bool valid_j = (new_j < 1e-8) || (new_j >= lo && new_j <= hi);

            if (!valid_i || !valid_j) {
                continue; 
            }

            // Apply the move temporarily (in-place modification).
            current_sol[i] = (new_i < 1e-8) ? 0.0 : new_i; 
            current_sol[j] = (new_j < 1e-8) ? 0.0 : new_j;

            tFitness neighbor_fitness = problem.fitness(current_sol);
            evals++;

            // Accept the first neighbor that provides an improvement (First-Best strategy).
            if (neighbor_fitness > current_fitness) {
                current_fitness = neighbor_fitness;
                improvement_found = true;
                break; 
            } else {
                // Restore state if the move is not improving.
                current_sol[i] = old_val_i;
                current_sol[j] = old_val_j;
            }
        }
    }

    return ResultMH<double>(current_sol, current_fitness, evals);
}
