/**
 * @file testlca_portfolio.cpp
 * @brief Aplicacion del Liver Cancer Algorithm (LCA) al problema del Portfolio
 *        (Markowitz) de las Practicas 1-3, para comparar con sus resultados.
 *
 * Reutiliza el framework de la Practica 1 (PortfolioProblem, fitness/fix/
 * createSolution) y replica su protocolo experimental: 50 ejecuciones, 10000
 * evaluaciones, lambda=500, semilla base 42, sobre IBEX35 / S&P100 / S&P500.
 *
 * El LCA conserva sus operadores (crecimiento hemi-elipsoidal 80%, vuelo de Levy
 * 20%, metastasis) y, al ser un problema con restricciones (suma=1, rango,
 * cardinalidad), llama a problem.fix() tras inicializar y tras generar cada
 * candidato. El problema se MAXIMIZA (igual que las BL/ES/ILS de las practicas).
 */

#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <chrono>
#include <iomanip>
#include <random>

#include "problem.h"
#include "mh.h"
#include "portfolio_problem.h"
#include "random.hpp"

using namespace std;
using namespace std::chrono;

// ---------------------------------------------------------------------------
// LCA como metaheuristica MH<double> (maximizacion).
// ---------------------------------------------------------------------------
class LCA_Portfolio : public MH<double> {
public:
  ResultMH<double> optimize(Problem<double> &problem, int maxevals) override {
    const int N = 30;
    const int dim = (int)problem.getSolutionSize();
    auto rango = problem.getSolutionDomainRange();
    (void)rango;

    // mt19937 para los operadores del LCA (Random global lo usan fix/createSolution)
    static thread_local std::mt19937 gen(12345);
    std::uniform_real_distribution<double> u01(0.0, 1.0);
    std::normal_distribution<double> ndist(0.0, 1.0);

    // sigma de Mantegna (beta=1.5) para el vuelo de Levy
    const double beta = 1.5;
    double num = std::tgamma(1.0 + beta) * std::sin(M_PI * beta / 2.0);
    double den = std::tgamma((1.0 + beta) / 2.0) * beta *
                 std::pow(2.0, (beta - 1.0) / 2.0);
    double sigma = std::pow(num / den, 1.0 / beta);

    int evals = 0;
    vector<tSolution<double>> pop(N);
    vector<tFitness> fit(N);
    for (int i = 0; i < N; ++i) {
      pop[i] = problem.createSolution();   // ya factible (createSolution -> fix)
      fit[i] = problem.fitness(pop[i]);
      evals++;
    }

    int best_idx = 0;
    for (int i = 1; i < N; ++i)
      if (fit[i] > fit[best_idx]) best_idx = i;   // MAXIMIZAR
    tSolution<double> best_sol = pop[best_idx];
    tFitness best_fit = fit[best_idx];

    const int max_iterations = maxevals / (N * 4);
    int t = 0;

    while (evals < maxevals) {
      double zeta = std::min(1.0, (double)t / std::max(1, max_iterations));

      for (int i = 0; i < N && evals < maxevals; ++i) {
        tSolution<double> cand(dim);

        if (u01(gen) < 0.8) {
          // Crecimiento hemi-elipsoidal (explotacion)
          double l = u01(gen), w = u01(gen);
          double V = (M_PI / 6.0) * std::pow(l * w, 1.5);
          for (int j = 0; j < dim; ++j)
            cand[j] = pop[i][j] + u01(gen) * V;
        } else {
          // Vuelo de Levy (exploracion)
          for (int j = 0; j < dim; ++j) {
            double uu = ndist(gen) * sigma;
            double vv = ndist(gen);
            double lf = 0.01 * uu / std::pow(std::abs(vv), 1.0 / beta);
            cand[j] = pop[i][j] + u01(gen) * lf;
          }
        }
        problem.fix(cand);
        tFitness cf = problem.fitness(cand); evals++;
        if (cf > fit[i]) { pop[i] = cand; fit[i] = cf; }   // MAXIMIZAR

        if (evals >= maxevals) { if (fit[i] > best_fit) { best_fit = fit[i]; best_sol = pop[i]; } break; }

        // Metastasis: mutaciones y/z + cruce, controladas por zeta
        tSolution<double> y = pop[i], z = pop[i];
        for (int j = 0; j < dim; ++j) {
          if (u01(gen) < zeta) { double d = std::abs(pop[i][j] - best_sol[j]); y[j] = d * u01(gen); }
          if (u01(gen) < zeta) { double d = std::abs(pop[i][j] - best_sol[j]); z[j] = d - u01(gen); }
        }
        double tau = u01(gen), taup;
        do { taup = u01(gen); } while (std::abs(tau - taup) < 1e-4);
        tSolution<double> wc(dim);
        for (int j = 0; j < dim; ++j) wc[j] = tau * y[j] + (1.0 - taup) * z[j];

        problem.fix(y); problem.fix(z); problem.fix(wc);
        tFitness fy = problem.fitness(y); evals++;
        tFitness fz = (evals < maxevals) ? (evals++, problem.fitness(z)) : fit[i];
        tFitness fw = (evals < maxevals) ? (evals++, problem.fitness(wc)) : fit[i];

        tFitness bmf = fit[i]; tSolution<double> bms = pop[i];
        if (fy > bmf) { bmf = fy; bms = y; }
        if (fz > bmf) { bmf = fz; bms = z; }
        if (fw > bmf) { bmf = fw; bms = wc; }
        pop[i] = bms; fit[i] = bmf;

        if (fit[i] > best_fit) { best_fit = fit[i]; best_sol = pop[i]; }
      }
      t++;
    }
    return ResultMH<double>(best_sol, best_fit, (unsigned)evals);
  }
};

