#ifndef __CONFIG_READER_H
#define __CONFIG_READER_H

#include <string>

struct Config {
    long int seed;
    int max_evaluations;
    int num_executions;
    double lambda;
    double ls_ratio;

    // Evolutionary algorithm parameters
    int ga_pop_size;           // Population size (number of chromosomes/portfolios per generation)
    double ga_pc_agg;          // Crossover probability for Generational Genetic Algorithm
    double ga_pc_age;          // Crossover probability for Steady-State Genetic Algorithm
    double ga_pm_indiv;        // Mutation probability per individual
    double ga_blx_alpha;       // Alpha parameter for BLX-alpha crossover
    double ga_mutation_ratio;  // Percentage of gene value exchanged during standard mutation
    double ga_gaussian_sigma;  // Standard deviation for Gaussian mutation

    // Memetic algorithm parameters
    int am_ls_period;          // Period/frequency of applying Local Search (in number of generations)
    int am_ls_max_evals;       // Maximum evaluations allowed per Local Search invocation
    double am_ls_ratio;        // Ratio of the population to which Local Search is applied
    double am_pls_rand;        // Probability of applying Local Search to a specific individual in AM-Rand variant
    int am_lsch_base_budget;   // Base intensity for Local Search Chains variant
    int am_lsch_max_budget;    // Maximum intensity for Local Search Chains variant
    double am_lsch_growth;     // Growth factor for Local Search Chains variant

    // Differential Evolution parameters
    double de_f;               // Scaling factor for differential mutation
    double de_cr;              // Crossover ratio for recombination
    int de_pop_size;           // Population size for Differential Evolution

    // Execution flags
    int exp_run_ag_am;         // Flag to enable/disable Genetic and Memetic algorithms execution
    int exp_run_extras;        // Flag to enable/disable Extra variants execution
    int exp_run_gaussian;      // Flag to enable/disable Gaussian mutation variant
    int exp_run_lsch;          // Flag to enable/disable Local Search Chains variant
    
    // Parameters for the custom market
    int use_custom_market;
    std::string custom_name;
    std::string custom_path;
    double custom_lo;
    double custom_hi;
};

class ConfigReader {
public:
    static Config read(const std::string& filename);
};

#endif