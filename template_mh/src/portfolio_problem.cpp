#include "portfolio_problem.h"
#include <iostream>
#include <cmath>

using namespace std;

// definimos el constructor
PortfolioProblem::PortfolioProblem(const string& filename, double lo, double hi) : m_lo(lo), m_hi(hi) {
    m_data = CsvReader::read(filename);
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

        // Añadimos un margen de error 1e-8 a las fronteras para evitar falsos positivos
        if (w != 0.0 && (w < m_lo - 1e-8 || w > m_hi + 1e-8)){
            return false;
        }
    }
    if (abs(sum - 1.0) > 1e-8){
        return false; // sabemos que las máquinas no son perfectas, por ende, les permitimos un margen de error
    }
    return true;
}

// para el fitness sabemos que es beneficio - lambda * riesgo, como el beneficio usa logaritmos, hacemos que en la clase problema lo calcule, por temas de eficiencia

tFitness PortfolioProblem::fitness(const tSolution<double> &solution) {
    double riesgo = 0.0;
    double beneficio = 0.0;

    for (size_t i = 0; i < solution.size(); ++i) {
        beneficio += solution[i] * m_data.logBenefits[i];
    }

    for (size_t i = 0; i < solution.size(); ++i) {
        for (size_t j = 0; j < solution.size(); ++j) {
            riesgo += solution[i] * solution[j] * m_data.covariance[i][j];
        }
    }

    return beneficio - (m_lambda * riesgo);
}

tSolution<double> PortfolioProblem::createSolution() {
    size_t n = getSolutionSize();
    tSolution<double> sol(n, 0.0);

    for (size_t i = 0; i < n; ++i) {
        sol[i] = Random::get<double>(m_lo, m_hi); // asignamos un valor aleatorio entre m_lo y m_hi a cada peso
    }

    // escogemos el número de ceros de forma aleatoria entre 1 y N-1
    int num_zeros = Random::get<int>(1, n - 1);
    int zeros_colocados = 0;

    // colocamos los ceros en posiciones aleatorias
    while (zeros_colocados<num_zeros)
    {
        int pos = Random::get<int>(0,n-1);
        if(sol[pos]!=0.0){
            sol[pos] = 0.0;
            zeros_colocados++;
        }
    }

    // ahora debemos de normalizar y de reparar la sol
    fix(sol);

    return sol;
    
}

void PortfolioProblem::fix(tSolution<double> &solution) {
    size_t n = getSolutionSize();
    double sum = 0.0;
    for (double w : solution) sum += w;

    // primero normalizamos
    if (sum > 0.0 && abs(sum - 1.0) > 1e-8){
        for(size_t i = 0; i<n; ++i){
            solution[i] /= sum;
        }
    }

    // aseguramos el rango, w_i = 0 si w_i < m_lo, w_i = 1 si w_i > m_hi

    for(size_t i = 0; i<n; ++i){
        if (solution[i] < m_lo) solution[i] = 0.0;
        else if (solution[i] > m_hi) solution[i] = m_hi;
    }

    // recalculamos la suma tras el recorte

    sum = 0.0;
    for (double w : solution) sum += w;

    // comprobamos si la suma de pesos es mayor o menor que 1

    int failsafe = 0; // para evitar bucles infinitos ...

    while (abs(sum - 1.0) > 1e-8 && failsafe < 1000) {
        failsafe++;
        double dif = 1.0 - sum;
        if (dif>0){
            // suma menor que 1, elegimos pesos >0 y <hi
            vector<int> candidatos;
            for (size_t i = 0; i < n; ++i) {
                if (solution[i] > 0.0 && solution[i] < m_hi) {
                    candidatos.push_back(i);
                }
            }

            // si no es suficiente, incrementamos pesos iguales a 0 elegidos aleatoriamente
            if (candidatos.empty()) {
                vector<int> ceros;
                for (size_t i = 0; i < n; ++i) {
                    if (solution[i] == 0.0) ceros.push_back(i);
                }
                
                if (!ceros.empty()) {
                    // Elegimos un cero aleatorio y le damos la inversion minima legal
                    int lucky_zero = ceros[Random::get<int>(0, ceros.size() - 1)];
                    solution[lucky_zero] = m_lo; 
                    
                    // Recalculamos la suma y volvemos a evaluar el while sin hacer nada mas
                    sum = 0.0;
                    for (double w : solution) sum += w;
                    continue; 
                }
            }

            if (candidatos.empty()) {
                break; // no hay candidatos para ajustar, salimos del bucle
            }

            // dividimos la dif entre esos pesos sumándole un valor
            double incremento = dif / candidatos.size();
            for (int idx : candidatos) {
                double max_posible = m_hi - solution[idx];
                double a_sumar = min(incremento, max_posible);
                solution[idx] += a_sumar;
            }

        }
        else{
            // suma mayor que 1, escoger pesos mayores que lo
            vector<int> candidatos;
            for (size_t i = 0; i < n; ++i) {
                if (solution[i] > m_lo) {
                    candidatos.push_back(i);    
                }
            }
            if (candidatos.empty()) {
                break; // no hay candidatos para ajustar, salimos del bucle
            }
            // dividir la dif restando un valor
            double decremento = abs(dif) / candidatos.size();
            for (int idx : candidatos) {
                double max_posible = solution[idx] - m_lo;
                double a_restar = min(decremento, max_posible);
                solution[idx] -= a_restar;
            }
        }
        // recalculamos la suma para evaluar la condición del while
        sum = 0.0;
        for (double w : solution) sum += w;
    }
    
}

