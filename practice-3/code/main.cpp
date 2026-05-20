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
#include <sstream>

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
#include "simulated_annealing.h"
#include "bmb.h"
#include "ils.h"
#include "ils_es.h"
#include "ils_es_chc.h"

using namespace std;
using namespace std::chrono;

namespace ui
{
    // ANSI styles for terminal output.
    const string RESET = "\033[0m";
    const string BOLD = "\033[1m";
    const string DIM = "\033[2m";

    const string CYAN = "\033[36m";
    const string BLUE = "\033[34m";
    const string GREEN = "\033[32m";
    const string YELLOW = "\033[33m";
    const string RED = "\033[31m";
    const string MAGENTA = "\033[35m";

    string colorize(const string &text, const string &color)
    {
        return color + text + RESET;
    }

    string pad_double(double value, int width, int precision = 3)
    {
        ostringstream oss;
        oss << right << setw(width) << fixed << setprecision(precision) << value;
        return oss.str();
    }

    string pad_int(int value, int width)
    {
        ostringstream oss;
        oss << right << setw(width) << value;
        return oss.str();
    }

    string color_metric(double value, int width, int precision = 3)
    {
        const string padded = pad_double(value, width, precision);
        if (value > 0.0)
            return colorize(padded, GREEN);
        if (value < 0.0)
            return colorize(padded, RED);
        return colorize(padded, YELLOW);
    }

    string separator(int width)
    {
        return colorize(string(width, '-'), DIM);
    }
}

// Data structures

// Info about a stock market
struct Market
{
    string name;
    string csv_path;
    double lower_bound; // Min investment
    double upper_bound; // Max investment
};

// Results from an algorithm run
struct Statistics
{
    double mean_train_fitness;  // Average fitness on 2015-2024
    double mean_test_fitness;   // Average fitness on 2025
    double mean_test_profit; // Actual profit on 2025
    double standard_deviation;    // Standard deviation on training
    double mean_time_seconds;
    double mean_evaluations;
    vector<double> fitness_history; // All fitness values for boxplots
};

// Run a stochastic algorithm multiple times
Statistics run_stochastic_experiment(
    MH<double> &algoritmo,
    PortfolioProblem &prob_train,
    PortfolioProblem &prob_test,
    int num_executions,
    int max_evals,
    long int base_seed)
{
    Statistics results = {0, 0, 0, 0, 0, 0, {}};
    results.fitness_history.reserve(num_executions);

    for (int i = 0; i < num_executions; ++i)
    {
        Random::seed(base_seed + i);

        auto inicio = high_resolution_clock::now();
        // Train on 2015-2024 data
        auto resultado = algoritmo.optimize(prob_train, max_evals);
        auto fin = high_resolution_clock::now();

        // Test weights on 2025
        double fitness_2025 = prob_test.fitness(resultado.solution);
        double profit_2025 = prob_test.getProfit(resultado.solution);

        // Accumulate metrics
        results.mean_train_fitness += resultado.fitness;
        results.mean_test_fitness += fitness_2025;
        results.mean_test_profit += profit_2025;
        results.mean_time_seconds += duration<double>(fin - inicio).count();
        results.mean_evaluations += resultado.evaluations;

        results.fitness_history.push_back(resultado.fitness);
    }

    // Calculate averages
    results.mean_train_fitness /= num_executions;
    results.mean_test_fitness /= num_executions;
    results.mean_test_profit /= num_executions;
    results.mean_time_seconds /= num_executions;
    results.mean_evaluations /= num_executions;

    // Calculate standard deviation
    double variance = 0.0;
    for (double fitness : results.fitness_history)
    {
        variance += pow(fitness - results.mean_train_fitness, 2);
    }
    results.standard_deviation = sqrt(variance / num_executions);

    return results;
}

// Run a deterministic algorithm once
Statistics run_deterministic_experiment(
    MH<double> &algoritmo,
    PortfolioProblem &prob_train,
    PortfolioProblem &prob_test)
{
    Statistics results = {0, 0, 0, 0, 0, 0, {}};

    auto inicio = high_resolution_clock::now();
    auto resultado = algoritmo.optimize(prob_train, 1);
    auto fin = high_resolution_clock::now();

    // Test weights on 2025
    double fitness_2025 = prob_test.fitness(resultado.solution);
    double profit_2025 = prob_test.getProfit(resultado.solution);

    results.mean_train_fitness = resultado.fitness;
    results.mean_test_fitness = fitness_2025;
    results.mean_test_profit = profit_2025;
    results.mean_time_seconds = duration<double>(fin - inicio).count();
    results.mean_evaluations = resultado.evaluations;
    results.standard_deviation = 0.0;

    results.fitness_history.push_back(resultado.fitness);

    return results;
}

