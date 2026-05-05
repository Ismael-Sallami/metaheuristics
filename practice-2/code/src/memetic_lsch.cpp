#include "memetic_lsch.h"
#include "soft_local_search.h"
#include <algorithm>
#include <numeric>

MemeticLSCh::MemeticLSCh(
    CrossoverType crossover_type, int pop_size, double pc, double pm_indiv,
    double blx_alpha, double mutation_ratio, int ls_period, double ls_ratio,
    int i_str
) : m_crossover_type(crossover_type), m_pop_size(pop_size), m_pc(pc),
    m_pm_indiv(pm_indiv), m_blx_alpha(blx_alpha), m_mutation_ratio(mutation_ratio),
    m_ls_period(ls_period), m_ls_ratio(ls_ratio), m_i_str(i_str) {}

void MemeticLSCh::crossover_pair(
    const tSolution<double> &p1, const tSolution<double> &p2,
    tSolution<double> &c1, tSolution<double> &c2
) const {
    if (m_crossover_type == CrossoverType::ARITHMETIC) {
        ea_arithmetic_crossover(p1, p2, c1, c2);
    } else {
        ea_blx_crossover(p1, p2, m_blx_alpha, c1, c2);
    }
}

ResultMH<double> MemeticLSCh::optimize(Problem<double> &problem, int maxevals) {
    m_last_convergence.clear();

    unsigned int evals = 0;
    int generation = 0;

    auto population = ea_initialize_population(problem, m_pop_size, evals);
    std::vector<SoftLSState> ls_state(m_pop_size);
    EAIndividual best_global = population[ea_best_index(population)];

    while (evals + static_cast<unsigned int>(m_pop_size) <= static_cast<unsigned int>(maxevals)) {

        std::vector<int> parent_indices;
        parent_indices.reserve(m_pop_size);
        std::vector<EAIndividual> parents;
        parents.reserve(m_pop_size);
        for (int i = 0; i < m_pop_size; ++i) {
            int idx = ea_tournament_k3(population);
            parent_indices.push_back(idx);
            parents.push_back(population[idx]);
        }

        std::vector<EAIndividual> children(m_pop_size);
        std::vector<SoftLSState> child_state(m_pop_size);

        const int num_pairs = m_pop_size / 2;
        const int num_cross_pairs = static_cast<int>(m_pc * num_pairs);

        // Crossover with expected number of crossed pairs.
        for (int pair = 0; pair < num_pairs; ++pair) {
            const int i = 2 * pair;
            const int j = i + 1;
            const auto &p1 = parents[i].solution;
            const auto &p2 = parents[j].solution;

            tSolution<double> c1, c2;
            if (pair < num_cross_pairs) {
                crossover_pair(p1, p2, c1, c2);
                reset_soft_local_search_state(child_state[i]);
                reset_soft_local_search_state(child_state[j]);
            } else {
                c1 = p1; c2 = p2;
                child_state[i] = ls_state[parent_indices[i]];
                child_state[j] = ls_state[parent_indices[j]];
            }

            children[i].solution = c1;
            children[j].solution = c2;
        }

        if (m_pop_size % 2 != 0) {
            const int last = m_pop_size - 1;
            children[last].solution = parents[last].solution;
            child_state[last] = ls_state[parent_indices[last]];
        }

        // Mutation with expected number of mutated individuals.
        const int num_mut_individuals = static_cast<int>(m_pm_indiv * m_pop_size);
        if (num_mut_individuals > 0) {
            std::vector<int> mutation_indices(m_pop_size);
            std::iota(mutation_indices.begin(), mutation_indices.end(), 0);
            Random::shuffle(mutation_indices);

            const int limit = std::min(num_mut_individuals, m_pop_size);
            for (int k = 0; k < limit; ++k) {
                const int idx = mutation_indices[k];
                ea_mutate_transfer(children[idx].solution, problem, 1.0, m_mutation_ratio);
                reset_soft_local_search_state(child_state[idx]);
            }
        }

        // Evaluate Children
        for (int i = 0; i < m_pop_size; ++i) {
            ea_evaluate_individual(problem, children[i], evals);
        }

        // Strict Elitism 
        const EAIndividual best_prev = population[ea_best_index(population)];
        const SoftLSState best_prev_state = ls_state[ea_best_index(population)];

        population = std::move(children);
        ls_state = std::move(child_state);
        ++generation;

        // Canonical MA-LSCh: refine only the best individual periodically.
        if (m_ls_period > 0 && generation % m_ls_period == 0) {
            const int idx = ea_best_index(population);
            if (evals < static_cast<unsigned int>(maxevals)) {
                int remaining = static_cast<int>(maxevals - evals);
                int budget = std::min(m_i_str, remaining);
                if (budget > 0) {
                    int used = apply_soft_local_search(
                        problem,
                        population[idx].solution,
                        population[idx].fitness,
                        m_ls_ratio,
                        budget,
                        ls_state[idx]
                    );
                    evals += used;
                }
            }
        }

        // Elitism after BL: compare against the refined population, not the raw children.
        int best_child_idx = ea_best_index(population);
        if (ea_better(best_prev.fitness, population[best_child_idx].fitness)) {
            int worst = ea_worst_index(population);
            population[worst] = best_prev;
            ls_state[worst] = best_prev_state;
        }

        // Update Global Tracking
        int best_now_idx = ea_best_index(population);
        if (ea_better(population[best_now_idx].fitness, best_global.fitness)) {
            best_global = population[best_now_idx];
        }
        
        m_last_convergence.push_back(best_global.fitness);

        if (evals >= static_cast<unsigned int>(maxevals)) break;
    }

    return ResultMH<double>(best_global.solution, best_global.fitness, evals);
}