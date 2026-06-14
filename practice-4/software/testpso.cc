/**
 * @file testpso.cc
 * @brief Optimizacion por Enjambre de Particulas (PSO) de referencia para CEC 2017.
 *
 * Version con coeficientes de constriccion (Clerc & Kennedy). Linea base de
 * comparacion frente al LCA. Mismo protocolo: 10000*D evaluaciones,
 * rango [-100, 100], 31 ejecuciones independientes.
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

double run_pso(int funcid, int dim, int seed) {
  std::mt19937 gen(seed);
  std::uniform_real_distribution<double> dist_search(LOWER_BOUND, UPPER_BOUND);
  std::uniform_real_distribution<double> dist_01(0.0, 1.0);

  const int N = 50;                 // tamano del enjambre
  const double w = 0.7298;          // peso de inercia (constriccion)
  const double c1 = 1.49618;        // componente cognitiva
  const double c2 = 1.49618;        // componente social
  const double vmax = 0.2 * (UPPER_BOUND - LOWER_BOUND);
  const int max_evals = 10000 * dim;

  cec17_init("pso", funcid, dim);

  vector<vector<double>> pos(N, vector<double>(dim));
  vector<vector<double>> vel(N, vector<double>(dim));
  vector<vector<double>> pbest(N, vector<double>(dim));
  vector<double> pbest_fit(N, 1e99);

  std::uniform_real_distribution<double> dist_vel(-vmax, vmax);

  int evals = 0;
  vector<double> gbest(dim);
  double gbest_fit = 1e99;

  for (int i = 0; i < N; ++i) {
    for (int j = 0; j < dim; ++j) {
      pos[i][j] = dist_search(gen);
      vel[i][j] = dist_vel(gen);
    }
    double f = evaluate_solution(pos[i], evals, max_evals, LOWER_BOUND, UPPER_BOUND);
    pbest[i] = pos[i];
    pbest_fit[i] = f;
    if (f < gbest_fit) { gbest_fit = f; gbest = pos[i]; }
  }

  while (evals < max_evals) {
    for (int i = 0; i < N && evals < max_evals; ++i) {
      for (int j = 0; j < dim; ++j) {
        double r1 = dist_01(gen);
        double r2 = dist_01(gen);
        vel[i][j] = w * vel[i][j]
                  + c1 * r1 * (pbest[i][j] - pos[i][j])
                  + c2 * r2 * (gbest[j] - pos[i][j]);
        if (vel[i][j] > vmax) vel[i][j] = vmax;
        else if (vel[i][j] < -vmax) vel[i][j] = -vmax;
        pos[i][j] += vel[i][j];
      }

      double f = evaluate_solution(pos[i], evals, max_evals, LOWER_BOUND, UPPER_BOUND);

      if (f < pbest_fit[i]) {
        pbest_fit[i] = f;
        pbest[i] = pos[i];
        if (f < gbest_fit) { gbest_fit = f; gbest = pos[i]; }
      }
    }
  }

  return gbest_fit;
}

int main() {
  const vector<int> dims = {10, 30, 50};
  const int NRUNS = 31;

  for (int dim : dims) {
    cout << "==========================================================" << endl;
    cout << " PSO (constriccion) en CEC 2017 (D=" << dim << ", " << NRUNS << " runs)" << endl;
    cout << "==========================================================" << endl;

    for (int funcid = 1; funcid <= 30; funcid++) {
      double sum_err = 0.0;
      for (int run = 0; run < NRUNS; run++) {
        int seed = run + 1;
        double best_fitness = run_pso(funcid, dim, seed);
        sum_err += cec17_error(best_fitness);
      }
      cout << "F" << setw(2) << funcid << " media: "
           << scientific << setprecision(6) << (sum_err / NRUNS) << endl;
    }
  }

  return 0;
}
