/**
 * @file testde.cc
 * @brief Evolucion Diferencial (DE/rand/1/bin) de referencia para CEC 2017.
 *
 * Algoritmo clasico de Storn y Price (1997) usado como linea base de
 * comparacion frente al LCA. Mismo protocolo experimental que el resto de
 * algoritmos: 10000*D evaluaciones, rango [-100, 100], 31 ejecuciones.
 */

extern "C" {
#include "cec17.h"
}

#include <iostream>
#include <vector>
#include <random>
#include <cmath>
#include <algorithm>
#include <iomanip>

using namespace std;

const double LOWER_BOUND = -100.0;
const double UPPER_BOUND = 100.0;

void clip(vector<double> &sol, double lower, double upper) {
  for (auto &val : sol) {
    if (val < lower) val = lower;
    else if (val > upper) val = upper;
  }
}

double evaluate_solution(vector<double> &sol, int &evals, int max_evals, double lower, double upper) {
  clip(sol, lower, upper);
  if (evals >= max_evals) {
    return 1e99;
  }
  double fit = cec17_fitness(&sol[0]);
  evals += 1;
  return fit;
}

double run_de(int funcid, int dim, int seed) {
  std::mt19937 gen(seed);
  std::uniform_real_distribution<double> dist_search(LOWER_BOUND, UPPER_BOUND);
  std::uniform_real_distribution<double> dist_01(0.0, 1.0);
  std::uniform_int_distribution<int> dist_dim(0, dim - 1);

  const int N = 50;            // tamano de poblacion
  const double F = 0.5;        // factor de escala
  const double CR = 0.9;       // probabilidad de cruce
  const int max_evals = 10000 * dim;

  cec17_init("de", funcid, dim);
  std::uniform_int_distribution<int> dist_pop(0, N - 1);

  vector<vector<double>> pop(N, vector<double>(dim));
  vector<double> fitness(N, 1e99);

  int evals = 0;
  for (int i = 0; i < N; ++i) {
    for (int j = 0; j < dim; ++j) pop[i][j] = dist_search(gen);
    fitness[i] = evaluate_solution(pop[i], evals, max_evals, LOWER_BOUND, UPPER_BOUND);
  }

  int best_idx = 0;
  for (int i = 1; i < N; ++i)
    if (fitness[i] < fitness[best_idx]) best_idx = i;
  double best_fitness = fitness[best_idx];

  while (evals < max_evals) {
    for (int i = 0; i < N && evals < max_evals; ++i) {
      // Seleccion de tres indices distintos r1, r2, r3 != i
      int r1, r2, r3;
      do { r1 = dist_pop(gen); } while (r1 == i);
      do { r2 = dist_pop(gen); } while (r2 == i || r2 == r1);
      do { r3 = dist_pop(gen); } while (r3 == i || r3 == r1 || r3 == r2);

      // Mutacion DE/rand/1 y cruce binomial
      vector<double> trial(dim);
      int jrand = dist_dim(gen);
      for (int j = 0; j < dim; ++j) {
        if (dist_01(gen) < CR || j == jrand) {
          trial[j] = pop[r1][j] + F * (pop[r2][j] - pop[r3][j]);
        } else {
          trial[j] = pop[i][j];
        }
      }

      double trial_fit = evaluate_solution(trial, evals, max_evals, LOWER_BOUND, UPPER_BOUND);

      // Seleccion greedy
      if (trial_fit <= fitness[i]) {
        pop[i] = trial;
        fitness[i] = trial_fit;
        if (trial_fit < best_fitness) best_fitness = trial_fit;
      }
    }
  }

  return best_fitness;
}

int main() {
  const vector<int> dims = {10, 30, 50};
  const int NRUNS = 31;

  for (int dim : dims) {
    cout << "==========================================================" << endl;
    cout << " DE/rand/1/bin en CEC 2017 (D=" << dim << ", " << NRUNS << " runs)" << endl;
    cout << "==========================================================" << endl;

    for (int funcid = 1; funcid <= 30; funcid++) {
      double sum_err = 0.0;
      for (int run = 0; run < NRUNS; run++) {
        int seed = run + 1;
        double best_fitness = run_de(funcid, dim, seed);
        sum_err += cec17_error(best_fitness);
      }
      cout << "F" << setw(2) << funcid << " media: "
           << scientific << setprecision(6) << (sum_err / NRUNS) << endl;
    }
  }

  return 0;
}
