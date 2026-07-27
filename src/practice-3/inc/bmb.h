#pragma once
#include "mh.h"
#include "localsearch.h"

/**
 * Basic Multi-start Search (BMB) algorithm for the Portfolio Problem.
 *
 * This algorithm executes several independent local search starts from
 * random initial solutions to explore different regions of the search space.
 *
 * @author  Ismael Sallami Moreno
 */
class BMB : public MH<double> {
private:
    int m_num_starts;   ///< Total number of independent starts.
    int m_max_evals_bl; ///< Maximum budget for each local search execution.
    double m_ratio;     ///< Transfer ratio for the internal local search.

public:
    /**
     * Constructs a BMB instance with specified multi-start parameters.
     *
     * @param num_starts    The number of starts (iterations).
     * @param max_evals_bl  Evaluation limit for each local search start.
     * @param ratio         Neighborhood transfer ratio.
     */
    BMB(int num_starts, int max_evals_bl, double ratio)
        : m_num_starts(num_starts), m_max_evals_bl(max_evals_bl), m_ratio(ratio) {}

    virtual ~BMB() {}

    /**
     * Optimizes the portfolio problem using multiple independent starts.
     *
     * @param problem   The portfolio optimization problem.
     * @param maxevals  Global budget of function evaluations.
     * @return          The best solution found across all starts.
     */
    ResultMH<double> optimize(Problem<double> &problem, int maxevals) override;
};
