#include "gaussian_mutation.h"
#include "random.hpp"

#include <cmath>

namespace {
constexpr double kPi = 3.14159265358979323846;

double gaussian_noise(double sigma) {
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
    double sigma
) {
    if (Random::get<double>(0.0, 1.0) > pm_indiv) {
        return;
    }

    if (solution.empty()) {
        return;
    }

    int idx = Random::get<int>(0, static_cast<int>(solution.size()) - 1);
    solution[idx] += gaussian_noise(sigma);

    problem.fix(solution);
}
