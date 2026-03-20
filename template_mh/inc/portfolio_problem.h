#ifndef __PORTFOLIO_PROBLEM_H
#define __PORTFOLIO_PROBLEM_H

#include <string>
#include <vector>
#include <utility>
#include "problem.h"
#include "csv_reader.h"

class PortfolioProblem : public Problem<double> {
private:
    CsvReader::Data m_data;
    double m_lo;
    double m_hi;
    double m_lambda = 500.0;

public:
    // Constructor
    PortfolioProblem(const std::string& filename, double lo, double hi);

    // Métodos sobreescritos de Problem<double>
    size_t getSolutionSize() override;
    std::pair<double, double> getSolutionDomainRange() override;
    bool isValid(const tSolution<double> &solution) override;
    tFitness fitness(const tSolution<double> &solution) override;
    tSolution<double> createSolution() override;
    void fix(tSolution<double> &solution) override;
};

#endif