void print_header()
{
    const int width_total = 121;
    cout << ui::separator(width_total) << "\n";
    cout << ui::colorize(ui::BOLD + string("Results Table"), ui::BLUE) << "\n";
    cout << ui::separator(width_total) << "\n";
    cout << ui::colorize((ostringstream() << left << setw(16) << "Algorithm"
                        << right << setw(14) << "2015-2024"
                        << setw(14) << "2025"
                        << setw(14) << "2025"
                        << setw(14) << "Std"
                        << setw(14) << "Evals"
                        << setw(14) << "Time(s)"
                        << setw(1) << "").str(), ui::BOLD + ui::MAGENTA) << "\n";
    cout << ui::colorize((ostringstream() << left << setw(16) << ""
                        << right << setw(14) << "Fitness"
                        << setw(14) << "Fitness"
                        << setw(14) << "Profit"
                        << setw(14) << ""
                        << setw(14) << ""
                        << setw(14) << ""
                        << setw(1) << "").str(), ui::BOLD + ui::MAGENTA) << "\n";
    cout << ui::separator(width_total) << "\n";
}

void print_row(const string &name, const Statistics &s)
{
    ostringstream name_col;
    name_col << left << setw(16) << name;

    cout << ui::colorize(name_col.str(), ui::CYAN)
         << ui::color_metric(s.mean_train_fitness, 14)
         << ui::color_metric(s.mean_test_fitness, 14)
         << ui::color_metric(s.mean_test_profit, 14)
         << ui::color_metric(s.standard_deviation, 14)
         << ui::colorize(ui::pad_int(static_cast<int>(s.mean_evaluations), 14), ui::YELLOW)
         << ui::colorize(ui::pad_double(s.mean_time_seconds, 14), ui::BLUE)
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
    // 1. Initialize configuration from file or command-line overrides.
    Config config = ConfigReader::read("config.cfg");

    // Allow seed override from command line
    if (argc > 1)
    {
        config.seed = atoi(argv[1]);
    }

    vector<Market> markets;

    // Setup markets
    if (config.use_custom_market == 1)
    {
        cout << "\n" << ui::colorize("[INFO]", ui::GREEN) << " "
             << "CUSTOM mode ON. Running only config market.\n";
        markets.push_back({config.custom_name,
                  config.custom_path,
                  config.custom_lo,
                  config.custom_hi});
    }
    else
    {
        // Default 3 markets
        markets = {
            {"IBEX 35", "datos_portfolio_2526/ibex_35.csv", 0.005, 0.08},
            {"S&P 100", "datos_portfolio_2526/syp_100.csv", 0.005, 0.05},
            {"S&P 500", "datos_portfolio_2526/syp_500.csv", 0.005, 0.02}};
    }

        cout << "\n" << ui::colorize(string(62, '='), ui::DIM) << "\n";
        cout << ui::colorize(ui::BOLD + string("Portfolio Optimization Pr3 - Trajectory Algorithms (ES, BMB, ILS)"), ui::BLUE) << "\n";
        cout << ui::colorize("[PARAMS]", ui::MAGENTA) << " seed=" << config.seed
         << " lambda=" << config.lambda
         << " evals=" << config.max_evaluations
         << " runs=" << config.num_executions << "\n";
        cout << ui::colorize(string(62, '='), ui::DIM) << "\n\n";

    for (const auto &market : markets)
    {
        try
        {
            PortfolioProblem prob_train(
                market.csv_path,
                market.lower_bound,
                market.upper_bound,
                false,
                config.lambda);
            PortfolioProblem prob_test(
                market.csv_path,
                market.lower_bound,
                market.upper_bound,
                true,
                config.lambda);

            RandomSearch<double> alg_random;
            GreedySearch alg_greedy;
            LocalSearch alg_local(config.ls_ratio);
            LocalSearchBest alg_local_best(config.ls_ratio);
            LocalSearchMultiStart alg_local_multi(config.ls_ratio);

            Statistics s_gr = run_deterministic_experiment(alg_greedy, prob_train, prob_test);
            Statistics s_rs = run_stochastic_experiment(
                alg_random, prob_train, prob_test,
                config.num_executions, config.max_evaluations, config.seed);
            Statistics s_ls = run_stochastic_experiment(
                alg_local, prob_train, prob_test,
                config.num_executions, config.max_evaluations, config.seed);
            Statistics s_lsb = run_stochastic_experiment(
                alg_local_best, prob_train, prob_test,
                config.num_executions, config.max_evaluations, config.seed);
            Statistics s_lsm = run_stochastic_experiment(
                alg_local_multi, prob_train, prob_test,
                config.num_executions, config.max_evaluations, config.seed);

            Statistics s_agg_arit = {}; // generational genetic algorithm with arithmetic crossover
            Statistics s_agg_blx = {}; // generational genetic algorithm with BLX crossover
            Statistics s_age_arit = {}; // steady-state genetic algorithm with arithmetic crossover
            Statistics s_age_blx = {}; // steady-state genetic algorithm with BLX crossover
            Statistics s_am_all = {}; // memetic algorithm with all local search
            Statistics s_am_rand = {}; // memetic algorithm with random local search
            Statistics s_am_best = {}; // memetic algorithm with best local search
            Statistics s_de = {}; // differential evolution
            Statistics s_agg_gauss = {}; // generational genetic algorithm with Gaussian mutation
            Statistics s_am_lsch = {}; // memetic algorithm with Local Search Chains
            Statistics s_es = {};      // simulated annealing
            Statistics s_bmb = {};     // basic multi-start search
            Statistics s_ils = {};     // iterated local search
            Statistics s_ils_es = {};  // hybrid ils-es
            Statistics s_ils_es_chc = {};  // hybrid ils-es with reinicialization (CHC/GADEGD)

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

                s_agg_arit = run_stochastic_experiment(
                    agg_arit, prob_train, prob_test,
                    config.num_executions, config.max_evaluations, config.seed);
                s_agg_blx = run_stochastic_experiment(
                    agg_blx, prob_train, prob_test,
                    config.num_executions, config.max_evaluations, config.seed);
                s_age_arit = run_stochastic_experiment(
                    age_arit, prob_train, prob_test,
                    config.num_executions, config.max_evaluations, config.seed);
                s_age_blx = run_stochastic_experiment(
                    age_blx, prob_train, prob_test,
                    config.num_executions, config.max_evaluations, config.seed);

                if (config.exp_run_gaussian == 1)
                {
                    s_agg_gauss = run_stochastic_experiment(
                        agg_gauss, prob_train, prob_test,
                        config.num_executions, config.max_evaluations, config.seed);
                }

                best_agg_crossover =
                    (s_agg_blx.mean_train_fitness > s_agg_arit.mean_train_fitness)
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

                s_am_all = run_stochastic_experiment(
                    am_all, prob_train, prob_test,
                    config.num_executions, config.max_evaluations, config.seed);
                s_am_rand = run_stochastic_experiment(
                    am_rand, prob_train, prob_test,
                    config.num_executions, config.max_evaluations, config.seed);
                s_am_best = run_stochastic_experiment(
                    am_best, prob_train, prob_test,
                    config.num_executions, config.max_evaluations, config.seed);

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
                        config.am_ls_ratio,
                        config.am_lsch_i_str);

                    s_am_lsch = run_stochastic_experiment(
                        am_lsch, prob_train, prob_test,
                        config.num_executions, config.max_evaluations, config.seed);
                }
            }

            if (config.exp_run_extras == 1)
            {
                DifferentialEvolution de(config.de_f, config.de_cr, config.de_pop_size);
                s_de = run_stochastic_experiment(
                    de, prob_train, prob_test,
                    config.num_executions, config.max_evaluations, config.seed);
            }

            if (config.exp_run_p3 == 1)
            {
                size_t n = prob_train.getSolutionSize();
                // If es_max_vecinos is -1, we dynamically scale it to 10 * dimension (n).
                int es_max_vecinos = (config.es_max_vecinos == -1) ? 10 * n : config.es_max_vecinos;
                // If es_max_exitos is -1, we scale it to 10% of the max neighbors per level.
                int es_max_exitos = (config.es_max_exitos == -1) ? 0.1 * es_max_vecinos : config.es_max_exitos;

                SimulatedAnnealing es(config.es_mu, config.es_phi, config.es_tf, config.ls_ratio, es_max_vecinos, es_max_exitos);
                BMB bmb(config.traj_num_starts, config.traj_max_evals_bl, config.ls_ratio);
                ILS ils(config.traj_num_starts, config.traj_max_evals_bl, config.ls_ratio, config.traj_mutation_rate);
                ILSES ils_es(config.traj_num_starts, config.traj_max_evals_bl, config.ls_ratio, 
                             config.es_mu, config.es_phi, config.es_tf, es_max_vecinos, es_max_exitos, config.traj_mutation_rate);
                ILSES_CHC ils_es_chc(config.es_mu, config.es_phi, config.es_tf, config.ls_ratio, 
                                     es_max_vecinos, es_max_exitos, config.traj_num_starts, 
                                     10,  // limit_stagnation: 10 iterations without improvement
                                     0.40); // macro mutation rate: 40%

                s_es = run_stochastic_experiment(es, prob_train, prob_test, config.num_executions, config.max_evaluations, config.seed);
                s_bmb = run_stochastic_experiment(bmb, prob_train, prob_test, config.num_executions, config.max_evaluations, config.seed);
                s_ils = run_stochastic_experiment(ils, prob_train, prob_test, config.num_executions, config.max_evaluations, config.seed);
                s_ils_es = run_stochastic_experiment(ils_es, prob_train, prob_test, config.num_executions, config.max_evaluations, config.seed);
                s_ils_es_chc = run_stochastic_experiment(ils_es_chc, prob_train, prob_test, config.num_executions, config.max_evaluations, config.seed);
            }

              cout << "\n" << ui::colorize("Market:", ui::BOLD + ui::MAGENTA) << " "
                  << ui::colorize(market.name, ui::BOLD + ui::CYAN) << "\n";
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

            if (config.exp_run_p3 == 1)
            {
                print_row("ES", s_es);
                print_row("BMB", s_bmb);
                print_row("ILS", s_ils);
                print_row("ILS-ES", s_ils_es);
                print_row("ILS-ES+CHC", s_ils_es_chc);
            }
            cout << ui::separator(121) << "\n";

            // Save results to CSV
            string clean_name = market.name;
            for (char &c : clean_name)
            {
                if (c == ' ')
                    c = '_';
            }
            string csv_name = clean_name + "_results.csv";

            ofstream csv(csv_name);
            if (!csv.is_open())
            {
                cout << " " << ui::colorize("[WARNING]", ui::YELLOW)
                     << " Could not create CSV for " << market.name << "\n";
            }
            else
            {
                csv << "alg,fitness\n";
                append_csv_rows(csv, "RANDOMSEARCH", s_rs.fitness_history);
                append_csv_rows(csv, "GREEDYSEARCH", s_gr.fitness_history);
                append_csv_rows(csv, "LOCALSEARCH", s_ls.fitness_history);
                append_csv_rows(csv, "LOCALSEARCHBEST", s_lsb.fitness_history);
                append_csv_rows(csv, "LOCALSEARCHMULTI", s_lsm.fitness_history);

                if (config.exp_run_ag_am == 1)
                {
                    append_csv_rows(csv, "AGG-ARIT", s_agg_arit.fitness_history);
                    append_csv_rows(csv, "AGG-BLX", s_agg_blx.fitness_history);
                    append_csv_rows(csv, "AGE-ARIT", s_age_arit.fitness_history);
                    append_csv_rows(csv, "AGE-BLX", s_age_blx.fitness_history);
                    append_csv_rows(csv, "AM-ALL", s_am_all.fitness_history);
                    append_csv_rows(csv, "AM-RAND", s_am_rand.fitness_history);
                    append_csv_rows(csv, "AM-BEST", s_am_best.fitness_history);
                    if (config.exp_run_gaussian == 1)
                    {
                        append_csv_rows(csv, "AGG-GAUSS", s_agg_gauss.fitness_history);
                    }
                    if (config.exp_run_lsch == 1)
                    {
                        append_csv_rows(csv, "AM-LSCH", s_am_lsch.fitness_history);
                    }
                }

                if (config.exp_run_extras == 1)
                {
                    append_csv_rows(csv, "DE", s_de.fitness_history);
                }

                if (config.exp_run_p3 == 1)
                {
                    append_csv_rows(csv, "ES", s_es.fitness_history);
                    append_csv_rows(csv, "BMB", s_bmb.fitness_history);
                    append_csv_rows(csv, "ILS", s_ils.fitness_history);
                    append_csv_rows(csv, "ILS-ES", s_ils_es.fitness_history);
                    append_csv_rows(csv, "ILS-ES-CHC", s_ils_es_chc.fitness_history);
                }
            }
        }
        catch (const exception &e)
        {
            cout << "\n " << ui::colorize("[ERROR]", ui::RED)
                 << " Failed to process " << market.name << ": " << e.what() << "\n\n";
        }
    }

    cout << "\n" << ui::colorize(ui::BOLD + string(">>> Done."), ui::GREEN)
         << " Results saved to CSV files." << endl;
    return 0;
}
