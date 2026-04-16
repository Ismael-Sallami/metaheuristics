#ifndef __CONFIG_READER_H
#define __CONFIG_READER_H

#include <string>

struct Config {
    long int seed;
    int max_evaluaciones;
    int num_ejecuciones;
    double lambda;
    double ls_ratio;

    // Evolutionary algorithm parameters
    int ga_pop_size;
    double ga_pc_agg;
    double ga_pc_age;
    double ga_pm_indiv;
    double ga_blx_alpha;
    double ga_mutation_ratio;
    double ga_gaussian_sigma;

    // Memetic algorithm parameters
    int am_ls_period;
    int am_ls_max_evals;
    double am_ls_ratio;
    double am_pls_rand;
    int am_lsch_base_budget;
    int am_lsch_max_budget;
    double am_lsch_growth;

    // Differential Evolution parameters
    double de_f;
    double de_cr;
    int de_pop_size;

    // Execution flags
    int exp_run_ag_am;
    int exp_run_extras;
    int exp_run_gaussian;
    int exp_run_lsch;
    
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