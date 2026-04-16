#include "soft_local_search.h"
#include "random.hpp"

#include <vector>
#include <utility>

int apply_soft_local_search(
    Problem<double> &problem,
    tSolution<double> &solution,
    tFitness &fitness,
    double ratio,
    int max_local_evals
) {
    const int n = static_cast<int>(solution.size());
    auto limits = problem.getSolutionDomainRange();
    const double lo = limits.first;
    const double hi = limits.second;

    std::vector<std::pair<int, int>> neighborhood;
    neighborhood.reserve(n * (n - 1));
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            if (i != j) {
                neighborhood.push_back({i, j});
            }
        }
    }

    int evals = 0;
    bool improved = true;

    while (evals < max_local_evals && improved) {
        improved = false;
        Random::shuffle(neighborhood);

        for (const auto &move : neighborhood) {
            if (evals >= max_local_evals) {
                break;
            }

            int i = move.first;
            int j = move.second;

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

            // Minimize fitness in Pr2.
            if (nfit < fitness) {
                solution = std::move(neighbor);
                fitness = nfit;
                improved = true;
                break;
            }
        }
    }

    return evals;
}
