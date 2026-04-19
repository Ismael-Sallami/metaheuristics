#pragma once

#include "mh.h"
#include "ea_common.h"

#include <vector>

class DifferentialEvolution : public MH<double> {
private:
    double m_f; // scale factor of mutation 
    double m_cr; // crossover rate
    int m_pop_size; // Target population size

    std::vector<double> m_last_convergence; // Stores the best fitness found at the end of each generation to plot convergence charts later.

public:
    DifferentialEvolution(double f, double cr, int pop_size)
        : m_f(f), m_cr(cr), m_pop_size(pop_size) {} // constructor

    ResultMH<double> optimize(Problem<double> &problem, int maxevals) override; // optimization process

    const std::vector<double> &last_convergence() const { return m_last_convergence; } // Retrieves the historical fitness tracking.
};