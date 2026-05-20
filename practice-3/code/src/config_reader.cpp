#include "config_reader.h"
#include <fstream>
#include <sstream>
#include <iostream>

Config ConfigReader::read(const std::string& filename) {
    // Initialization with default values to ensure a valid state if parameters are missing.
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
        0.3,      // es_mu
        0.2,      // es_phi
        1e-3,     // es_tf
        -1,       // es_max_vecinos (-1 triggers calculation as 10*n in main)
        -1,       // es_max_exitos (-1 triggers calculation as 0.1 * max_vecinos)
        2000,     // traj_max_evals_bl
        5,        // traj_num_starts
        0.2,      // traj_mutation_rate
        1,        // exp_run_ag_am
        1,        // exp_run_extras
        1,        // exp_run_gaussian
        1,        // exp_run_lsch
        1,        // exp_run_p3
        0,        // use_custom_market
        "Default", // custom_name
        "",         // custom_path
        0.0,        // custom_lo
        1.0         // custom_hi
    };

    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << " [WARNING] Configuration file not found: " << filename << ". Using default parameters.\n";
        return config;
    }

    std::string line;
    while (std::getline(file, line)) {
        // Ignore empty lines and comments (lines starting with # or ;)
        if (line.empty() || line[0] == '#' || line[0] == ';') continue; 
        
        std::stringstream ss(line);
        std::string key, value;
        
        // Split by the first '=' character.
        if (std::getline(ss, key, '=') && std::getline(ss, value)) {
            // Trim whitespace from both ends of key and value for robustness.
            auto trim = [](std::string& s) {
                s.erase(0, s.find_first_not_of(" \t\r\n"));
                s.erase(s.find_last_not_of(" \t\r\n") + 1);
            };
            trim(key);
            trim(value);

            // Manual mapping of keys to Config struct members.
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
            else if (key == "ES_MU") config.es_mu = std::stod(value);
            else if (key == "ES_PHI") config.es_phi = std::stod(value);
            else if (key == "ES_TF") config.es_tf = std::stod(value);
            else if (key == "ES_MAX_VECINOS") config.es_max_vecinos = std::stoi(value);
            else if (key == "ES_MAX_EXITOS") config.es_max_exitos = std::stoi(value);
            else if (key == "TRAJ_MAX_EVALS_BL") config.traj_max_evals_bl = std::stoi(value);
            else if (key == "TRAJ_NUM_STARTS") config.traj_num_starts = std::stoi(value);
            else if (key == "TRAJ_MUTATION_RATE") config.traj_mutation_rate = std::stod(value);
            else if (key == "EXP_RUN_AG_AM") config.exp_run_ag_am = std::stoi(value);
            else if (key == "EXP_RUN_EXTRAS") config.exp_run_extras = std::stoi(value);
            else if (key == "EXP_RUN_GAUSSIAN") config.exp_run_gaussian = std::stoi(value);
            else if (key == "EXP_RUN_LSCH") config.exp_run_lsch = std::stoi(value);
            else if (key == "EXP_RUN_P3") config.exp_run_p3 = std::stoi(value);
            else if (key == "USE_CUSTOM_MARKET") config.use_custom_market = std::stoi(value);
            else if (key == "CUSTOM_NAME") config.custom_name = value;
            else if (key == "CUSTOM_PATH") config.custom_path = value;
            else if (key == "CUSTOM_LO") config.custom_lo = std::stod(value);
            else if (key == "CUSTOM_HI") config.custom_hi = std::stod(value);
        }
    }
    return config;
}
