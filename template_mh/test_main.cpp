#include <iostream>
#include <cmath>
#include <stdexcept>
#include <vector>
#include "portfolio_problem.h"

using namespace std;

// === Definición de colores ANSI ===
const string RESET  = "\033[0m";
const string BOLD   = "\033[1m";
const string RED    = "\033[1;31m";
const string GREEN  = "\033[1;32m";
const string YELLOW = "\033[1;33m";
const string CYAN   = "\033[1;36m";

bool almost_equal(double a, double b, double epsilon = 1e-8) {
    return abs(a - b) < epsilon;
}

// Función parametrizada para testear cualquier conjunto de datos
void run_dataset_tests(const string& name, const string& filepath, int expected_size, double lo, double hi) {
    cout << BOLD << CYAN << "\n==================================================" << RESET << endl;
    cout << BOLD << CYAN << "   TESTEANDO MERCADO: " << name << RESET << endl;
    cout << BOLD << CYAN << "==================================================\n" << RESET << endl;

    cout << YELLOW << "[SETUP] Instanciando el problema..." << RESET << endl;
    PortfolioProblem problem(filepath, lo, hi);
    cout << GREEN << "  [OK] Problema instanciado correctamente.\n" << RESET << endl;

    // --- TEST 1: Dimensiones ---
    cout << BOLD << "TEST 1: Dimensiones del problema" << RESET << endl;
    if (problem.getSolutionSize() == expected_size) {
        cout << GREEN << "  [PASS] Numero de empresas detectadas: " << expected_size << "." << RESET << endl;
    } else {
        throw runtime_error("El numero de empresas no coincide con lo esperado (" + to_string(expected_size) + ").");
    }

    // --- TEST 2: Robustez en Generación ---
    cout << BOLD << "\nTEST 2: Generacion en masa (Test de Robustez)" << RESET << endl;
    int num_tests = 50;
    for (int i = 0; i < num_tests; ++i) {
        tSolution<double> sol = problem.createSolution();
        if (!problem.isValid(sol)) {
            throw runtime_error("La solucion " + to_string(i) + " generada NO es valida.");
        }
    }
    cout << GREEN << "  [PASS] " << num_tests << " soluciones generadas y validadas sin fallos." << RESET << endl;

    // --- TEST 3: Reparacion Extrema (fix) ---
    cout << BOLD << "\nTEST 3: Reparacion Extrema (Romper para arreglar)" << RESET << endl;
    tSolution<double> sol_rota(expected_size, 0.0);
    sol_rota[0] = 5.0;  // Nos pasamos brutalmente del maximo
    sol_rota[1] = -2.0; // Numero negativo ilegal
    // El resto a 0. La suma es 3.0 (ilegal).
    
    problem.fix(sol_rota); // Llamamos al medico
    
    if (problem.isValid(sol_rota)) {
        cout << GREEN << "  [PASS] La funcion fix() logro reparar una solucion criticamente rota." << RESET << endl;
    } else {
        throw runtime_error("La funcion fix() fallo al intentar reparar la solucion extrema.");
    }

    // --- TEST 4: Fitness ---
    cout << BOLD << "\nTEST 4: Evaluacion de Fitness" << RESET << endl;
    tFitness fitness_val = problem.fitness(sol_rota);
    if (!isnan(fitness_val)) {
        cout << GREEN << "  [PASS] Fitness calculado correctamente (Sin NaNs)." << RESET << endl;
    } else {
        throw runtime_error("El calculo de fitness devolvio Not-a-Number (NaN).");
    }
}

void ejecutar_todos_los_tests() {
    cout << BOLD << YELLOW << "\nINICIANDO BATERIA DE PRUEBAS DE INTEGRACION" << RESET << endl;
    
    // 1. IBEX 35
    run_dataset_tests("IBEX 35", "../datos_portfolio_2526/ibex_35.csv", 30, 0.005, 0.08);
    
    // 2. S&P 100
    run_dataset_tests("S&P 100", "../datos_portfolio_2526/syp_100.csv", 97, 0.005, 0.05);
    
    // 3. S&P 500
    run_dataset_tests("S&P 500", "../datos_portfolio_2526/syp_500.csv", 457, 0.005, 0.02);

    cout << BOLD << GREEN << "\n**************************************************" << RESET << endl;
    cout << BOLD << GREEN << "  ENHORABUENA: TODOS LOS MERCADOS SUPERARON LOS TESTS" << RESET << endl;
    cout << BOLD << GREEN << "**************************************************\n" << RESET << endl;
}

int main() {
    try {
        ejecutar_todos_los_tests();
    } catch (const exception& e) {
        cout << "\n" << BOLD << RED << "[FAILED] CRASH DURANTE LOS TESTS:" << RESET << endl;
        cout << RED << " -> " << e.what() << RESET << "\n" << endl;
        return 1; 
    }
    return 0; 
}