#include "config_reader.h"
#include <fstream>
#include <sstream>
#include <iostream>

Config ConfigReader::read(const std::string& filename) {
    // Set default values for the configuration
    Config config = {
        42,       // seed
        10000,    // max_evaluations
        50,       // num_executions
        500.0,    // lambda
        0.4,      // ls_ratio
        100,      // ga_pop_size
        0.8,      // ga_pc_agg
        1.0,      // ga_pc_age
        0.1,      // ga_pm_indiv
        0.3,      // ga_blx_alpha
        0.15,     // ga_mutation_ratio
        0.02,     // ga_gaussian_sigma
        10,       // am_ls_period
        100,      // am_ls_max_evals
        0.15,     // am_ls_ratio
        0.1,      // am_pls_rand
        500,      // am_lsch_i_str
        0.5,      // de_f
        0.9,      // de_cr
        100,      // de_pop_size
        1,        // exp_run_ag_am
        1,        // exp_run_extras
        1,        // exp_run_gaussian
        1,        // exp_run_lsch
        0,        // use_custom_market
        "Default", // custom_name
        "",         // custom_path
        0.0,        // custom_lo
        1.0         // custom_hi
    };


    // Try to open the configuration file
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << " [WARNING] File not found " << filename << ". Using defaults.\n";
        return config;
    }

    // Read the file line by line
    std::string line;
    while (std::getline(file, line)) {
        // Skip empty lines and comments
        if (line.empty() || line[0] == '#' || line[0] == ';') continue; 
        
        // Parse the key-value pair separated by '='
        std::stringstream ss(line);
        std::string key, value;
        
        if (std::getline(ss, key, '=') && std::getline(ss, value)) {
            // Map each key to its corresponding config value
            if (key == "SEED") config.seed = std::stol(value);
            else if (key == "MAX_EVALUATIONS") config.max_evaluations = std::stoi(value);
            else if (key == "NUM_EXECUTIONS") config.num_executions = std::stoi(value);
            else if (key == "LAMBDA") config.lambda = std::stod(value);
            else if (key == "LS_RATIO") config.ls_ratio = std::stod(value);
            else if (key == "GA_POP_SIZE") config.ga_pop_size = std::stoi(value);
            else if (key == "GA_PC_AGG") config.ga_pc_agg = std::stod(value);
            else if (key == "GA_PC_AGE") config.ga_pc_age = std::stod(value);
            else if (key == "GA_PM_INDIV") config.ga_pm_indiv = std::stod(value);
            else if (key == "GA_BLX_ALPHA") config.ga_blx_alpha = std::stod(value);
            else if (key == "GA_MUTATION_RATIO") config.ga_mutation_ratio = std::stod(value);
            else if (key == "GA_GAUSSIAN_SIGMA") config.ga_gaussian_sigma = std::stod(value);
            else if (key == "AM_LS_PERIOD") config.am_ls_period = std::stoi(value);
            else if (key == "AM_LS_MAX_EVALS") config.am_ls_max_evals = std::stoi(value);
            else if (key == "AM_LS_RATIO") config.am_ls_ratio = std::stod(value);
            else if (key == "AM_PLS_RAND") config.am_pls_rand = std::stod(value);
            else if (key == "AM_LSCH_I_STR") config.am_lsch_i_str = std::stoi(value);
            else if (key == "DE_F") config.de_f = std::stod(value);
            else if (key == "DE_CR") config.de_cr = std::stod(value);
            else if (key == "DE_POP_SIZE") config.de_pop_size = std::stoi(value);
            else if (key == "EXP_RUN_AG_AM") config.exp_run_ag_am = std::stoi(value);
            else if (key == "EXP_RUN_EXTRAS") config.exp_run_extras = std::stoi(value);
            else if (key == "EXP_RUN_GAUSSIAN") config.exp_run_gaussian = std::stoi(value);
            else if (key == "EXP_RUN_LSCH") config.exp_run_lsch = std::stoi(value);
            // Parse custom market parameters
            else if (key == "USE_CUSTOM_MARKET") config.use_custom_market = std::stoi(value);
            else if (key == "CUSTOM_NAME") config.custom_name = value;
            else if (key == "CUSTOM_PATH") config.custom_path = value;
            else if (key == "CUSTOM_LO") config.custom_lo = std::stod(value);
            else if (key == "CUSTOM_HI") config.custom_hi = std::stod(value);
        }
    }
    return config;
}