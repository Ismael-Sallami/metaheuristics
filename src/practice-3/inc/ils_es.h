#pragma once
#include "ils.h"
#include "simulated_annealing.h"

/**
 * Hybrid ILS-ES algorithm for the Portfolio Problem.
 *
 * Combines the Iterated Local Search framework with Simulated Annealing
 * instead of standard local search for the intensification phase.
 *
 * @author  Ismael Sallami Moreno
 */
class ILSES : public ILS {
private:
    double m_mu;            ///< SA initial temperature coefficient.
    double m_phi;           ///< SA initial acceptance probability.
    double m_tf;            ///< SA final temperature.
    int m_max_vecinos;      ///< SA neighbors per cooling step.
    int m_max_exitos;       ///< SA successes per cooling step.

public:
    /**
     * Constructs a hybrid ILS-ES instance.
     *
     * @param num_starts     Number of iterations.
     * @param max_evals_bl   Budget per annealing execution.
     * @param ratio          Neighborhood transfer ratio.
     * @param mu             SA worsening coefficient.
     * @param phi            SA initial prob.
     * @param tf             SA final temperature.
     * @param max_vecinos    SA neighbors per level.
     * @param max_exitos     SA successes per level.
     * @param mutation_rate  Perturbation fraction.
     */
    ILSES(int num_starts, int max_evals_bl, double ratio, double mu, double phi, double tf, int max_vecinos, int max_exitos, double mutation_rate = 0.2)
        : ILS(num_starts, max_evals_bl, ratio, mutation_rate), 
          m_mu(mu), m_phi(phi), m_tf(tf), m_max_vecinos(max_vecinos), m_max_exitos(max_exitos) {}

    virtual ~ILSES() {}

    /**
     * Optimizes the problem using SA as the local optimizer within the ILS framework.
     *
     * @param problem   The portfolio optimization problem.
     * @param maxevals  Global budget of function evaluations.
     * @return          The best solution found.
     */
    ResultMH<double> optimize(Problem<double> &problem, int maxevals) override;
};
