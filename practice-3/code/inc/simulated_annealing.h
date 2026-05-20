#pragma once
#include "mh.h"
#include "portfolio_problem.h"

/**
 * Simulated Annealing (ES) algorithm for the Portfolio Problem.
 *
 * Implements a trajectory-based metaheuristic that accepts worse solutions
 * with a certain probability to escape local optima.
 *
 * @author  Ismael Sallami Moreno
 */
class SimulatedAnnealing : public MH<double> {
private:
    double m_mu;        ///< Coefficient for initial temperature calculation (allowed worsening).
    double m_phi;       ///< Probability of accepting a worsening move at T0.
    double m_tf;        ///< Final temperature for the cooling scheme.
    double m_ratio;     ///< Transfer ratio for the neighborhood operator.
    int m_max_vecinos;  ///< Maximum number of neighbors explored per cooling step.
    int m_max_exitos;   ///< Maximum number of accepted moves per cooling step.

public:
    /**
     * Constructs a Simulated Annealing instance with specific cooling parameters.
     *
     * @param mu           Allowed worsening coefficient.
     * @param phi          Initial acceptance probability.
     * @param tf           Final temperature.
     * @param ratio        Neighborhood transfer ratio.
     * @param max_vecinos  Max neighbors per temperature level.
     * @param max_exitos   Max successes per temperature level.
     */
    SimulatedAnnealing(double mu, double phi, double tf, double ratio, int max_vecinos, int max_exitos)
        : m_mu(mu), m_phi(phi), m_tf(tf), m_ratio(ratio), m_max_vecinos(max_vecinos), m_max_exitos(max_exitos) {}

    virtual ~SimulatedAnnealing() {}

    /**
     * Executes the simulated annealing optimization starting from a random solution.
     *
     * @param problem   The portfolio optimization problem.
     * @param maxevals  Total budget of function evaluations.
     * @return          The best solution found and its fitness.
     */
    ResultMH<double> optimize(Problem<double> &problem, int maxevals) override;

    /**
     * Executes the simulated annealing optimization starting from a provided solution.
     *
     * @param problem       The portfolio optimization problem.
     * @param maxevals      Total budget of function evaluations.
     * @param initial_sol   The starting point for the trajectory.
     * @return              The best solution found and its fitness.
     */
    ResultMH<double> optimize(Problem<double> &problem, int maxevals, const tSolution<double>& initial_sol);
};
