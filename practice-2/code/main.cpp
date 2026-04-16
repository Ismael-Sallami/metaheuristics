/**
 * @file main.cpp
 * @brief Lab for testing Portfolio Optimization algorithms.
 * @details Runs three algorithms on historical data (2015-2024) and tests on 2025.
 */

#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <chrono>
#include <iomanip>
#include <stdexcept>
#include <fstream>

// Config reader
#include "config_reader.h"

// Problem and utilities
#include "portfolio_problem.h"
#include "random.hpp"

// Algorithms
#include "randomsearch.h"
#include "greedy.h"
#include "localsearch.h"
#include "localsearch_best.h"
#include "localsearch_multistart.h"

using namespace std;
using namespace std::chrono;

// Data Structures

// Info about a stock market
struct Mercado
{
    string nombre;
    string ruta_csv;
    double limite_inferior; // Min investment
    double limite_superior; // Max investment
};

// Results from an algorithm run
struct Estadisticas
{
    double media_fitness_train;  // Average fitness on 2015-2024
    double media_fitness_test;   // Average fitness on 2025
    double media_beneficio_test; // Actual profit on 2025
    double desviacion_tipica;    // Standard deviation on training
    double tiempo_medio_segundos;
    double evaluaciones_medias;
    vector<double> historico_fitness; // All fitness values for boxplots
};


// Run a stochastic algorithm multiple times
Estadisticas ejecutar_experimento_estocastico(MH<double> &algoritmo, PortfolioProblem &prob_train,
                                              PortfolioProblem &prob_test, int num_ejecuciones,
                                              int max_evals, long int base_seed)
{
    Estadisticas stats = {0, 0, 0, 0, 0, 0, {}};
    stats.historico_fitness.reserve(num_ejecuciones);

    for (int i = 0; i < num_ejecuciones; ++i)
    {
        Random::seed(base_seed + i);

        auto inicio = high_resolution_clock::now();
        // Train on 2015-2024 data
        auto resultado = algoritmo.optimize(prob_train, max_evals);
        auto fin = high_resolution_clock::now();

        // Test weights on 2025
        double fitness_2025 = prob_test.fitness(resultado.solution);
        double beneficio_2025 = prob_test.getBeneficio(resultado.solution);

        // Accumulate metrics
        stats.media_fitness_train += resultado.fitness;
        stats.media_fitness_test += fitness_2025;
        stats.media_beneficio_test += beneficio_2025;
        stats.tiempo_medio_segundos += duration<double>(fin - inicio).count();
        stats.evaluaciones_medias += resultado.evaluations;

        stats.historico_fitness.push_back(resultado.fitness);
    }

    // Calculate averages
    stats.media_fitness_train /= num_ejecuciones;
    stats.media_fitness_test /= num_ejecuciones;
    stats.media_beneficio_test /= num_ejecuciones;
    stats.tiempo_medio_segundos /= num_ejecuciones;
    stats.evaluaciones_medias /= num_ejecuciones;

    // Calculate standard deviation
    double varianza = 0.0;
    for (double f : stats.historico_fitness)
    {
        varianza += pow(f - stats.media_fitness_train, 2);
    }
    stats.desviacion_tipica = sqrt(varianza / num_ejecuciones);

    return stats;
}

// Run a deterministic algorithm once
Estadisticas ejecutar_experimento_determinista(MH<double> &algoritmo, PortfolioProblem &prob_train,
                                               PortfolioProblem &prob_test)
{
    Estadisticas stats = {0, 0, 0, 0, 0, 0, {}};

    auto inicio = high_resolution_clock::now();
    auto resultado = algoritmo.optimize(prob_train, 1);
    auto fin = high_resolution_clock::now();

    // Test weights on 2025
    double fitness_2025 = prob_test.fitness(resultado.solution);
    double beneficio_2025 = prob_test.getBeneficio(resultado.solution);

    stats.media_fitness_train = resultado.fitness;
    stats.media_fitness_test = fitness_2025;
    stats.media_beneficio_test = beneficio_2025;
    stats.tiempo_medio_segundos = duration<double>(fin - inicio).count();
    stats.evaluaciones_medias = 1.0;
    stats.desviacion_tipica = 0.0;

    stats.historico_fitness.push_back(resultado.fitness);

    return stats;
}


