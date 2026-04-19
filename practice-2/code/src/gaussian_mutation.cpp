#include "gaussian_mutation.h"
#include "random.hpp"
#include <cmath>

// Anonymous namespace to restrict visibility of these helpers to this translation unit only (internal linkage).
// Prevents ODR (One Definition Rule) violations if kPi or gaussian_noise are defined elsewhere.
namespace {
    constexpr double kPi = 3.14159265358979323846;

    /**
     * @brief Generates random numbers matching a Normal/Gaussian distribution N(0, sigma^2).
     * Uses the Box-Muller transform method.
     * * @param sigma The standard deviation (controls the mutation step size).
     * @return A random value from the Gaussian distribution.
     */
    double gaussian_noise(double sigma) {
        // u1 bounded to a minimum of 1e-12 to prevent std::log(0) which evaluates to -Infinity (NaN errors).
        double u1 = Random::get<double>(1e-12, 1.0);
        double u2 = Random::get<double>(0.0, 1.0);
        
        // Box-Muller transform: converts two independent uniform variables into a standard normal variable N(0,1)
        double z0 = std::sqrt(-2.0 * std::log(u1)) * std::cos(2.0 * kPi * u2);
        
        // Scale to the requested standard deviation
        return z0 * sigma;
    }
}

/**
 * @brief Applies Single-Point Gaussian Mutation to a given individual.
 * * @param solution The vector representing the portfolio weights. Modified in-place.
 * @param problem  Reference to the problem instance (used for constraints repair).
 * @param pm_indiv Probability of mutating this specific individual.
 * @param sigma    Standard deviation for the Gaussian noise (step size).
 */
void gaussian_mutate_individual(
    tSolution<double> &solution,
    Problem<double> &problem,
    double pm_indiv,
    double sigma) 
{
    // Roll the dice to see if this individual undergoes mutation
    if (Random::get<double>(0.0, 1.0) > pm_indiv) {
        return;
    }

    // Safety guard against empty solutions
    if (solution.empty()) {
        return;
    }

    // Select a single random gene (financial asset) to mutate (Single-Point Mutation strategy)
    int idx = Random::get<int>(0, static_cast<int>(solution.size()) - 1);
    
    // Apply the gaussian perturbation to the selected weight
    solution[idx] += gaussian_noise(sigma);

    // Constraint Handling: Repair the solution
    // Since a weight was modified, the sum of the portfolio is no longer 1.0.
    // The fix method normalizes the weights and ensures no negative allocations exist.
    problem.fix(solution);
}