#pragma once

#include "mh.h"
#include "ea_common.h"
#include <vector>

/**
 * @file genetic_algorithm.h
 * @brief Genetic algorithm variants used in this practice.
 *
 * The class below implements two classic GA schemes:
 * - AGG: generational genetic algorithm.
 * - AGE: steady-state genetic algorithm.
 *
 * It also supports two crossover operators and two mutation styles.
 */

/**
 * @brief Genetic algorithm execution scheme.
 */
enum class GAScheme {
    /** Create a full new population on each generation. */
    AGG,
    /** Replace only a few individuals at a time. */
    AGE
};

/**
 * @brief Crossover operator used by the genetic algorithm.
 */
enum class CrossoverType {
    /** Blend parents using a random weighted average. */
    ARITHMETIC,
    /** Sample children from the interval expanded with BLX-alpha. */
    BLX
};

/**
 * @brief Mutation operator used by the genetic algorithm.
 */
enum class MutationType {
    /** Move a small fraction of one value from one position to another. */
    TRANSFER,
    /** Apply a Gaussian perturbation to the individual. */
    GAUSSIAN
};

/**
 * @brief Genetic algorithm for continuous optimization problems.
 *
 * The algorithm stores the last convergence curve so it can be plotted later.
 * It works on problems that use double-valued solutions.
 */
class GeneticAlgorithm : public MH<double> {
private:
    /** Selected GA scheme: AGG or AGE. */
    GAScheme m_scheme;
    /** Selected crossover operator. */
    CrossoverType m_crossover_type;

    /** Population size. */
    int m_pop_size;
    /** Probability of applying crossover to a parent pair. */
    double m_pc;
    /** Probability of mutating one individual. */
    double m_pm_indiv;
    /** BLX-alpha expansion factor. */
    double m_blx_alpha;
    /** Fraction used by transfer mutation. */
    double m_mutation_ratio;
    /** Selected mutation operator. */
    MutationType m_mutation_type;
    /** Standard deviation used by Gaussian mutation. */
    double m_gaussian_sigma;

    /** Best fitness value found after each generation or iteration. */
    std::vector<double> m_last_convergence;

    /**
     * Apply the chosen crossover operator to one pair of parents.
     */
    void crossover_pair(
        const tSolution<double> &p1,
        const tSolution<double> &p2,
        tSolution<double> &c1,
        tSolution<double> &c2
    ) const;

    /**
     * Run the generational version of the algorithm.
     */
    ResultMH<double> run_agg(Problem<double> &problem, int maxevals);
    /**
     * Run the steady-state version of the algorithm.
     */
    ResultMH<double> run_age(Problem<double> &problem, int maxevals);

public:
    /**
     * Build a genetic algorithm with the selected operators and parameters.
     *
     * @param scheme Genetic algorithm scheme to use.
     * @param crossover_type Crossover operator to apply.
     * @param pop_size Number of individuals in the population.
     * @param pc Probability of crossover.
     * @param pm_indiv Probability of mutation for each individual.
     * @param blx_alpha BLX-alpha expansion factor.
     * @param mutation_ratio Fraction used by transfer mutation.
     * @param mutation_type Mutation operator to use.
     * @param gaussian_sigma Standard deviation for Gaussian mutation.
     */
    GeneticAlgorithm(
        GAScheme scheme,
        CrossoverType crossover_type,
        int pop_size,
        double pc,
        double pm_indiv,
        double blx_alpha,
        double mutation_ratio,
        MutationType mutation_type = MutationType::TRANSFER,
        double gaussian_sigma = 0.02
    );

    /**
     * Optimize the given problem using the configured GA variant.
     *
     * @param problem Problem instance to solve.
     * @param maxevals Maximum number of fitness evaluations allowed.
     * @return Final solution, its fitness, and the number of evaluations used.
     */
    ResultMH<double> optimize(Problem<double> &problem, int maxevals) override;

    /**
     * Get the convergence history of the last run.
     *
     * @return Fitness values recorded after each generation or iteration.
     */
    const std::vector<double> &last_convergence() const { return m_last_convergence; }
};