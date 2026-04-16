#pragma once
#include <mh.h>

#include<cassert> 

using namespace std;

/**
 * Implementation of the Random Search metaheuristic
 *  - Randomly generates solutions and selects the best one
 *
 * @see MH
 * @see Problem
 */
template <typename tDomain> class RandomSearch : public MH<tDomain> {

public:
  RandomSearch() : MH<tDomain>() {}
  virtual ~RandomSearch() {}
  // Implement the MH interface methods
  /**
   * Create random solutions until maxevals has been achieved, and returns the
   * best one.
   *
   * @param problem The problem to be optimized
   * @param maxevals Maximum number of evaluations allowed
   * @return A pair containing the best solution found and its fitness
   */
  ResultMH<tDomain> optimize(Problem<tDomain> &problem, int maxevals) override {
    assert(maxevals > 0);
    tSolution<tDomain> best;
    tFitness best_fitness = -1;

    for (int i = 0; i < maxevals; i++) {
      auto solution = problem.createSolution();
      tFitness fitness = problem.fitness(solution);
      // we ensure that in the first iteration we update the best solution, and then we compare it with the others
      if (i == 0 || fitness > best_fitness) { 
        best = solution;
        best_fitness = fitness;
      }
    }

    // return ResultMH(best, best_fitness, maxevals);
    return ResultMH<tDomain>(best, best_fitness, maxevals); // ADD TDOMAIN
  }
};

/*
why have we implemented this method here instead of in the .cpp file?
Because it is a template class, and template implementations must be
defined in the same translation unit where they are instantiated.
Separating the declaration and definition would cause linker errors.
*/
