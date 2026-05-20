#include "ils_es_chc.h"
#include "random.hpp"
#include <cmath>
#include <algorithm>

using namespace std;

void ILSES_CHC::mutate(tSolution<double>& sol, double rate) {
    size_t n = sol.size();
    int num_mut = max(1, (int)(rate * n));
    
    // Select num_mut assets at random
    vector<size_t> indices(n);
    iota(indices.begin(), indices.end(), 0);
    random_shuffle(indices.begin(), indices.end());
    
    // Shuffle the weights of the num_mut selected assets
    vector<double> weights;
    for (int i = 0; i < num_mut; i++) {
        weights.push_back(sol[indices[i]]);
    }
    random_shuffle(weights.begin(), weights.end());
    for (int i = 0; i < num_mut; i++) {
        sol[indices[i]] = weights[i];
    }
}

void ILSES_CHC::macromutate(tSolution<double>& sol, double rate) {
    // Same as mutate but at higher rate (0.4-0.5)
    // Effectively provides significant solution perturbation
    mutate(sol, rate);
}

tSolution<double> ILSES_CHC::constructGreedyRandomized(Problem<double>& problem) {
    // Simple randomized greedy construction for portfolios
    // Start with empty solution and add assets per randomized heuristic
    size_t n = problem.getSolutionSize();
    tSolution<double> sol(n, 0.0);
    
    // Generate vector of random "attractiveness" values (simulating heuristic info)
    vector<pair<double, size_t>> attractiveness;
    for (size_t i = 0; i < n; i++) {
        attractiveness.push_back({Random::get<double>(0.0, 1.0), i});
    }
    sort(attractiveness.rbegin(), attractiveness.rend());
    
    // Assign weights in randomized fashion
    double suma = 0.0;
    for (size_t i = 0; i < n; i++) {
        sol[attractiveness[i].second] = Random::get<double>(0.005, 0.08);
        suma += sol[attractiveness[i].second];
    }
    
    // Normalize
    for (size_t i = 0; i < n; i++) {
        sol[i] /= suma;
    }
    
    problem.fix(sol);
    return sol;
}

ResultMH<double> ILSES_CHC::optimize(Problem<double> &problem, int maxevals) {
    size_t n = problem.getSolutionSize();
    auto bounds = problem.getSolutionDomainRange();
    double lo = bounds.first;
    double hi = bounds.second;

    tSolution<double> current_sol = problem.createSolution();
    tFitness current_fitness = problem.fitness(current_sol);
    int evals = 1;

    tSolution<double> best_sol = current_sol;
    tFitness best_fitness = current_fitness;

    // Create SA engine
    SimulatedAnnealing es(m_mu, m_phi, m_tf, m_ratio, m_max_vecinos, m_max_exitos);

    int stagnation_counter = 0; // Counter for iterations without improvement

    // First restart: pure SA
    ResultMH<double> result_es = es.optimize(problem, maxevals - evals, current_sol);
    best_sol = result_es.solution;
    best_fitness = result_es.fitness;
    evals += result_es.evaluations;

    // Subsequent restarts with stagnation monitoring
    for (int start = 2; start <= m_num_starts && evals < maxevals; start++) {
        tSolution<double> mutated_sol;

        // --- CHC MECHANISM: STAGNATION DETECTION AND DIVERSIFICATION ---
        if (stagnation_counter >= m_limit_stagnation) {
            // Apply DIVERSIFICATION: macro-mutation (Option A: CHC)
            mutated_sol = best_sol;
            macromutate(mutated_sol, m_mutation_rate_macro);
            problem.fix(mutated_sol);

            // Alternative (commented): Option B (GADEGD)
            // mutated_sol = constructGreedyRandomized(problem);

            stagnation_counter = 0; // Reset counter
        } else {
            // Standard mutation (classic ILS)
            mutated_sol = best_sol;
            mutate(mutated_sol, m_mutation_rate_normal);
            problem.fix(mutated_sol);
        }

        // Apply SA to mutated solution
        int remaining_evals = maxevals - evals;
        ResultMH<double> result = es.optimize(problem, remaining_evals, mutated_sol);
        evals += result.evaluations;

        // --- ACCEPTANCE CRITERION: BEST-OF (classic ILS) ---
        if (result.fitness > best_fitness) {
            best_sol = result.solution;
            best_fitness = result.fitness;
            stagnation_counter = 0; // Reset on improvement
        } else {
            stagnation_counter++; // Increment if no improvement
        }
    }

    return ResultMH<double>(best_sol, best_fitness, evals);
}
