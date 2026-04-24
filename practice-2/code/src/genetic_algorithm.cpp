#include "genetic_algorithm.h"
#include "gaussian_mutation.h"

#include <algorithm>

GeneticAlgorithm::GeneticAlgorithm(
    GAScheme scheme,
    CrossoverType crossover_type,
    int pop_size,
    double pc,
    double pm_indiv,
    double blx_alpha,
    double mutation_ratio,
    MutationType mutation_type,
    double gaussian_sigma
) : m_scheme(scheme),
    m_crossover_type(crossover_type),
    m_pop_size(pop_size),
    m_pc(pc),
    m_pm_indiv(pm_indiv),
    m_blx_alpha(blx_alpha),
    m_mutation_ratio(mutation_ratio),
    m_mutation_type(mutation_type),
    m_gaussian_sigma(gaussian_sigma) {}

void GeneticAlgorithm::crossover_pair(
    const tSolution<double> &p1,
    const tSolution<double> &p2,
    tSolution<double> &c1,
    tSolution<double> &c2
) const {
    if (m_crossover_type == CrossoverType::ARITHMETIC) {
        ea_arithmetic_crossover(p1, p2, c1, c2);
    } else {
        ea_blx_crossover(p1, p2, m_blx_alpha, c1, c2);
    }
}

ResultMH<double> GeneticAlgorithm::run_agg(Problem<double> &problem, int maxevals) {
    m_last_convergence.clear();

    unsigned int evals = 0;
    auto population = ea_initialize_population(problem, m_pop_size, evals);

    int best_idx = ea_best_index(population);
    EAIndividual best_global = population[best_idx];

    while (evals + static_cast<unsigned int>(m_pop_size) <= static_cast<unsigned int>(maxevals)) {
        std::vector<EAIndividual> parents(m_pop_size);

        // Selection phase (k=3 tournament)
        for (int i = 0; i < m_pop_size; ++i) {
            parents[i] = population[ea_tournament_k3(population)];
        }

        std::vector<EAIndividual> children(m_pop_size);

        // Crossover (expected number of crosses)
        const int num_pairs = m_pop_size / 2;
        const int num_cross_pairs = static_cast<int>(m_pc * num_pairs);

        for (int pair = 0; pair < num_pairs; ++pair) {
            const int i = 2 * pair;
            const int j = (i + 1) % m_pop_size;

            const auto &p1 = parents[i].solution;
            const auto &p2 = parents[j].solution;

            if (pair < num_cross_pairs) {
                crossover_pair(p1, p2, children[i].solution, children[j].solution);
            } else {
                children[i].solution = p1;
                children[j].solution = p2;
            }
        }

        // If pop size is odd, keep last individual copied
        if (m_pop_size % 2 != 0) {
            children[m_pop_size - 1].solution = parents[m_pop_size - 1].solution;
        }

        // Mutation (expected mutated individuals) 
        const int num_mut_individuals = static_cast<int>(m_pm_indiv * m_pop_size);

        for (int i = 0; i < num_mut_individuals; ++i) {
            int random_idx = Random::get<int>(0, m_pop_size - 1);

            if (m_mutation_type == MutationType::GAUSSIAN) {
                gaussian_mutate_individual(children[random_idx].solution, problem, 1.0, m_gaussian_sigma);
            } else {
                ea_mutate_transfer(children[random_idx].solution, problem, 1.0, m_mutation_ratio);
            }

        }

        for (int i = 0; i < m_pop_size; ++i) {
            ea_evaluate_individual(problem, children[i], evals);
        }

        // Elitism: preserve best from previous population
        const int best_prev_idx = ea_best_index(population);
        const int best_child_idx = ea_best_index(children);

        if (ea_better(population[best_prev_idx].fitness, children[best_child_idx].fitness)) {
            const int worst_child_idx = ea_worst_index(children);
            children[worst_child_idx] = population[best_prev_idx];
        }

        population = std::move(children);

        const int best_now_idx = ea_best_index(population);
        if (ea_better(population[best_now_idx].fitness, best_global.fitness)) {
            best_global = population[best_now_idx];
        }

        m_last_convergence.push_back(best_global.fitness);
    }

    return ResultMH<double>(best_global.solution, best_global.fitness, evals);
}

ResultMH<double> GeneticAlgorithm::run_age(Problem<double> &problem, int maxevals) {
    m_last_convergence.clear();

    unsigned int evals = 0;
    auto population = ea_initialize_population(problem, m_pop_size, evals);

    int best_idx = ea_best_index(population);
    EAIndividual best_global = population[best_idx];

    while (evals + 2 <= static_cast<unsigned int>(maxevals)) { // Only 2 individuals are born per iteration
        int p1_idx = ea_tournament_k3(population);
        int p2_idx = ea_tournament_k3(population);

        const auto &p1 = population[p1_idx].solution;
        const auto &p2 = population[p2_idx].solution;

        EAIndividual c1, c2;

        if (Random::get<double>(0.0, 1.0) <= m_pc) {
            crossover_pair(p1, p2, c1.solution, c2.solution);
        } else {
            c1.solution = p1;
            c2.solution = p2;
        }

        if (m_mutation_type == MutationType::GAUSSIAN) {
            gaussian_mutate_individual(c1.solution, problem, m_pm_indiv, m_gaussian_sigma);
            gaussian_mutate_individual(c2.solution, problem, m_pm_indiv, m_gaussian_sigma);
        } else {
            ea_mutate_transfer(c1.solution, problem, m_pm_indiv, m_mutation_ratio);
            ea_mutate_transfer(c2.solution, problem, m_pm_indiv, m_mutation_ratio);
        }

        ea_evaluate_individual(problem, c1, evals);
        ea_evaluate_individual(problem, c2, evals);

        // Find the current worst individual and compare it with the first child (c1)
        int worst_idx = ea_worst_index(population);
        if (ea_better(c1.fitness, population[worst_idx].fitness)) {
            population[worst_idx] = c1;
        }

        // Find the worst individual index again.
        // This is important because the population may have changed if c1 was inserted.
        worst_idx = ea_worst_index(population);

        // Compare the second child (c2) with the new worst individual
        if (ea_better(c2.fitness, population[worst_idx].fitness)) {
            population[worst_idx] = c2;
        }

        int best_now_idx = ea_best_index(population);
        if (ea_better(population[best_now_idx].fitness, best_global.fitness)) {
            best_global = population[best_now_idx];
        }

        m_last_convergence.push_back(best_global.fitness);
    }

    return ResultMH<double>(best_global.solution, best_global.fitness, evals);
}

ResultMH<double> GeneticAlgorithm::optimize(Problem<double> &problem, int maxevals) {
    if (m_scheme == GAScheme::AGG) {
        return run_agg(problem, maxevals);
    }
    return run_age(problem, maxevals);
}