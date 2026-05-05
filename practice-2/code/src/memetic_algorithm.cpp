#include "memetic_algorithm.h"
#include "soft_local_search.h"

#include <algorithm>
#include <numeric>

MemeticAlgorithm::MemeticAlgorithm(
    MemeticVariant variant,
    CrossoverType crossover_type,
    int pop_size,
    double pc,
    double pm_indiv,
    double blx_alpha,
    double mutation_ratio,
    int ls_period,
    int ls_max_evals,
    double ls_ratio,
    double pls_rand
) : m_variant(variant),
    m_crossover_type(crossover_type),
    m_pop_size(pop_size),
    m_pc(pc),
    m_pm_indiv(pm_indiv),
    m_blx_alpha(blx_alpha),
    m_mutation_ratio(mutation_ratio),
    m_ls_period(ls_period),
    m_ls_max_evals(ls_max_evals),
    m_ls_ratio(ls_ratio),
    m_pls_rand(pls_rand) {}

void MemeticAlgorithm::crossover_pair(
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

ResultMH<double> MemeticAlgorithm::optimize(Problem<double> &problem, int maxevals) {
    m_last_convergence.clear();

    unsigned int evals = 0;
    int generation = 0;

    auto population = ea_initialize_population(problem, m_pop_size, evals);
    EAIndividual best_global = population[ea_best_index(population)];

    while (evals + static_cast<unsigned int>(m_pop_size) <= static_cast<unsigned int>(maxevals)) {
        std::vector<EAIndividual> parents;
        parents.reserve(m_pop_size);

        // Pick the parents with tournament selection.
        for (int i = 0; i < m_pop_size; ++i) {
            int idx = ea_tournament_k3(population);
            parents.push_back(population[idx]);
        }

        std::vector<EAIndividual> children(m_pop_size);

        const int num_pairs = m_pop_size / 2;
        const int num_cross_pairs = static_cast<int>(m_pc * num_pairs);

        // Cross only the expected number of pairs.
        for (int pair = 0; pair < num_pairs; ++pair) {
            const int i = 2 * pair;
            const int j = i + 1;

            const auto &p1 = parents[i].solution;
            const auto &p2 = parents[j].solution;

            tSolution<double> c1, c2;
            if (pair < num_cross_pairs) {
                crossover_pair(p1, p2, c1, c2);
            } else {
                c1 = p1;
                c2 = p2;
            }

            children[i].solution = c1;
            children[j].solution = c2;
        }

        if (m_pop_size % 2 != 0) {
            children[m_pop_size - 1].solution = parents[m_pop_size - 1].solution;
        }

        // Expected mutation count in AM is the same 0.1 * population logic.
        const int num_mut_individuals = static_cast<int>(m_pm_indiv * m_pop_size);

        if (num_mut_individuals > 0) {
            // Shuffle the population and mutate only the chosen indexes.
            std::vector<int> mutation_indices(m_pop_size);
            std::iota(mutation_indices.begin(), mutation_indices.end(), 0); // Fill with 0, 1, ..., pop_size-1
            Random::shuffle(mutation_indices);

            const int limit = std::min(num_mut_individuals, m_pop_size);
            for (int k = 0; k < limit; ++k) {
                const int idx = mutation_indices[k];
                ea_mutate_transfer(children[idx].solution, problem, 1.0, m_mutation_ratio);
            }
        }

        for (int i = 0; i < m_pop_size; ++i) {
            ea_evaluate_individual(problem, children[i], evals);
        }

        const EAIndividual best_prev = population[ea_best_index(population)];

        population = std::move(children);
        ++generation;

        if (m_ls_period > 0 && (generation % m_ls_period == 0)) {
            std::vector<int> selected;

            // Choose who will receive local search in this generation.
            if (m_variant == MemeticVariant::ALL) {
                selected.resize(m_pop_size);
                for (int i = 0; i < m_pop_size; ++i) selected[i] = i;
            } else if (m_variant == MemeticVariant::RAND) {
                for (int i = 0; i < m_pop_size; ++i) {
                    if (Random::get<double>(0.0, 1.0) <= m_pls_rand) {
                        selected.push_back(i);
                    }
                }
            } else {
                // BEST uses the top 10 percent of the population.
                int k = std::max(1, static_cast<int>(0.1 * m_pop_size));
                selected = ea_best_k_indices(population, k);
            }

            for (int idx : selected) {
                if (evals >= static_cast<unsigned int>(maxevals)) {
                    break;
                }

                // Respect the global evaluation limit before local search.
                int remaining = static_cast<int>(maxevals - evals);
                int local_budget = std::min(m_ls_max_evals, remaining);
                if (local_budget <= 0) {
                    break;
                }

                int used = apply_soft_local_search(
                    problem,
                    population[idx].solution,
                    population[idx].fitness,
                    m_ls_ratio,
                    local_budget
                );

                evals += used;
            }
        }

        // Elitism is applied after BL so the comparison uses the refined children.
        int best_child_idx = ea_best_index(population);
        if (ea_better(best_prev.fitness, population[best_child_idx].fitness)) {
            int worst_child_idx = ea_worst_index(population);
            population[worst_child_idx] = best_prev;
        }

        int best_now_idx = ea_best_index(population);
        if (ea_better(population[best_now_idx].fitness, best_global.fitness)) {
            best_global = population[best_now_idx];
        }

        m_last_convergence.push_back(best_global.fitness);

        if (evals >= static_cast<unsigned int>(maxevals)) {
            break;
        }
    }

    return ResultMH<double>(best_global.solution, best_global.fitness, evals);
}