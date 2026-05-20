#pragma once
#include "mh.h"
#include "portfolio_problem.h"
#include "simulated_annealing.h"

/**
 * ILS-ES enhanced with Stagnation Detection and Diversification (CHC/GADEGD).
 * 
 * Implements Iterated Local Search combined with Simulated Annealing,
 * adding a stagnation monitor (premature convergence detection). When the search
 * fails to improve for several iterations, a "catastrophic" macromutation (~40% of variables)
 * is applied to escape to unexplored regions of the search space.
 *
 * Inspired by:
 *   - CHC (Eshelman & Schaffer, 1993): Diversification upon stagnation detection.
 *   - GADEGD (course proposal): Injection of randomized greedy solutions.
 *
 * @author  Ismael Sallami Moreno
 */
class ILSES_CHC : public MH<double> {
private:
    double m_mu;                        ///< Coefficient for initial temperature T0 (allowed worsening).
    double m_phi;                       ///< Acceptance probability at T0.
    double m_tf;                        ///< Final temperature for SA.
    double m_ratio;                     ///< Neighborhood transfer ratio.
    int m_max_vecinos;                  ///< Max neighbors per temperature level in SA.
    int m_max_exitos;                   ///< Max successes per temperature level in SA.
    int m_num_starts;                   ///< Total number of local search restarts.
    int m_limit_stagnation;             ///< Iterations without improvement before divergence.
    double m_mutation_rate_normal;      ///< Standard mutation rate (ILS): 0.20.
    double m_mutation_rate_macro;       ///< Macro-mutation rate for divergence (CHC): 0.40-0.50.

public:
    /**
     * Constructs an ILS-ES instance with stagnation-triggered diversification.
     *
     * @param mu                     Worsening coefficient for initial temperature.
     * @param phi                    Acceptance probability at initial temperature.
     * @param tf                     Final temperature.
     * @param ratio                  Transfer ratio for local search.
     * @param max_vecinos            Max neighbors per temperature level.
     * @param max_exitos             Max successes per temperature level.
     * @param num_starts             Total number of restarts.
     * @param limit_stagnation       Iterations without improvement before macro-mutation.
     * @param mutation_macro         Macro-mutation rate (default 0.40).
     */
    ILSES_CHC(double mu, double phi, double tf, double ratio,
              int max_vecinos, int max_exitos, int num_starts,
              int limit_stagnation = 10, double mutation_macro = 0.40)
        : m_mu(mu), m_phi(phi), m_tf(tf), m_ratio(ratio),
          m_max_vecinos(max_vecinos), m_max_exitos(max_exitos),
          m_num_starts(num_starts), m_limit_stagnation(limit_stagnation),
          m_mutation_rate_normal(0.20), m_mutation_rate_macro(mutation_macro) {}

    virtual ~ILSES_CHC() {}

    /**
     * Runs the ILS-ES algorithm with stagnation-based diversification.
     *
     * @param problem       The optimization problem (Portfolio).
     * @param maxevals      Maximum evaluation budget.
     * @return              Best solution found.
     */
    ResultMH<double> optimize(Problem<double> &problem, int maxevals) override;

private:
    /**
     * Apply standard mutation (20% of assets modified).
     *
     * @param sol       Solution to mutate.
     * @param rate      Fraction of variables to modify.
     */
    void mutate(tSolution<double>& sol, double rate);

    /**
     * Apply CHC-style macro-mutation (40-50% of variables modified).
     * Used when stagnation is detected to force diversification.
     *
     * @param sol       Solution to macro-mutate.
     * @param rate      Fraction of variables to modify (0.4-0.5).
     */
    void macromutate(tSolution<double>& sol, double rate);

    /**
     * Generate a new solution via Randomized Greedy construction (GADEGD).
     * Alternative to macro-mutation: provides fresh solutions of good quality.
     *
     * @param problem   Problem reference.
     * @return          Randomized greedy solution.
     */
    tSolution<double> constructGreedyRandomized(Problem<double>& problem);
};
