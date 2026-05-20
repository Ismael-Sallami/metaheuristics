#pragma once
#include "mh.h"
#include "portfolio_problem.h"

using namespace std;

class LocalSearch : public MH<double> {
private:
    double m_ratio; // Ratio of the amount to transfer in the neighborhood (e.g., 0.4 for 40%)
public:
    LocalSearch(double ratio) : MH<double>(), m_ratio(ratio) {}
    virtual ~LocalSearch() {}

    ResultMH<double> optimize(Problem<double> &problem, int maxevals) override;
    ResultMH<double> optimize(Problem<double> &problem, int maxevals, const tSolution<double>& initial_sol);
};