#include "localsearch.h"
#include "random.hpp"
#include <vector>
#include <algorithm>
#include <utility>

using namespace std;

ResultMH<double> LocalSearch::optimize(Problem<double> &problem, int maxevals) {
    size_t n = problem.getSolutionSize();
    auto bounds = problem.getSolutionDomainRange();
    double lo = bounds.first;
    double hi = bounds.second;

    tSolution<double> current_sol = problem.createSolution();
    tFitness current_fitness = problem.fitness(current_sol);
    int evals = 1;

    // The size of this neighborhood will be n*(n-1), since we generate the set of all pairs
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

        Random::shuffle(neighborhood); // Shuffle randomly

        // Explore the neighborhood
        for (const auto& pair : neighborhood) {
            if (evals >= maxevals) break; // Safety check for evaluations

            int i = pair.first;
            int j = pair.second;

            // If company i has 0 investment, 40% of 0 is 0 (useless move)
            if (current_sol[i] == 0.0) continue;

            // Calculate the amount to transfer: 40% of the initial amount of i
            double transfer = current_sol[i] * m_ratio;
            double new_i = current_sol[i] - transfer;
            double new_j = current_sol[j] + transfer;

            // Check limits (including that it can be 0)
            // Since we subtract and add the same amount, it is NOT necessary to check the sum to 1
            bool valid_i = (new_i < 1e-8) || (new_i >= lo && new_i <= hi);
            bool valid_j = (new_j < 1e-8) || (new_j >= lo && new_j <= hi);

            if (!valid_i || !valid_j) {
                continue; // This neighbor violates market rules, move to next pair
            }

            // Apply the change temporarily
            tSolution<double> neighbor = current_sol;
            // If the value is residually small (less than tolerance), force it to 0
            neighbor[i] = (new_i < 1e-8) ? 0.0 : new_i; 
            neighbor[j] = (new_j < 1e-8) ? 0.0 : new_j;

            // Evaluate the new portfolio
            tFitness neighbor_fitness = problem.fitness(neighbor);
            evals++;

            // "First Best" strategy
            // Accept the first neighbor that improves the current solution
            // Note: In our problem we seek to maximize fitness
            if (neighbor_fitness > current_fitness) {
                current_sol = neighbor;
                current_fitness = neighbor_fitness;
                improvement_found = true;
                
                // Update the best solution and go back to step 2 (shuffle)
                break; 
            }
        }
        // If the for loop completes fully and improvement_found remains false,
        // the algorithm stops by reaching a local optimum
    }

    return ResultMH<double>(current_sol, current_fitness, evals);
}