#include "ils.h"
#include "random.hpp"
#include <vector>
#include <numeric>
#include <algorithm>

using namespace std;

void ILS::mutate(tSolution<double>& sol) {
    size_t n = sol.size();
    size_t num_to_mutate = static_cast<size_t>(n * m_mutation_rate);
    // Ensure at least two elements are selected to allow a meaningful shuffle.
    if (num_to_mutate < 2) num_to_mutate = 2; 

    // Select random indices without repetition by shuffling an index pool.
    vector<int> indices(n);
    iota(indices.begin(), indices.end(), 0);
    Random::shuffle(indices);
    indices.resize(num_to_mutate);

    // Collect weight values from the selected indices.
    vector<double> values;
    for (int idx : indices) {
        values.push_back(sol[idx]);
    }

    // Shuffle the collected weights to perturb the solution.
    Random::shuffle(values);

    // Reassign the shuffled weights back to the selected indices.
    for (size_t i = 0; i < indices.size(); ++i) {
        sol[indices[i]] = values[i];
    }
}

ResultMH<double> ILS::optimize(Problem<double> &problem, int maxevals) {
    int total_evals = 0;
    LocalSearch bl(m_ratio);

    // Phase 1: Initial random start followed by local search optimization.
    tSolution<double> current_sol = problem.createSolution();
    ResultMH<double> result = bl.optimize(problem, m_max_evals_bl, current_sol);
    total_evals += result.evaluations;

    tSolution<double> best_sol = result.solution;
    tFitness best_fitness = result.fitness;

    // Phase 2: Iterative improvement via perturbation and local search.
    for (int i = 1; i < m_num_starts; ++i) {
        if (total_evals >= maxevals) break;

        // Apply perturbation to the current best solution to explore a new region.
        tSolution<double> mutated_sol = best_sol;
        mutate(mutated_sol);
        // It's reasonable to ask why a simple validity check isn't enough after mutation.
        // In constrained problems, rejecting invalid mutants or retrying can block exploration.
        // Use `problem.fix()` to map the infeasible solution back to a nearby feasible one.
        problem.fix(mutated_sol);

        // Refine the perturbed solution using local search.
        ResultMH<double> result_mut = bl.optimize(problem, m_max_evals_bl, mutated_sol);
        total_evals += result_mut.evaluations;

        // Update global best if the new trajectory finds a better peak.
        if (result_mut.fitness > best_fitness) {
            best_fitness = result_mut.fitness;
            best_sol = result_mut.solution;
        }
    }

    return ResultMH<double>(best_sol, best_fitness, total_evals);
}
