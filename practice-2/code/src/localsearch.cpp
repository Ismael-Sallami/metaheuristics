#include "localsearch.h"
#include "random.hpp"
#include <vector>
#include <algorithm>
#include <utility>

using namespace std;

ResultMH<double> LocalSearch::optimize(Problem<double> &problem, int maxevals) {
    size_t n = problem.getSolutionSize();
    auto limites = problem.getSolutionDomainRange();
    double lo = limites.first;
    double hi = limites.second;

    tSolution<double> current_sol = problem.createSolution();
    tFitness current_fitness = problem.fitness(current_sol);
    int evals = 1;

    // The size of this neighborhood will be n*(n-1), since we generate the set of all pairs
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

        Random::shuffle(entorno); // Shuffle randomly

        // Explore the neighborhood
        for (const auto& par : entorno) {
            if (evals >= maxevals) break; // Safety check for evaluations

            int i = par.first;
            int j = par.second;

            // If company i has 0 investment, 40% of 0 is 0 (useless move)
            if (current_sol[i] == 0.0) continue;

            // Calculate the amount to transfer: 40% of the initial amount of i
            double trasvase = current_sol[i] * m_ratio;
            double nuevo_i = current_sol[i] - trasvase;
            double nuevo_j = current_sol[j] + trasvase;

            // Check limits (including that it can be 0)
            // Since we subtract and add the same amount, it is NOT necessary to check the sum to 1
            bool valido_i = (nuevo_i < 1e-8) || (nuevo_i >= lo && nuevo_i <= hi);
            bool valido_j = (nuevo_j < 1e-8) || (nuevo_j >= lo && nuevo_j <= hi);

            if (!valido_i || !valido_j) {
                continue; // This neighbor violates market rules, move to next pair
            }

            // Apply the change temporarily
            tSolution<double> neighbor = current_sol;
            // If the value is residually small (less than tolerance), force it to 0
            neighbor[i] = (nuevo_i < 1e-8) ? 0.0 : nuevo_i; 
            neighbor[j] = (nuevo_j < 1e-8) ? 0.0 : nuevo_j;

            // Evaluate the new portfolio
            tFitness neighbor_fitness = problem.fitness(neighbor);
            evals++;

            // "First Best" strategy for minimization.
            if (neighbor_fitness < current_fitness) {
                current_sol = neighbor;
                current_fitness = neighbor_fitness;
                mejora_encontrada = true;
                
                // Update the best solution and go back to step 2 (shuffle)
                break; 
            }
        }
        // If the for loop completes fully and mejora_encontrada remains false,
        // the algorithm stops by reaching a local optimum
    }

    return ResultMH<double>(current_sol, current_fitness, evals);
}