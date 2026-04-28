#include "localsearch_best.h"
#include "random.hpp"
#include <vector>
#include <algorithm>
#include <utility>
#include <cmath>

using namespace std;

ResultMH<double> LocalSearchBest::optimize(Problem<double> &problem, int maxevals) {
    size_t n = problem.getSolutionSize();
    auto bounds = problem.getSolutionDomainRange();
    double lo = bounds.first;
    double hi = bounds.second;

    tSolution<double> current_sol = problem.createSolution();
    tFitness current_fitness = problem.fitness(current_sol);
    int evals = 1;

    // Generate all possible pairs for the neighborhood
    vector<pair<int, int>> neighborhood;
    for (size_t i = 0; i < n; ++i) {
        for (size_t j = 0; j < n; ++j) {
            if (i != j) {
                neighborhood.push_back({i, j});
            }
        }
    }

    bool improvement_found = true;

    while (evals < maxevals && improvement_found) {
        improvement_found = false;

        // Shuffle the neighborhood to avoid bias when hitting evaluation limits
        Random::shuffle(neighborhood);

        tSolution<double> best_neighbor_solution = current_sol;
        tFitness best_neighbor_fitness = -INFINITY; // Initialize with very low value

        // Explore the entire neighborhood (or until we run out of evaluations)
        for (const auto& pair : neighborhood) {
            if (evals >= maxevals) break; 

            int i = pair.first;
            int j = pair.second;

            if (current_sol[i] == 0.0) continue;

            double transfer = current_sol[i] * m_ratio;
            double new_i = current_sol[i] - transfer;
            double new_j = current_sol[j] + transfer;

            bool valid_i = (new_i < 1e-8) || (new_i >= lo && new_i <= hi);
            bool valid_j = (new_j < 1e-8) || (new_j >= lo && new_j <= hi);

            if (!valid_i || !valid_j) continue;

            tSolution<double> neighbor = current_sol;
            neighbor[i] = (new_i < 1e-8) ? 0.0 : new_i; 
            neighbor[j] = (new_j < 1e-8) ? 0.0 : new_j;

            tFitness neighbor_fitness = problem.fitness(neighbor);
            evals++;

            // Keep the best neighbor found in this iteration
            if (neighbor_fitness > best_neighbor_fitness) {
                best_neighbor_fitness = neighbor_fitness;
                best_neighbor_solution = neighbor;
            }
        }

        // Check if the best neighbor is better than the current solution
        if (best_neighbor_fitness > current_fitness) {
            current_sol = best_neighbor_solution;
            current_fitness = best_neighbor_fitness;
            improvement_found = true;
        }
    }

    return ResultMH<double>(current_sol, current_fitness, evals);
}