struct Mercado { string nombre, ruta; double lo, hi; };

int main(int argc, char **argv) {
  const double LAMBDA = 500.0;
  const int MAX_EVALS = 10000;
  const int NUM_RUNS = 50;
  const long base_seed = 42;

  vector<Mercado> mercados = {
    {"IBEX 35", "datos_portfolio_2526/ibex_35.csv", 0.005, 0.08},
    {"S&P 100", "datos_portfolio_2526/syp_100.csv", 0.005, 0.05},
    {"S&P 500", "datos_portfolio_2526/syp_500.csv", 0.005, 0.02},
  };
  if (argc > 1) mercados = {mercados[atoi(argv[1])]};

  cout << fixed << setprecision(3);
  cout << "Algoritmo & Fitness(2015-2024) & Fitness(2025) & Beneficio(2025) & Std & Evaluaciones & Tiempo(s)\\\\\n";

  LCA_Portfolio alg;
  for (auto &m : mercados) {
    PortfolioProblem train(m.ruta, m.lo, m.hi, false, LAMBDA);
    PortfolioProblem test(m.ruta, m.lo, m.hi, true, LAMBDA);

    double s_train = 0, s_test = 0, s_ben = 0, s_time = 0, s_evals = 0;
    vector<double> hist;
    for (int r = 0; r < NUM_RUNS; ++r) {
      Random::seed(base_seed + r);
      auto t0 = high_resolution_clock::now();
      auto res = alg.optimize(train, MAX_EVALS);
      auto t1 = high_resolution_clock::now();
      double ft = test.fitness(res.solution);
      double bn = test.getBeneficio(res.solution);
      s_train += res.fitness; s_test += ft; s_ben += bn;
      s_time += duration<double>(t1 - t0).count(); s_evals += res.evaluations;
      hist.push_back(res.fitness);
    }
    double mt = s_train / NUM_RUNS;
    double var = 0; for (double f : hist) var += (f - mt) * (f - mt);
    double sd = std::sqrt(var / NUM_RUNS);
    cout << "LCA (" << m.nombre << ") & " << mt << " & " << (s_test/NUM_RUNS)
         << " & " << (s_ben/NUM_RUNS) << " & " << sd << " & "
         << (long)(s_evals/NUM_RUNS) << " & " << (s_time/NUM_RUNS) << " \\\\\n";
  }
  return 0;
}
