#ifndef __CONFIG_READER_H
#define __CONFIG_READER_H

#include <string>

/**
 * Stores all configuration parameters loaded from the .cfg file.
 *
 * This structure centralizes hyperparameters for all implemented metaheuristics
 * to allow experimentation without recompilation.
 *
 * @author  Ismael Sallami Moreno
 */
struct Config {
    long int seed;          ///< Random number generator seed.
    int max_evaluations;    ///< Total budget of function evaluations per execution.
    int num_executions;     ///< Number of independent runs per algorithm/market.
    double lambda;          ///< Risk aversion parameter for the fitness function.
    double ls_ratio;        ///< Neighborhood transfer ratio for local search.

    // Evolutionary algorithm parameters
    int ga_pop_size;           ///< Population size for Genetic Algorithms.
    double ga_pc_agg;          ///< Crossover probability (Generational).
    double ga_pc_age;          ///< Crossover probability (Steady-State).
    double ga_pm_indiv;        ///< Mutation probability per individual.
    double ga_blx_alpha;       ///< Alpha parameter for BLX crossover.
    double ga_mutation_ratio;  ///< Intensity of gene value exchange in mutation.
    double ga_gaussian_sigma;  ///< Standard deviation for Gaussian mutation.

    // Memetic algorithm parameters
    int am_ls_period;          ///< Generation frequency for applying LS.
    int am_ls_max_evals;       ///< Max evaluations per LS call within AM.
    double am_ls_ratio;        ///< Population percentage to undergo LS.
    double am_pls_rand;        ///< LS probability for AM-Random.
    int am_lsch_i_str;         ///< Fixed intensity for LS Chains.

    // Differential Evolution parameters
    double de_f;               ///< Scaling factor for mutation.
    double de_cr;              ///< Crossover ratio.
    int de_pop_size;           ///< Population size.

    // Practice 3 (Trajectory-based) parameters
    double es_mu;              ///< Initial worsening coefficient for SA.
    double es_phi;             ///< Initial acceptance probability for SA.
    double es_tf;              ///< Final temperature for SA.
    int es_max_vecinos;        ///< Max neighbors explored per SA level.
    int es_max_exitos;         ///< Max moves accepted per SA level.
    int traj_max_evals_bl;     ///< Evaluation budget per LS in BMB/ILS.
    int traj_num_starts;       ///< Total restarts for BMB/ILS.
    double traj_mutation_rate; ///< Fraction of assets perturbed in ILS.

    // Execution flags (Toggle specific experiment blocks)
    int exp_run_ag_am;         ///< Enable GA and Memetic variants.
    int exp_run_extras;        ///< Enable extra experimental variants.
    int exp_run_gaussian;      ///< Enable Gaussian mutation tests.
    int exp_run_lsch;          ///< Enable Local Search Chains.
    int exp_run_p3;            ///< Enable Practice 3 (Trajectory) algorithms.
    
    // Custom market configuration
    int use_custom_market;     ///< Boolean flag to use an external dataset.
    std::string custom_name;   ///< Label for the custom market.
    std::string custom_path;   ///< Path to the custom CSV file.
    double custom_lo;          ///< Custom lower bound for weights.
    double custom_hi;          ///< Custom upper bound for weights.
};

/**
 * Utility class for parsing the project's configuration file.
 */
class ConfigReader {
public:
    /**
     * Parses the given configuration file and returns a populated Config struct.
     *
     * @param filename Path to the .cfg file.
     * @return A structure containing all loaded parameters.
     */
    static Config read(const std::string& filename);
};

#endif
