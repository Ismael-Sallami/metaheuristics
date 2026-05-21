#include "simulated_annealing.h"
#include "random.hpp"
#include <cmath>
#include <vector>
#include <algorithm>

using namespace std;

ResultMH<double> SimulatedAnnealing::optimize(Problem<double> &problem, int maxevals) {
    tSolution<double> current_sol = problem.createSolution();
    return optimize(problem, maxevals, current_sol);
}

ResultMH<double> SimulatedAnnealing::optimize(Problem<double> &problem, int maxevals, const tSolution<double>& initial_sol) {
    size_t n = problem.getSolutionSize();
    auto bounds = problem.getSolutionDomainRange();
    double lo = bounds.first;
    double hi = bounds.second;

    tSolution<double> current_sol = initial_sol;
    tFitness current_fitness = problem.fitness(current_sol);
    int evals = 1;

    tSolution<double> best_sol = current_sol;
    tFitness best_fitness = current_fitness;

    // Calculate initial temperature T0 based on initial fitness and provided mu/phi parameters.
    // T0 = (mu * |f(S0)|) / -log(phi)
    // We use absolute value to ensure positive temperature as fitness is usually negative.
    double T0 = (m_mu * abs(current_fitness)) / -log(m_phi);

    // Guard required by the practice guide
    // If T0 <= Tf the cooling scheme would be degenerate; we return the initial solution directly.
    if (T0 <= m_tf) {
        return ResultMH<double>(best_sol, best_fitness, evals);
    }

    double T = T0;

    // Determine the number of cooling steps M.
    // If max_vecinos is L, then M = MaxEvals / L.
    double M = static_cast<double>(maxevals) / m_max_vecinos;
    if (M < 1.0) M = 1.0;
    
    // Calculate beta for the Modified Cauchy cooling scheme.
    double beta = (T0 - m_tf) / (M * T0 * m_tf);

    // Build the neighborhood by indexing all possible pairs of assets for transfer moves.
    vector<pair<int, int>> neighborhood;
    for (size_t i = 0; i < n; ++i) {
        for (size_t j = 0; j < n; ++j) {
            if (i != j) neighborhood.push_back({i, j});
        }
    }

    int n_exitos = 1; 

    // Main annealing loop (cooling steps).
    while (evals < maxevals && T > m_tf && n_exitos > 0) {
        n_exitos = 0;
        int n_vecinos = 0;

        // Internal loop: exploration at constant temperature level T.
        while (n_vecinos < m_max_vecinos && n_exitos < m_max_exitos && evals < maxevals) {
            n_vecinos++; 

            // Pick a random move from the pre-generated neighborhood.
            auto& move = neighborhood[Random::get<size_t>(0, neighborhood.size() - 1)];
            int i = move.first;
            int j = move.second;

            // Skip moves if the source asset has no weight.
            if (current_sol[i] == 0.0) {
                continue;
            }

            double transfer = current_sol[i] * m_ratio;
            double old_val_i = current_sol[i];
            double old_val_j = current_sol[j];
            double new_i = old_val_i - transfer;
            double new_j = old_val_j + transfer;

            // Validate both weights against market constraints.
            bool valid_i = (new_i < 1e-8) || (new_i >= lo && new_i <= hi);
            bool valid_j = (new_j < 1e-8) || (new_j >= lo && new_j <= hi);

            if (!valid_i || !valid_j) continue;

            // Modify the current solution in-place to save memory.
            current_sol[i] = (new_i < 1e-8) ? 0.0 : new_i;
            current_sol[j] = (new_j < 1e-8) ? 0.0 : new_j;

            tFitness neighbor_fitness = problem.fitness(current_sol);
            evals++;

            // Difference in fitness (current - neighbor) for maximization.
            double diff = current_fitness - neighbor_fitness; 

            // Metropolis acceptance criterion.
            if (diff < 0 || Random::get<double>(0.0, 1.0) <= exp(-diff / T)) {
                current_fitness = neighbor_fitness;
                n_exitos++;

                if (current_fitness > best_fitness) {
                    best_sol = current_sol;
                    best_fitness = current_fitness;
                }
            } else {
                // Revert changes if the move is rejected.
                current_sol[i] = old_val_i;
                current_sol[j] = old_val_j;
            }
        }

        // Apply Modified Cauchy cooling scheme: T_next = T / (1 + beta * T).
        T = T / (1 + beta * T);
    }

    return ResultMH<double>(best_sol, best_fitness, evals);
}
