/**
 * @file testlcamulti.cc
 * @brief Implementación del algoritmo LCA Multimodal con Crowding Determinista.
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
#include <numeric>
#include <limits>

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

void initialize_population_robl(vector<vector<double>> &pop, vector<double> &fitness, 
                                 int N, int dim, int &evals, int max_evals, std::mt19937 &gen) {
  std::uniform_real_distribution<double> dist_search(LOWER_BOUND, UPPER_BOUND);
  std::uniform_real_distribution<double> dist_01(0.0, 1.0);

  for (int i = 0; i < N; ++i) {
    vector<double> x(dim);
    vector<double> x_opp(dim);

    for (int j = 0; j < dim; ++j) {
      x[j] = dist_search(gen);
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

void generate_levy_flight(vector<double> &lf, int dim, double sigma, std::mt19937 &gen) {
  std::normal_distribution<double> rand_normal(0.0, 1.0);
  const double beta = 1.5;

  for (int j = 0; j < dim; ++j) {
    double u = rand_normal(gen) * sigma;
    double v = rand_normal(gen);
    lf[j] = 0.01 * u / std::pow(std::abs(v), 1.0 / beta);
  }
}

int crowdingCompetidor(const vector<vector<double>> &pop, const vector<double> &h) {
  int N = pop.size(), dim = h.size();
  int closest = 0;
  double minDist = std::numeric_limits<double>::max();

  for (int i = 0; i < N; i++) {
    double dist = 0.0;
    for (int j = 0; j < dim; j++) {
      double d = h[j] - pop[i][j];
      dist += d * d;
    }
    if (dist < minDist) {
      minDist = dist;
      closest = i;
    }
  }
  return closest;
}

double run_lcamulti(int funcid, int dim, int seed) {
  std::mt19937 gen(seed);
  std::uniform_real_distribution<double> dist_01(0.0, 1.0);

  const int N = 30;
  const int max_evals = 10000 * dim;

  cec17_init("lcamulti", funcid, dim);

  const double beta = 1.5;
  double num = std::tgamma(1.0 + beta) * std::sin(M_PI * beta / 2.0);
  double den = std::tgamma((1.0 + beta) / 2.0) * beta * std::pow(2.0, (beta - 1.0) / 2.0);
  double sigma = std::pow(num / den, 1.0 / beta);

  vector<vector<double>> pop(N, vector<double>(dim));
  vector<double> fitness(N, 1e99);

  int evals = 0;
  initialize_population_robl(pop, fitness, N, dim, evals, max_evals, gen);

  int best_idx = 0;
  for (int i = 1; i < N; ++i) {
    if (fitness[i] < fitness[best_idx]) {
      best_idx = i;
    }
  }
  vector<double> best_sol = pop[best_idx];
  double best_fitness = fitness[best_idx];

  const int max_iterations = max_evals / (N * 4);
  int t = 0;

  while (evals < max_evals) {
    double zeta = std::min(1.0, static_cast<double>(t) / max_iterations);

    for (int i = 0; i < N; ++i) {
      if (evals >= max_evals) break;

      vector<double> candidate(dim);

      if (dist_01(gen) < 0.8) {
        double l = dist_01(gen);
        double w = dist_01(gen);
        double V = (M_PI / 6.0) * 1.0 * std::pow(l * w, 1.5);

        for (int j = 0; j < dim; ++j) {
          double r = dist_01(gen);
          double PG = r * V;
          candidate[j] = pop[i][j] + PG;
        }
      } else {
        vector<double> lf(dim);
        generate_levy_flight(lf, dim, sigma, gen);

        for (int j = 0; j < dim; ++j) {
          double S = dist_01(gen);
          candidate[j] = pop[i][j] + S * lf[j];
        }
      }

      double cand_fit = evaluate_solution(candidate, evals, max_evals, LOWER_BOUND, UPPER_BOUND);

      // Mutación y Cruce para generar el hijo final
      vector<double> y_mut = candidate;
      vector<double> z_mut = candidate;

      for (int j = 0; j < dim; ++j) {
        double r1 = dist_01(gen);
        double r2 = dist_01(gen);

        if (r1 < zeta) {
          double diff = std::abs(candidate[j] - best_sol[j]);
          y_mut[j] = diff * dist_01(gen);
        }
        if (r2 < zeta) {
          double diff = std::abs(candidate[j] - best_sol[j]);
          z_mut[j] = diff - dist_01(gen);
        }
      }

      double tau = dist_01(gen);
      double tau_prime;
      do {
        tau_prime = dist_01(gen);
      } while (std::abs(tau - tau_prime) < 1e-4);

      vector<double> w_cross(dim);
      for (int j = 0; j < dim; ++j) {
        w_cross[j] = tau * y_mut[j] + (1.0 - tau_prime) * z_mut[j];
      }

      double f_y_mut = evaluate_solution(y_mut, evals, max_evals, LOWER_BOUND, UPPER_BOUND);
      double f_z_mut = evaluate_solution(z_mut, evals, max_evals, LOWER_BOUND, UPPER_BOUND);
      double f_w_cross = evaluate_solution(w_cross, evals, max_evals, LOWER_BOUND, UPPER_BOUND);

      double best_hijo_fit = cand_fit;
      vector<double> best_hijo_sol = candidate;

      if (f_y_mut < best_hijo_fit) {
        best_hijo_fit = f_y_mut;
        best_hijo_sol = y_mut;
      }
      if (f_z_mut < best_hijo_fit) {
        best_hijo_fit = f_z_mut;
        best_hijo_sol = z_mut;
      }
      if (f_w_cross < best_hijo_fit) {
        best_hijo_fit = f_w_cross;
        best_hijo_sol = w_cross;
      }

      // Crowding determinista: Compite contra el más similar de la población
      int closest_idx = crowdingCompetidor(pop, best_hijo_sol);
      if (best_hijo_fit < fitness[closest_idx]) {
        pop[closest_idx] = best_hijo_sol;
        fitness[closest_idx] = best_hijo_fit;

        if (best_hijo_fit < best_fitness) {
          best_fitness = best_hijo_fit;
          best_sol = best_hijo_sol;
        }
      }
    }
    t++;
  }

  return best_fitness;
}

int main() {
  const vector<int> dims = {10, 30, 50};
  const int NRUNS = 31;

  for (int dim : dims) {
    cout << "==========================================================" << endl;
    cout << " LCA-Multi en CEC 2017 (D=" << dim << ", " << NRUNS << " runs)" << endl;
    cout << "==========================================================" << endl;

    for (int funcid = 1; funcid <= 30; funcid++) {
      double sum_err = 0.0;
      for (int run = 0; run < NRUNS; run++) {
        int seed = run + 1;
        double best_fitness = run_lcamulti(funcid, dim, seed);
        sum_err += cec17_error(best_fitness);
      }
      cout << "F" << setw(2) << funcid << " media: "
           << scientific << setprecision(6) << (sum_err / NRUNS) << endl;
    }
  }

  return 0;
}
