#include "soft_local_search.h"
#include "random.hpp"

#include <vector>
#include <utility>

namespace {
void ensure_soft_ls_state(SoftLSState &state, int n) {
    if (state.dimension == n && !state.neighborhood.empty()) {
        return;
    }

    state.neighborhood.clear();
    state.neighborhood.reserve(n * (n - 1));
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            if (i != j) {
                state.neighborhood.push_back({i, j});
            }
        }
    }

    Random::shuffle(state.neighborhood);
    state.cursor = 0;
    state.dimension = n;
}
} // namespace

void reset_soft_local_search_state(SoftLSState &state) {
    state.neighborhood.clear();
    state.cursor = 0;
    state.dimension = -1;
}

int apply_soft_local_search(
    Problem<double> &problem,
    tSolution<double> &solution,
    tFitness &fitness,
    double ratio,
    int max_local_evals
) {
    SoftLSState local_state;
    return apply_soft_local_search(problem, solution, fitness, ratio, max_local_evals, local_state);
}

int apply_soft_local_search(
    Problem<double> &problem,
    tSolution<double> &solution,
    tFitness &fitness,
    double ratio,
    int max_local_evals,
    SoftLSState &state
) {
    const int n = static_cast<int>(solution.size());
    if (n < 2 || max_local_evals <= 0) {
        return 0;
    }

    ensure_soft_ls_state(state, n);

    auto limits = problem.getSolutionDomainRange();
    const double lo = limits.first;
    const double hi = limits.second;

    int evals = 0;

    const int total_moves = static_cast<int>(state.neighborhood.size());
    int inspected_since_improvement = 0;

    while (evals < max_local_evals && inspected_since_improvement < total_moves) {
        const auto &move = state.neighborhood[state.cursor];
        state.cursor = (state.cursor + 1) % total_moves;
        ++inspected_since_improvement;

        const int i = move.first;
        const int j = move.second;

        if (solution[i] == 0.0) {
            continue;
        }

        const double transfer = solution[i] * ratio;
        const double new_i = solution[i] - transfer;
        const double new_j = solution[j] + transfer;

        const bool valid_i = (new_i < 1e-8) || (new_i >= lo && new_i <= hi);
        const bool valid_j = (new_j < 1e-8) || (new_j >= lo && new_j <= hi);
        if (!valid_i || !valid_j) {
            continue;
        }

        auto neighbor = solution;
        neighbor[i] = (new_i < 1e-8) ? 0.0 : new_i;
        neighbor[j] = (new_j < 1e-8) ? 0.0 : new_j;

        const tFitness nfit = problem.fitness(neighbor);
        ++evals;

        if (nfit > fitness) {
            solution = std::move(neighbor);
            fitness = nfit;
            inspected_since_improvement = 0;
        }
    }

    return evals;
}