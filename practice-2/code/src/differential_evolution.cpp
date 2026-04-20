#include "differential_evolution.h"
#include <algorithm>

ResultMH<double> DifferentialEvolution::optimize(Problem<double> &problem, int maxevals) {
    // Clear previous execution tracking
    m_last_convergence.clear();

    const int dimension = static_cast<int>(problem.getSolutionSize()); // number of the assets in the portfolio optimization problem
    
    // Dynamic adjustment of population size based on maximum evaluations
    int pop_size = m_pop_size;
    pop_size = std::max(20, pop_size); // Ensure a minimum population size of 20 for sufficient diversity
    pop_size = std::min(pop_size, std::max(20, maxevals / 4)); // If maxevals is 100, limiting pop_size to 25 guarantees that the algorithm will at least be able to complete 4 full "rounds" (generations).

    unsigned int evals = 0;
    
    auto population = ea_initialize_population(problem, pop_size, evals);
    EAIndividual best_global = population[ea_best_index(population)];

    // Main evolutionary loop
    while (evals < static_cast<unsigned int>(maxevals)) {
        
        // Loop through each individual in the current generation
        for (int i = 0; i < pop_size; ++i) {
            // Stop immediately if we hit the evaluation limit
            if (evals >= static_cast<unsigned int>(maxevals)) {
                break;
            }

            // mutation 
            // Select 3 distinct random individuals (r0, r1, r2) that are also different from the current index (i)
            int r0 = Random::get<int>(0, pop_size - 1);
            int r1 = Random::get<int>(0, pop_size - 1);
            int r2 = Random::get<int>(0, pop_size - 1);

            while (r0 == i) r0 = Random::get<int>(0, pop_size - 1);
            while (r1 == i || r1 == r0) r1 = Random::get<int>(0, pop_size - 1);
            while (r2 == i || r2 == r0 || r2 == r1) r2 = Random::get<int>(0, pop_size - 1);

            // Create the donor vector: v = x_r0 + F * (x_r1 - x_r2)
            tSolution<double> donor(dimension, 0.0);
            for (int d = 0; d < dimension; ++d) {
                donor[d] = population[r0].solution[d]
                         + m_f * (population[r1].solution[d] - population[r2].solution[d]);
            }
            
            // Do not apply fix to the donor vector donor because the difference (X_r1 - X_r2) should stay as a pure geometric vector. If it is clipped here, the DE step size behavior is changed: large steps when the population is spread out, and small steps when it converges.

            // binomial crossover
            tSolution<double> trial = population[i].solution;
            // Guarantee that at least one dimension is taken from the donor
            int j_rand = Random::get<int>(0, dimension - 1);
            
            for (int d = 0; d < dimension; ++d) {
                // If random value <= CR or it's the guaranteed index, take from donor
                if (Random::get<double>(0.0, 1.0) <= m_cr || d == j_rand) {
                    trial[d] = donor[d];
                }
            }

            // Fix the final trial vector to ensure constraints are respected before evaluation
            problem.fix(trial);
            
            // Evaluate the new trial solution
            tFitness trial_fit = problem.fitness(trial);
            ++evals;

            // Deterministic selection
            // If the trial vector is better, it replaces the target vector in the population
            if (ea_better(trial_fit, population[i].fitness)) {
                population[i].solution = std::move(trial);
                population[i].fitness = trial_fit;
            }

            // Update the global best solution if the newly accepted individual is better
            int best_now = ea_best_index(population);
            if (ea_better(population[best_now].fitness, best_global.fitness)) {
                best_global = population[best_now];
            }
        }

        // Record the best fitness of the current generation for the convergence chart
        m_last_convergence.push_back(best_global.fitness);
    }

    return ResultMH<double>(best_global.solution, best_global.fitness, evals);
}