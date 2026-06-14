/**
 * @file testlca.cc
 * @brief Implementación del Liver Cancer Algorithm (LCA, 2023) integrado con CEC 2017.
 * 
 * Este archivo contiene la implementación del algoritmo bio-inspirado LCA
 * propuesto por Essam H. Houssein et al. (2023) para la resolución de las
 * 30 funciones de prueba del benchmark CEC 2017.
 * 
 * Asignatura: Metaheurísticas (MH)
 */

extern "C" {
#include "cec17.h"
}

#include <iostream>
#include <vector>
#include <random>
#include <cmath>
#include <algorithm>
#include <cassert>
#include <iomanip>

using namespace std;

// Rango de búsqueda para el benchmark CEC 2017
const double LOWER_BOUND = -100.0;
const double UPPER_BOUND = 100.0;

/**
 * @brief Limita los valores de una solución a los rangos permitidos [lower, upper].
 */
void clip(vector<double> &sol, double lower, double upper) {
  for (auto &val : sol) {
    if (val < lower) {
      val = lower;
    } else if (val > upper) {
      val = upper;
    }
  }
}

/**
 * @brief Evalúa una solución y lleva el control del presupuesto de evaluaciones.
 */
double evaluate_solution(vector<double> &sol, int &evals, int max_evals, double lower, double upper) {
  clip(sol, lower, upper);
  if (evals >= max_evals) {
    return 1e99; // Retornar fitness muy alto si se agota el presupuesto
  }
  double fit = cec17_fitness(&sol[0]);
  evals += 1;
  return fit;
}

/**
 * @brief Inicializa la población utilizando Random Opposition-Based Learning (ROBL).
 * 
 * ROBL genera soluciones aleatorias y sus opuestas para maximizar la diversidad
 * inicial en el espacio de búsqueda. Luego selecciona la mejor de cada par.
 */
void initialize_population_robl(vector<vector<double>> &pop, vector<double> &fitness, 
                                 int N, int dim, int &evals, int max_evals, std::mt19937 &gen) {
  std::uniform_real_distribution<double> dist_search(LOWER_BOUND, UPPER_BOUND);
  std::uniform_real_distribution<double> dist_01(0.0, 1.0);

  for (int i = 0; i < N; ++i) {
    vector<double> x(dim);
    vector<double> x_opp(dim);

    for (int j = 0; j < dim; ++j) {
      x[j] = dist_search(gen);
      // ROBL: X_opp = lb + ub - rand * X. Como lb + ub = 0 en [-100, 100], simplifica a -rand * X
      x_opp[j] = LOWER_BOUND + UPPER_BOUND - dist_01(gen) * x[j];
    }

    double f_x = evaluate_solution(x, evals, max_evals, LOWER_BOUND, UPPER_BOUND);
    double f_opp = evaluate_solution(x_opp, evals, max_evals, LOWER_BOUND, UPPER_BOUND);

    if (f_opp < f_x) {
      pop[i] = x_opp;
      fitness[i] = f_opp;
    } else {
      pop[i] = x;
      fitness[i] = f_x;
    }
  }
}

/**
 * @brief Genera un paso de Vuelo de Lévy usando el algoritmo de Mantegna.
 */
void generate_levy_flight(vector<double> &lf, int dim, double sigma, std::mt19937 &gen) {
  std::normal_distribution<double> rand_normal(0.0, 1.0);
  const double beta = 1.5;

  for (int j = 0; j < dim; ++j) {
    double u = rand_normal(gen) * sigma;
    double v = rand_normal(gen);
    lf[j] = 0.01 * u / std::pow(std::abs(v), 1.0 / beta);
  }
}

/**
 * @brief Ejecuta el Liver Cancer Algorithm (LCA) para una función y dimensión dadas.
 */
