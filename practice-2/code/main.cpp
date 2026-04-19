/**
 * @file main.cpp
 * @brief Lab for testing Portfolio Optimization algorithms.
 * @details Runs algorithms.
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
#include "memetic_algorithm.h"
#include "genetic_algorithm.h"
#include "memetic_lsch.h"
#include "differential_evolution.h"

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
Estadisticas ejecutar_experimento_estocastico(
    MH<double> &algoritmo,
    PortfolioProblem &prob_train,
    PortfolioProblem &prob_test,
    int num_ejecuciones,
    int max_evals,
    long int base_seed)
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
Estadisticas ejecutar_experimento_determinista(
    MH<double> &algoritmo,
    PortfolioProblem &prob_train,
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
    stats.evaluaciones_medias = resultado.evaluations;
    stats.desviacion_tipica = 0.0;

    stats.historico_fitness.push_back(resultado.fitness);

    return stats;
}

void print_header()
{
    cout << string(121, '-') << "\n";
    cout << left << setw(16) << "Algoritmo"
         << right << setw(14) << "Train"
         << setw(14) << "Test"
         << setw(14) << "Beneficio"
         << setw(14) << "Desv"
         << setw(14) << "Evals"
         << setw(14) << "Tiempo(s)"
         << setw(21) << "\n";
    cout << string(121, '-') << "\n";
}

void print_row(const string &name, const Estadisticas &s)
{
    cout << left << setw(16) << name
         << right << setw(14) << s.media_fitness_train
         << setw(14) << s.media_fitness_test
         << setw(14) << s.media_beneficio_test
         << setw(14) << s.desviacion_tipica
         << setw(14) << static_cast<int>(s.evaluaciones_medias)
         << setw(14) << s.tiempo_medio_segundos
         << "\n";
}

void append_csv_rows(ofstream &csv, const string &alg_name, const vector<double> &values)
{
    for (double f : values)
    {
        csv << alg_name << "," << f << "\n";
    }
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

    cout << "\n==============================================================\n";
    cout << "Portfolio Optimization Pr2 - Baselines + AG/AM/DE\n";
    cout << "[PARAMS] seed=" << config.seed
         << " lambda=" << config.lambda
         << " evals=" << config.max_evaluaciones
         << " runs=" << config.num_ejecuciones << "\n";
    cout << "==============================================================\n\n";

    for (const auto &mercado : mercados)
    {
        try
        {
            PortfolioProblem prob_train(
                mercado.ruta_csv,
                mercado.limite_inferior,
                mercado.limite_superior,
                false,
                config.lambda);
            PortfolioProblem prob_test(
                mercado.ruta_csv,
                mercado.limite_inferior,
                mercado.limite_superior,
                true,
                config.lambda);

            RandomSearch<double> alg_random;
            GreedySearch alg_greedy;
            LocalSearch alg_local(config.ls_ratio);
            LocalSearchBest alg_local_best(config.ls_ratio);
            LocalSearchMultiStart alg_local_multi(config.ls_ratio);

            Estadisticas s_gr = ejecutar_experimento_determinista(alg_greedy, prob_train, prob_test);
            Estadisticas s_rs = ejecutar_experimento_estocastico(
                alg_random, prob_train, prob_test,
                config.num_ejecuciones, config.max_evaluaciones, config.seed);
            Estadisticas s_ls = ejecutar_experimento_estocastico(
                alg_local, prob_train, prob_test,
                config.num_ejecuciones, config.max_evaluaciones, config.seed);
            Estadisticas s_lsb = ejecutar_experimento_estocastico(
                alg_local_best, prob_train, prob_test,
                config.num_ejecuciones, config.max_evaluaciones, config.seed);
            Estadisticas s_lsm = ejecutar_experimento_estocastico(
                alg_local_multi, prob_train, prob_test,
                config.num_ejecuciones, config.max_evaluaciones, config.seed);

            Estadisticas s_agg_arit = {}; // generational genetic algorithm with arithmetic crossover
            Estadisticas s_agg_blx = {}; // generational genetic algorithm with BLX crossover
            Estadisticas s_age_arit = {}; // steady-state genetic algorithm with arithmetic crossover
            Estadisticas s_age_blx = {}; // steady-state genetic algorithm with BLX crossover
            Estadisticas s_am_all = {}; // memetic algorithm with all local search
            Estadisticas s_am_rand = {}; // memetic algorithm with random local search
            Estadisticas s_am_best = {}; // memetic algorithm with best local search
            Estadisticas s_de = {}; // differential evolution
            Estadisticas s_agg_gauss = {}; // generational genetic algorithm with Gaussian mutation
            Estadisticas s_am_lsch = {}; // memetic algorithm with Local Search Chains

            CrossoverType best_agg_crossover = CrossoverType::ARITHMETIC;

            if (config.exp_run_ag_am == 1)
            {
                GeneticAlgorithm agg_arit(
                    GAScheme::AGG,
                    CrossoverType::ARITHMETIC,
                    config.ga_pop_size,
                    config.ga_pc_agg,
                    config.ga_pm_indiv,
                    config.ga_blx_alpha,
                    config.ga_mutation_ratio);
                GeneticAlgorithm agg_blx(
                    GAScheme::AGG,
                    CrossoverType::BLX,
                    config.ga_pop_size,
                    config.ga_pc_agg,
                    config.ga_pm_indiv,
                    config.ga_blx_alpha,
                    config.ga_mutation_ratio);
                GeneticAlgorithm age_arit(
                    GAScheme::AGE,
                    CrossoverType::ARITHMETIC,
                    config.ga_pop_size,
                    config.ga_pc_age,
                    config.ga_pm_indiv,
                    config.ga_blx_alpha,
                    config.ga_mutation_ratio);
                GeneticAlgorithm age_blx(
                    GAScheme::AGE,
                    CrossoverType::BLX,
                    config.ga_pop_size,
                    config.ga_pc_age,
                    config.ga_pm_indiv,
                    config.ga_blx_alpha,
                    config.ga_mutation_ratio);

                GeneticAlgorithm agg_gauss(
                    GAScheme::AGG,
                    CrossoverType::ARITHMETIC,
                    config.ga_pop_size,
                    config.ga_pc_agg,
                    config.ga_pm_indiv,
                    config.ga_blx_alpha,
                    config.ga_mutation_ratio,
                    MutationType::GAUSSIAN,
                    config.ga_gaussian_sigma);

                s_agg_arit = ejecutar_experimento_estocastico(
                    agg_arit, prob_train, prob_test,
                    config.num_ejecuciones, config.max_evaluaciones, config.seed);
                s_agg_blx = ejecutar_experimento_estocastico(
                    agg_blx, prob_train, prob_test,
                    config.num_ejecuciones, config.max_evaluaciones, config.seed);
                s_age_arit = ejecutar_experimento_estocastico(
                    age_arit, prob_train, prob_test,
                    config.num_ejecuciones, config.max_evaluaciones, config.seed);
                s_age_blx = ejecutar_experimento_estocastico(
                    age_blx, prob_train, prob_test,
                    config.num_ejecuciones, config.max_evaluaciones, config.seed);

                if (config.exp_run_gaussian == 1)
                {
                    s_agg_gauss = ejecutar_experimento_estocastico(
                        agg_gauss, prob_train, prob_test,
                        config.num_ejecuciones, config.max_evaluaciones, config.seed);
                }

                best_agg_crossover =
                    (s_agg_blx.media_fitness_train < s_agg_arit.media_fitness_train)
                        ? CrossoverType::BLX
                        : CrossoverType::ARITHMETIC;

                MemeticAlgorithm am_all(
                    MemeticVariant::ALL,
                    best_agg_crossover,
                    config.ga_pop_size,
                    config.ga_pc_agg,
                    config.ga_pm_indiv,
                    config.ga_blx_alpha,
                    config.ga_mutation_ratio,
                    config.am_ls_period,
                    config.am_ls_max_evals,
                    config.am_ls_ratio,
                    config.am_pls_rand);
                MemeticAlgorithm am_rand(
                    MemeticVariant::RAND,
                    best_agg_crossover,
                    config.ga_pop_size,
                    config.ga_pc_agg,
                    config.ga_pm_indiv,
                    config.ga_blx_alpha,
                    config.ga_mutation_ratio,
                    config.am_ls_period,
                    config.am_ls_max_evals,
                    config.am_ls_ratio,
                    config.am_pls_rand);
                MemeticAlgorithm am_best(
                    MemeticVariant::BEST,
                    best_agg_crossover,
                    config.ga_pop_size,
                    config.ga_pc_agg,
                    config.ga_pm_indiv,
                    config.ga_blx_alpha,
                    config.ga_mutation_ratio,
                    config.am_ls_period,
                    config.am_ls_max_evals,
                    config.am_ls_ratio,
                    config.am_pls_rand);

                s_am_all = ejecutar_experimento_estocastico(
                    am_all, prob_train, prob_test,
                    config.num_ejecuciones, config.max_evaluaciones, config.seed);
                s_am_rand = ejecutar_experimento_estocastico(
                    am_rand, prob_train, prob_test,
                    config.num_ejecuciones, config.max_evaluaciones, config.seed);
                s_am_best = ejecutar_experimento_estocastico(
                    am_best, prob_train, prob_test,
                    config.num_ejecuciones, config.max_evaluaciones, config.seed);

                if (config.exp_run_lsch == 1)
                {
                    MemeticLSCh am_lsch(
                        best_agg_crossover,
                        config.ga_pop_size,
                        config.ga_pc_agg,
                        config.ga_pm_indiv,
                        config.ga_blx_alpha,
                        config.ga_mutation_ratio,
                        config.am_ls_period,
                        config.am_lsch_base_budget,
                        config.am_lsch_max_budget,
                        config.am_lsch_growth);

                    s_am_lsch = ejecutar_experimento_estocastico(
                        am_lsch, prob_train, prob_test,
                        config.num_ejecuciones, config.max_evaluaciones, config.seed);
                }
            }

            if (config.exp_run_extras == 1)
            {
                DifferentialEvolution de(config.de_f, config.de_cr, config.de_pop_size);
                s_de = ejecutar_experimento_estocastico(
                    de, prob_train, prob_test,
                    config.num_ejecuciones, config.max_evaluaciones, config.seed);
            }

            cout << "\nResultados: " << mercado.nombre << "\n";
            cout << fixed << setprecision(3);

            print_header();
            print_row("Greedy", s_gr);
            print_row("Random", s_rs);
            print_row("BL", s_ls);
            print_row("BL Best", s_lsb);
            print_row("BL Multi", s_lsm);

            if (config.exp_run_ag_am == 1)
            {
                print_row("AGG-Arit", s_agg_arit);
                print_row("AGG-BLX", s_agg_blx);
                print_row("AGE-Arit", s_age_arit);
                print_row("AGE-BLX", s_age_blx);
                print_row("AM-All", s_am_all);
                print_row("AM-Rand", s_am_rand);
                print_row("AM-Best", s_am_best);
                if (config.exp_run_gaussian == 1)
                {
                    print_row("AGG-Gauss", s_agg_gauss);
                }
                if (config.exp_run_lsch == 1)
                {
                    print_row("AM-LSCh", s_am_lsch);
                }
            }

            if (config.exp_run_extras == 1)
            {
                print_row("DE", s_de);
            }
            cout << string(121, '-') << "\n";

            // Save results to CSV
            string nombre_limpio = mercado.nombre;
            for (char &c : nombre_limpio)
            {
                if (c == ' ')
                    c = '_';
            }
            string csv_name = nombre_limpio + "_resultados.csv";

            ofstream csv(csv_name);
            if (!csv.is_open())
            {
                cout << " [WARNING] Could not create CSV for " << mercado.nombre << "\n";
            }
            else
            {
                csv << "alg,fitness\n";
                append_csv_rows(csv, "RANDOMSEARCH", s_rs.historico_fitness);
                append_csv_rows(csv, "GREEDYSEARCH", s_gr.historico_fitness);
                append_csv_rows(csv, "LOCALSEARCH", s_ls.historico_fitness);
                append_csv_rows(csv, "LOCALSEARCHBEST", s_lsb.historico_fitness);
                append_csv_rows(csv, "LOCALSEARCHMULTI", s_lsm.historico_fitness);

                if (config.exp_run_ag_am == 1)
                {
                    append_csv_rows(csv, "AGG-ARIT", s_agg_arit.historico_fitness);
                    append_csv_rows(csv, "AGG-BLX", s_agg_blx.historico_fitness);
                    append_csv_rows(csv, "AGE-ARIT", s_age_arit.historico_fitness);
                    append_csv_rows(csv, "AGE-BLX", s_age_blx.historico_fitness);
                    append_csv_rows(csv, "AM-ALL", s_am_all.historico_fitness);
                    append_csv_rows(csv, "AM-RAND", s_am_rand.historico_fitness);
                    append_csv_rows(csv, "AM-BEST", s_am_best.historico_fitness);
                    if (config.exp_run_gaussian == 1)
                    {
                        append_csv_rows(csv, "AGG-GAUSS", s_agg_gauss.historico_fitness);
                    }
                    if (config.exp_run_lsch == 1)
                    {
                        append_csv_rows(csv, "AM-LSCH", s_am_lsch.historico_fitness);
                    }
                }

                if (config.exp_run_extras == 1)
                {
                    append_csv_rows(csv, "DE", s_de.historico_fitness);
                }
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
