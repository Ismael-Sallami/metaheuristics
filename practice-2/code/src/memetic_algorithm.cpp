#include "memetic_algorithm.h"
#include "soft_local_search.h"

#include <algorithm>

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

        for (int i = 0; i < m_pop_size; ++i) {
            int idx = ea_tournament_k3(population);
            parents.push_back(population[idx]);
        }

        std::vector<EAIndividual> children(m_pop_size);

        for (int i = 0; i < m_pop_size; i += 2) {
            const auto &p1 = parents[i].solution;
            const auto &p2 = parents[(i + 1) % m_pop_size].solution;

            tSolution<double> c1, c2;
            if (Random::get<double>(0.0, 1.0) <= m_pc) {
                crossover_pair(p1, p2, c1, c2);
            } else {
                c1 = p1;
                c2 = p2;
            }

            ea_mutate_transfer(c1, problem, m_pm_indiv, m_mutation_ratio);
            ea_mutate_transfer(c2, problem, m_pm_indiv, m_mutation_ratio);

            children[i].solution = c1;
            children[(i + 1) % m_pop_size].solution = c2;
        }

        for (int i = 0; i < m_pop_size; ++i) {
            ea_evaluate_individual(problem, children[i], evals);
        }

        int best_prev_idx = ea_best_index(population);
        int best_child_idx = ea_best_index(children);

        // AGG elitism before memetic local search stage.
        if (ea_better(population[best_prev_idx].fitness, children[best_child_idx].fitness)) {
            int worst_child_idx = ea_worst_index(children);
            children[worst_child_idx] = population[best_prev_idx];
        }

        population = std::move(children);
        ++generation;

        if (m_ls_period > 0 && (generation % m_ls_period == 0)) {
            std::vector<int> selected;

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
                int k = std::max(1, static_cast<int>(0.1 * m_pop_size));
                selected = ea_best_k_indices(population, k);
            }

            for (int idx : selected) {
                if (evals >= static_cast<unsigned int>(maxevals)) {
                    break;
                }

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
