#include "greedy.h"
#include <vector>
#include <algorithm>
#include <stdexcept>
#include "portfolio_problem.h" 

using namespace std;

ResultMH<double> GreedySearch::optimize(Problem<double> &problem, int maxevals) {
    // Extract the real problem and its limits
    PortfolioProblem* port_problem = dynamic_cast<PortfolioProblem*>(&problem);
    if (!port_problem) {
        throw runtime_error("Critical Error: The provided problem is not a PortfolioProblem.");
    } // This is only possible to do in greedy

    size_t n = port_problem->getSolutionSize();
    auto limites = port_problem->getSolutionDomainRange();
    double lo = limites.first;
    double hi = limites.second;

    // Create the ranking table
    // Store pairs of {Company_ID, Heuristic}
    vector<pair<int, double>> ranking(n);
    
    // Score each company (The Heuristic)
    for (size_t i = 0; i < n; ++i) {
        // Evaluate this portfolio to see how good this company is by itself
        double heuristica = port_problem->getGreedyHeuristic(i);
        ranking[i] = {static_cast<int>(i), heuristica};
    }

    // Sort the ranking from BEST to WORST score
    sort(ranking.begin(), ranking.end(), 
         [](const pair<int, double>& a, const pair<int, double>& b) {
             return a.second > b.second; 
         });

    // The Greedy: Distribute the budget
    tSolution<double> best_sol(n, 0.0);
    double suma_w = 0.0;

    for (size_t i = 0; i < n; ++i) {
        int id_empresa = ranking[i].first;

        // While we haven't exceeded the total budget of 1.0
        if (suma_w < 1.0 - 1e-8) { 
            // We assign the minimum between the legal limit of the company (hi) 
            // and what we have left of money (1.0 - suma_w)
            double asignar = min(hi, 1.0 - suma_w);
            best_sol[id_empresa] = asignar;
            suma_w += asignar;
        } else {
            break; // Budget exhausted
        }
    }

    // Pass the solution through the repair function to adjust 
    // the minimum limits (lo) if the last insertion was too small.
    port_problem->fix(best_sol);
    // In the script doesn't say to do it, but we have to consider the case where the sum of weights is less than 1, we can assign the remaining budget, for example, we could assign 0,002 and the minimum is 0,005, so it would break the isValid function. This is necessary to do in order to any kind of market and values hi and lo, if we reduce it to this 3 types of market, doing the fix is unnecessary, because for maths, it is impossible.
    // I comment this line, but I want to give it clear, in this case the options are:
    /*
    - IBEX 35: lo = 0,005, hi = 0,08: 1.0/0.08=12.5, 12 companies have the top, the sum is 0.96 and the last will have 0.04>0.005, so we can assign the remaining 0.04 to it without breaking the isValid function.
    - S&P 100: lo = 0,005, hi = 0,05: 1.0/0.05=20, 20 companies have the top, the sum is exactly 1.0, no remainder.
    - S&P 500: lo = 0,005, hi = 0,02: 1.0/0.02=50, 50 companies have the top, the sum is exactly 1.0, no remainder.

    IF WE WANT TO MAKE IT MORE GENERIC:
    12 companies × 0.083 = 0.996
    Remaining budget: 1.0 - 0.996 = 0.004
    ---- Assign it to company 13.
    It passes through isValid(), detects that 0.004 < 0.005 (lo), 
    and program crashes returning an error!
    Solution: apply the repair function to redistribute and ensure validity
    port_problem->fix(best_sol);
    */

    // Calculate the actual score of the complete portfolio and return
    tFitness best_fitness = port_problem->fitness(best_sol);

    return ResultMH<double>(best_sol, best_fitness, 1); // 1 evaluation
}