double run_lca(int funcid, int dim, int seed, bool print_progress) {
  std::mt19937 gen(seed);
  std::uniform_real_distribution<double> dist_01(0.0, 1.0);

  // Parámetros del algoritmo
  const int N = 30; // Tamaño de la población (tumores)
  const int max_evals = 10000 * dim; // Presupuesto de evaluaciones de la CEC

  // Inicializar API de la CEC 2017
  cec17_init("lca", funcid, dim);
  if (print_progress) {
    cec17_print_output(); // Imprime hitos por pantalla si se desea depurar
  }

  // Pre-cálculo del parámetro Sigma para el Vuelo de Lévy (Mantegna)
  const double beta = 1.5;
  double num = std::tgamma(1.0 + beta) * std::sin(M_PI * beta / 2.0);
  double den = std::tgamma((1.0 + beta) / 2.0) * beta * std::pow(2.0, (beta - 1.0) / 2.0);
  double sigma = std::pow(num / den, 1.0 / beta);

  // Estructuras de la población
  vector<vector<double>> pop(N, vector<double>(dim));
  vector<double> fitness(N, 1e99);

  int evals = 0;

  // 1. Inicialización con ROBL
  initialize_population_robl(pop, fitness, N, dim, evals, max_evals, gen);

  // Encontrar el mejor tumor inicial
  int best_idx = 0;
  for (int i = 1; i < N; ++i) {
    if (fitness[i] < fitness[best_idx]) {
      best_idx = i;
    }
  }
  vector<double> best_sol = pop[best_idx];
  double best_fitness = fitness[best_idx];

  // Estimar el número total de iteraciones máximas aproximadas
  // Cada iteración evalúa hasta 1 (crecimiento/levy) + 3 (mutación y cruce) = 4 evaluaciones por tumor
  // Por tanto, un límite conservador de iteraciones es max_evals / (N * 4)
  const int max_iterations = max_evals / (N * 4);
  int t = 0;

  // 2. Bucle principal de búsqueda
  while (evals < max_evals) {
    // Parámetro zeta dinámico lineal creciente para la tasa de mutación/metástasis (Eq 12)
    double zeta = std::min(1.0, static_cast<double>(t) / max_iterations);

    for (int i = 0; i < N; ++i) {
      if (evals >= max_evals) break;

      vector<double> candidate(dim);

      // --- FASES DE EXPLORACIÓN / EXPLOTACIÓN ---
      if (dist_01(gen) < 0.8) {
        // A. Explotación: Crecimiento Adaptativo (Hemi-elipsoide)
        // Se calcula el volumen de crecimiento V con Eq (2)
        double l = dist_01(gen);
        double w = dist_01(gen);
        double V = (M_PI / 6.0) * 1.0 * std::pow(l * w, 1.5);

        for (int j = 0; j < dim; ++j) {
          double r = dist_01(gen);
          double PG = r * V; // Crecimiento (Eq 3)
          candidate[j] = pop[i][j] + PG; // Actualización local (Eq 6)
        }
      } else {
        // B. Exploración: Vuelo de Lévy
        vector<double> lf(dim);
        generate_levy_flight(lf, dim, sigma, gen);

        for (int j = 0; j < dim; ++j) {
          double S = dist_01(gen);
          candidate[j] = pop[i][j] + S * lf[j]; // Salto global (Eq 7/8)
        }
      }

      // Evaluar la posición tentativa (Crecimiento o Lévy)
      double cand_fit = evaluate_solution(candidate, evals, max_evals, LOWER_BOUND, UPPER_BOUND);

      // Criterio de aceptación greedy inicial (Eq 8)
      if (cand_fit < fitness[i]) {
        pop[i] = candidate;
        fitness[i] = cand_fit;
      }

      // --- OPERADORES GENÉTICOS (METÁSTASIS) ---
      vector<double> y_mut = pop[i];
      vector<double> z_mut = pop[i];

      // Mutación adaptativa controlada por zeta (Eq 11 y 12)
      for (int j = 0; j < dim; ++j) {
        double r1 = dist_01(gen);
        double r2 = dist_01(gen);

        if (r1 < zeta) {
          // Mutación de tipo Y: distancia al mejor
          double diff = std::abs(pop[i][j] - best_sol[j]);
          y_mut[j] = diff * dist_01(gen);
        }
        if (r2 < zeta) {
          // Mutación de tipo Z
          double diff = std::abs(pop[i][j] - best_sol[j]);
          z_mut[j] = diff - dist_01(gen);
        }
      }

      // Cruce (Crossover) (Eq 13)
      double tau = dist_01(gen);
      double tau_prime;
      do {
        tau_prime = dist_01(gen);
      } while (std::abs(tau - tau_prime) < 1e-4);

      vector<double> w_cross(dim);
      for (int j = 0; j < dim; ++j) {
        w_cross[j] = tau * y_mut[j] + (1.0 - tau_prime) * z_mut[j];
      }

      // Evaluar candidatos metastásicos
      double f_y_mut = evaluate_solution(y_mut, evals, max_evals, LOWER_BOUND, UPPER_BOUND);
      double f_z_mut = evaluate_solution(z_mut, evals, max_evals, LOWER_BOUND, UPPER_BOUND);
      double f_w_cross = evaluate_solution(w_cross, evals, max_evals, LOWER_BOUND, UPPER_BOUND);

      // Selección greedy final (Eq 14)
      double best_metastasis_fit = fitness[i];
      vector<double> best_metastasis_sol = pop[i];

      if (f_y_mut < best_metastasis_fit) {
        best_metastasis_fit = f_y_mut;
        best_metastasis_sol = y_mut;
      }
      if (f_z_mut < best_metastasis_fit) {
        best_metastasis_fit = f_z_mut;
        best_metastasis_sol = z_mut;
      }
      if (f_w_cross < best_metastasis_fit) {
        best_metastasis_fit = f_w_cross;
        best_metastasis_sol = w_cross;
      }

      // Reemplazar solución actual si la descendencia/metástasis es mejor
      pop[i] = best_metastasis_sol;
      fitness[i] = best_metastasis_fit;

      // Actualizar el mejor global
      if (fitness[i] < best_fitness) {
        best_fitness = fitness[i];
        best_sol = pop[i];
      }
    }
    t++;
  }

  return best_fitness;
}

int main() {
  const vector<int> dims = {10, 30, 50};
  const int NRUNS = 31; // ejecuciones independientes con semillas distintas

  for (int dim : dims) {
    cout << "==========================================================" << endl;
    cout << " LCA en CEC 2017 (D=" << dim << ", " << NRUNS << " runs)" << endl;
    cout << "==========================================================" << endl;

    for (int funcid = 1; funcid <= 30; funcid++) {
      double sum_err = 0.0;
      for (int run = 0; run < NRUNS; run++) {
        int seed = run + 1; // semillas 1..NRUNS
        double best_fitness = run_lca(funcid, dim, seed, false);
        sum_err += cec17_error(best_fitness);
      }
      cout << "F" << setw(2) << funcid << " media: "
           << scientific << setprecision(6) << (sum_err / NRUNS) << endl;
    }
  }

  return 0;
}
