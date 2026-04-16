#pragma once
#include "mh.h"
#include "portfolio_problem.h"

using namespace std;

class LocalSearchBest : public MH<double> {
private:
    double m_ratio; // Ratio of the amount to transfer in the neighborhood (e.g., 0.4 for 40%)
public:
    LocalSearchBest(double ratio) : MH<double>(), m_ratio(ratio) {}
    virtual ~LocalSearchBest() {}

    ResultMH<double> optimize(Problem<double> &problem, int maxevals) override;
};