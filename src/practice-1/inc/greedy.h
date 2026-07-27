#pragma once
#include <mh.h>

using namespace std;

/**
 * Implementation of the Greedy Search metaheuristic
 *  - Iteratively selects the best neighboring solution
 *
 * @see MH
 * @see Problem
 */
// Template instantiation for double type
using MHDouble = MH<double>;
using ProblemDouble = Problem<double>;
using ResultMHDouble = ResultMH<double>;

class GreedySearch : public MHDouble {

public:
  GreedySearch() : MHDouble() {}
  virtual ~GreedySearch() {}
  // Implement the MH interface methods
  /**
   * Create random solutions until maxevals has been achieved, and returns the
   * best one.
   *
   * @param problem The problem to be optimized
   * @param maxevals Maximum number of evaluations allowed
   * @return A pair containing the best solution found and its fitness
   */
  // virtual ResultMH<int> optimize(Problem<int> &problem, int maxevals);
  // This is usually a redundant style since optimize is already virtual, it's not necessary to add it. In modern C++ it would be:
  ResultMHDouble optimize(ProblemDouble &problem, int maxevals) override;
};
