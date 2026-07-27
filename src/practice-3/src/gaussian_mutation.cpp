#include "gaussian_mutation.h"
#include "random.hpp"
#include <cmath>
#include <algorithm>
#include <numeric>

namespace {
    constexpr double kPi = 3.14159265358979323846;

    double gaussian_noise(double sigma) {
        // Avoid log(0) in Box-Muller.
        double u1 = Random::get<double>(1e-12, 1.0);
        double u2 = Random::get<double>(0.0, 1.0);

        double z0 = std::sqrt(-2.0 * std::log(u1)) * std::cos(2.0 * kPi * u2);

        return z0 * sigma;
    }
}

void gaussian_mutate_individual(
    tSolution<double> &solution,
    Problem<double> &problem,
    double pm_indiv,
    double sigma)
{
    if (solution.empty()) {
        return;
    }

    const auto original = solution;
    const auto bounds = problem.getSolutionDomainRange();
    const double lo = bounds.first;
    const double hi = bounds.second;
    constexpr double eps = 1e-8;

    const auto in_domain = [&](double w) {
        return std::abs(w) <= eps || (w >= lo - eps && w <= hi + eps);
    };

    const double pm_gene = std::min(1.0, pm_indiv / static_cast<double>(solution.size()));
    bool mutated = false;

    for (double &gene : solution) {
        if (Random::get<double>(0.0, 1.0) <= pm_gene) {
            const double candidate = gene + gaussian_noise(sigma);
            if (in_domain(candidate)) {
                gene = candidate;
                mutated = true;
            }
        }
    }

    if (!mutated) {
        return;
    }

    double sum = std::accumulate(solution.begin(), solution.end(), 0.0);
    const double delta = 1.0 - sum;

    if (std::abs(delta) <= eps && problem.isValid(solution)) {
        return;
    }

    std::vector<int> indices(solution.size());
    for (int i = 0; i < static_cast<int>(solution.size()); ++i) {
        indices[i] = i;
    }
    std::shuffle(indices.begin(), indices.end(), Random::engine());

    for (int idx : indices) {
        const double old_value = solution[idx];
        const double candidate = old_value + delta;

        if (!in_domain(candidate)) {
            continue;
        }

        solution[idx] = candidate;
        if (problem.isValid(solution)) {
            return;
        }
        solution[idx] = old_value;
    }

    solution = original;
}