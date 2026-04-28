#include "portfolio_problem.h"
#include <iostream>
#include <cmath>

using namespace std;

// Define the constructor
PortfolioProblem::PortfolioProblem(const string& filename, double lo, double hi, bool is_2025, double lambda) : m_lo(lo), m_hi(hi), m_lambda(lambda) {
    m_data = CsvReader::read(filename, is_2025);
}

size_t PortfolioProblem::getSolutionSize() {
    return m_data.numAssets;
}

pair<double, double> PortfolioProblem::getSolutionDomainRange() {
    return {m_lo, m_hi};
}

bool PortfolioProblem::isValid(const tSolution<double> &solution) {
    double sum = 0.0;
    for (size_t i = 0; i<solution.size(); ++i){
        double w = solution[i];
        sum += w;

        // Add a margin of error 1e-8 to the boundaries to avoid false positives
        if (w != 0.0 && (w < m_lo - 1e-8 || w > m_hi + 1e-8)){
            return false;
        }
    }
    if (abs(sum - 1.0) > 1e-8){
        return false; // We know machines are not perfect, so we allow them a margin of error
    }
    return true;
}

// For fitness we know it is benefit - lambda * risk, since the benefit uses logarithms, we calculate it in the problem class for efficiency reasons

tFitness PortfolioProblem::fitness(const tSolution<double> &solution) {
    double risk = 0.0;
    double profit = 0.0;

    for (size_t i = 0; i < solution.size(); ++i) {
        profit += solution[i] * m_data.logBenefits[i];
    }

    for (size_t i = 0; i < solution.size(); ++i) {
        for (size_t j = 0; j < solution.size(); ++j) {
            risk += solution[i] * solution[j] * m_data.covariance[i][j];
        }
    }

    // in the pr2 the risk term is the portfolio standard deviation
    return profit - m_lambda * sqrt(std::max(risk, 0.0)); // We use max to avoid negative values due to numerical issues
}

tSolution<double> PortfolioProblem::createSolution() {
    size_t n = getSolutionSize();
    tSolution<double> sol(n, 0.0);

    for (size_t i = 0; i < n; ++i) {
        sol[i] = Random::get<double>(m_lo, m_hi); // Assign a random value between m_lo and m_hi to each weight
    }

    // Choose the number of zeros randomly between 1 and N-1
    int num_zeros = Random::get<int>(1, n - 1);
    int zeros_colocados = 0;

    // Place zeros in random positions
    while (zeros_colocados<num_zeros)
    {
        int pos = Random::get<int>(0,n-1);
        if(sol[pos]!=0.0){
            sol[pos] = 0.0;
            zeros_colocados++;
        }
    }

    // Now we normalize and repair the solution
    fix(sol);

    return sol;
    
}

void PortfolioProblem::fix(tSolution<double> &solution) {
    size_t n = getSolutionSize();
    double sum = 0.0;
    for (double w : solution) sum += w;

    // First normalize
    if (sum > 0.0 && abs(sum - 1.0) > 1e-8){
        for(size_t i = 0; i<n; ++i){
            solution[i] /= sum;
        }
    }

    // Ensure the range, w_i = 0 if w_i < m_lo, w_i = m_hi if w_i > m_hi

    for(size_t i = 0; i<n; ++i){
        if (solution[i] < m_lo) solution[i] = 0.0;
        else if (solution[i] > m_hi) solution[i] = m_hi;
    }

    // Recalculate sum after trimming

    sum = 0.0;
    for (double w : solution) sum += w;

    // Check if the sum of weights is greater or less than 1

    int failsafe = 0; // To avoid infinite loops...

    while (abs(sum - 1.0) > 1e-8 && failsafe < 1000) {
        failsafe++;
        double dif = 1.0 - sum;
        if (dif>0){
            // Sum less than 1, choose weights >0 and <hi
            vector<int> candidatos;
            for (size_t i = 0; i < n; ++i) {
                if (solution[i] > 0.0 && solution[i] < m_hi) {
                    candidatos.push_back(i);
                }
            }

            // If not enough, increment weights equal to 0 chosen randomly
            if (candidatos.empty()) {
                vector<int> ceros;
                for (size_t i = 0; i < n; ++i) {
                    if (solution[i] == 0.0) ceros.push_back(i);
                }
                
                if (!ceros.empty()) {
                    // Choose a random zero and give it the minimum legal investment
                    int lucky_zero = ceros[Random::get<int>(0, ceros.size() - 1)];
                    solution[lucky_zero] = m_lo; 
                    
                    // Recalculate sum and re-evaluate the while without doing anything else
                    sum = 0.0;
                    for (double w : solution) sum += w;
                    continue; 
                }
            }

            if (candidatos.empty()) {
                break; // No candidates to adjust, exit the loop
            }

            // Divide the difference among those weights by adding a value
            double incremento = dif / candidatos.size();
            for (int idx : candidatos) {
                double max_posible = m_hi - solution[idx];
                double a_sumar = min(incremento, max_posible);
                solution[idx] += a_sumar;
            }

        }
        else{
            // Sum greater than 1, choose weights greater than m_lo
            vector<int> candidatos;
            for (size_t i = 0; i < n; ++i) {
                if (solution[i] > m_lo) {
                    candidatos.push_back(i);    
                }
            }
            if (candidatos.empty()) {
                break; // No candidates to adjust, exit the loop
            }
            // Divide the difference by subtracting a value
            double decremento = abs(dif) / candidatos.size();
            for (int idx : candidatos) {
                double max_posible = solution[idx] - m_lo;
                double a_restar = min(decremento, max_posible);
                solution[idx] -= a_restar;
            }
        }
        // Recalculate sum to evaluate while condition
        sum = 0.0;
        for (double w : solution) sum += w;
    }
    
}

double PortfolioProblem::getGreedyHeuristic(int companyIndex) {
    double profit = m_data.logBenefits[companyIndex]; 
    
    double covariance_sum = 0.0;
    size_t n = getSolutionSize();
    for (size_t j = 0; j < n; ++j) {
        covariance_sum += m_data.covariance[companyIndex][j];
    }
    
    double heuristic = profit - (m_lambda * sqrt(std::max(covariance_sum, 0.0))); 
    
    return heuristic;
}

double PortfolioProblem::getProfit(const tSolution<double> &solution) {
    double profit = 0.0;

    for (size_t i = 0; i < solution.size(); ++i) {
        profit += solution[i] * m_data.logBenefits[i];
    }

    return profit;
}
