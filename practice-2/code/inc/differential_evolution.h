#pragma once

#include "mh.h"
#include "ea_common.h"

#include <vector>

class DifferentialEvolution : public MH<double> {
private:
    double m_f;
    double m_cr;
    int m_pop_size;

    std::vector<double> m_last_convergence;

public:
    DifferentialEvolution(double f, double cr, int pop_size)
        : m_f(f), m_cr(cr), m_pop_size(pop_size) {}

    ResultMH<double> optimize(Problem<double> &problem, int maxevals) override;

    const std::vector<double> &last_convergence() const { return m_last_convergence; }
};