int main(int argc, char *argv[])
{
    // Load config
    Config config = ConfigReader::read("config.cfg");

    // Allow seed override from command line
    if (argc > 1)
    {
        config.seed = atoi(argv[1]);
    }

    vector<Mercado> mercados;

    // Setup markets
    if (config.use_custom_market == 1)
    {
        cout << "\n [INFO] CUSTOM mode ON. Running only config market.\n";
        mercados.push_back({config.custom_name,
                            config.custom_path,
                            config.custom_lo,
                            config.custom_hi});
    }
    else
    {
        // Default 3 markets
        mercados = {
            {"IBEX 35", "datos_portfolio_2526/ibex_35.csv", 0.005, 0.08},
            {"S&P 100", "datos_portfolio_2526/syp_100.csv", 0.005, 0.05},
            {"S&P 500", "datos_portfolio_2526/syp_500.csv", 0.005, 0.02}};
    }

    RandomSearch<double> alg_random;
    GreedySearch alg_greedy;
    LocalSearch alg_local(config.ls_ratio);
    LocalSearchBest alg_local_best(config.ls_ratio);
    LocalSearchMultiStart alg_local_multi(config.ls_ratio);

    const int MAX_EVALUACIONES = 10000;
    const int NUM_EJECUCIONES = 50;

    cout << "\n===================================================================" << endl;
    cout << "   PORTFOLIO OPTIMIZATION ENGINE (MARKOWITZ)                      " << endl;
    cout << "   Train (2015-2024) vs Test (2025)                               " << endl;
    cout << "   [PARAMS] Seed: " << config.seed
         << " | Lambda: " << config.lambda
         << " | LS_Ratio: " << config.ls_ratio << endl;
    cout << "   [LIMITS] Evals: " << config.max_evaluaciones
         << " | Runs: " << config.num_ejecuciones << endl;
    cout << "===================================================================\n"
         << endl;

    for (const auto &mercado : mercados)
    {
        try
        {
            // Create two problems: train and test
            PortfolioProblem prob_train(mercado.ruta_csv, mercado.limite_inferior, mercado.limite_superior, false, config.lambda);
            PortfolioProblem prob_test(mercado.ruta_csv, mercado.limite_inferior, mercado.limite_superior, true, config.lambda);

            Estadisticas stats_gr = ejecutar_experimento_determinista(alg_greedy, prob_train, prob_test);
            Estadisticas stats_rs = ejecutar_experimento_estocastico(alg_random, prob_train, prob_test, NUM_EJECUCIONES, MAX_EVALUACIONES, config.seed);
            Estadisticas stats_ls = ejecutar_experimento_estocastico(alg_local, prob_train, prob_test, NUM_EJECUCIONES, MAX_EVALUACIONES, config.seed);
            Estadisticas stats_ls_best = ejecutar_experimento_estocastico(alg_local_best, prob_train, prob_test, NUM_EJECUCIONES, MAX_EVALUACIONES, config.seed);
            Estadisticas stats_ls_multi = ejecutar_experimento_estocastico(alg_local_multi, prob_train, prob_test, NUM_EJECUCIONES, MAX_EVALUACIONES, config.seed);

            // Print results table
            cout << "\nResults: " << mercado.nombre << " (Averages)\n";
            cout << string(95, '-') << "\n";
            cout << left << setw(15) << "Algorithm"
                 << right << setw(15) << "2015-2024"
                 << setw(25) << "2025"
                 << setw(18) << "Evaluations"
                 << setw(20) << "Time (s)" << "\n";

            cout << left << setw(15) << ""
                 << right << setw(15) << "Fitness"
                 << setw(12) << "Fitness" << setw(13) << "Profit"
                 << setw(18) << "" << setw(20) << "" << "\n";
            cout << string(95, '-') << "\n";

            cout << fixed << setprecision(3);

            // Greedy row
            cout << left << setw(15) << "Greedy"
                 << right << setw(15) << stats_gr.media_fitness_train
                 << setw(12) << stats_gr.media_fitness_test
                 << setw(13) << stats_gr.media_beneficio_test
                 << setw(18) << (int)stats_gr.evaluaciones_medias
                 << setw(20) << stats_gr.tiempo_medio_segundos << "\n";

            // Random row
            cout << left << setw(15) << "Random"
                 << right << setw(15) << stats_rs.media_fitness_train
                 << setw(12) << stats_rs.media_fitness_test
                 << setw(13) << stats_rs.media_beneficio_test
                 << setw(18) << (int)stats_rs.evaluaciones_medias
                 << setw(20) << stats_rs.tiempo_medio_segundos << "\n";

            // Local Search row
            cout << left << setw(15) << "BL"
                 << right << setw(15) << stats_ls.media_fitness_train
                 << setw(12) << stats_ls.media_fitness_test
                 << setw(13) << stats_ls.media_beneficio_test
                 << setw(18) << (int)stats_ls.evaluaciones_medias
                 << setw(20) << stats_ls.tiempo_medio_segundos << "\n";

            // Best Local Search row
            cout << left << setw(15) << "BL Best"
                 << right << setw(15) << stats_ls_best.media_fitness_train
                 << setw(12) << stats_ls_best.media_fitness_test
                 << setw(13) << stats_ls_best.media_beneficio_test
                 << setw(18) << (int)stats_ls_best.evaluaciones_medias
                 << setw(20) << stats_ls_best.tiempo_medio_segundos << "\n";

            cout << left << setw(15) << "BL Multi"
                 << right << setw(15) << stats_ls_multi.media_fitness_train
                 << setw(12) << stats_ls_multi.media_fitness_test
                 << setw(13) << stats_ls_multi.media_beneficio_test
                 << setw(18) << (int)stats_ls_multi.evaluaciones_medias
                 << setw(20) << stats_ls_multi.tiempo_medio_segundos << "\n";

            cout << string(95, '-') << "\n";

            // Save results to CSV
            string nombre_limpio = mercado.nombre;
            for (char &c : nombre_limpio)
                if (c == ' ' || c == '&')
                    c = '_';
            string nombre_archivo = nombre_limpio + "_resultados.csv";

            ofstream archivo_csv(nombre_archivo);
            if (archivo_csv.is_open())
            {
                archivo_csv << "alg,fitness\n";
                for (double f : stats_rs.historico_fitness)
                    archivo_csv << "RandomSearch," << f << "\n";
                for (double f : stats_gr.historico_fitness)
                    archivo_csv << "GreedySearch," << f << "\n";
                for (double f : stats_ls.historico_fitness)
                    archivo_csv << "LocalSearch," << f << "\n";
                for (double f : stats_ls_best.historico_fitness)
                    archivo_csv << "LocalSearchBest," << f << "\n";
                for (double f : stats_ls_multi.historico_fitness)
                    archivo_csv << "LocalSearchMulti," << f << "\n";
                archivo_csv.close();
            }
            else
            {
                cout << " [WARNING] Could not create CSV for " << mercado.nombre << "\n";
            }
        }
        catch (const exception &e)
        {
            cout << "\n [ERROR] Failed to process " << mercado.nombre << ": " << e.what() << "\n\n";
        }
    }

    cout << "\n>>> Done. Results saved to CSV files." << endl;
    return 0;
}
