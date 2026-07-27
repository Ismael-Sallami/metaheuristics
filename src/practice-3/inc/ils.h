#pragma once
#include "mh.h"
#include "localsearch.h"

/**
 * Iterated Local Search (ILS) algorithm for the Portfolio Problem.
 *
 * Implements a metaheuristic that alternates between local search phases 
 * and perturbation (mutation) steps to escape local optima and explore
 * connected regions of the search space.
 *
 * @author  Ismael Sallami Moreno
 */
class ILS : public MH<double> {
protected:
    int m_num_starts;       ///< Total number of iterations (starts).
    int m_max_evals_bl;     ///< Maximum budget for each internal local search.
    double m_ratio;         ///< Transfer ratio for the local search neighborhood.
    double m_mutation_rate; ///< Fraction of assets to mutate during perturbation.

    /**
     * Perturbs a solution by shuffling weights among a subset of assets.
     *
     * @param sol The solution to perturb (modified in-place).
     */
    void mutate(tSolution<double>& sol);

public:
    /**
     * Constructs an ILS instance with specified search parameters.
     *
     * @param num_starts     Number of ILS iterations.
     * @param max_evals_bl   Evaluation limit for internal local search.
     * @param ratio          Neighborhood transfer ratio.
     * @param mutation_rate  Fraction of the solution size to perturb.
     */
    ILS(int num_starts, int max_evals_bl, double ratio, double mutation_rate = 0.2)
        : m_num_starts(num_starts), m_max_evals_bl(max_evals_bl), m_ratio(ratio), m_mutation_rate(mutation_rate) {}

    virtual ~ILS() {}

    /**
     * Optimizes the portfolio problem by iterating between intensity (LS) and diversity (Mutation).
     *
     * @param problem   The portfolio optimization problem.
     * @param maxevals  Global budget of function evaluations.
     * @return          The best solution found.
     */
    ResultMH<double> optimize(Problem<double> &problem, int maxevals) override;
};
