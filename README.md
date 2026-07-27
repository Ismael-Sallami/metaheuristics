# metaheuristics

![C++](https://img.shields.io/badge/C%2B%2B-17-00599C)
![CMake](https://img.shields.io/badge/CMake-3.28-064F8C)
[![build](https://img.shields.io/github/actions/workflow/status/Ismael-Sallami/metaheuristics/ci.yml?branch=main&logo=github&label=build)](https://github.com/Ismael-Sallami/metaheuristics/actions/workflows/ci.yml)
![license](https://img.shields.io/badge/license-MIT-4c1)

Fifteen metaheuristics written from scratch in C++ and compared on the same problem under
the same evaluation budget.

## Context

Coursework for **Metaheurísticas**, year 4 of the double degree in Computer Science and
Business Administration, University of Granada (2025-26). Solo work.

## The problem

Practices 1 to 3 solve the same problem: **portfolio selection in the Markowitz sense**.
Given the historical returns of a market, choose the weight of each asset so the portfolio
balances return against risk, with each weight inside a band (0.005 to 0.08 on IBEX 35) and
the weights adding up to one.

Practice 4 changes the ground: the **CEC'17 benchmark** of continuous functions in 10, 30,
50 and 100 dimensions, which is what the field uses to compare optimisers.

The rule that shapes everything: **a fixed budget of evaluations**, the same for every
algorithm, 50 independent runs each. An algorithm is not better because it finds a good
solution, but because it finds it inside the budget.

## The solution

| Practice | Problem | Algorithms |
| --- | --- | --- |
| 1 | Markowitz portfolio | Greedy, local search (first and best improvement), multi-start local search, random search as a floor |
| 2 | Markowitz portfolio | Generational and steady-state genetic algorithms (BLX, arithmetic and Gaussian crossover), three memetic variants, differential evolution |
| 3 | Markowitz portfolio | Iterated local search, simulated annealing, CHC, and the combinations ILS-ES and ILS-ES-CHC |
| 4 | CEC'17 benchmark | Liver Cancer Algorithm (Houssein et al., 2023) and variants, against DE and PSO |

Things worth naming:

- **One shared framework.** `common/` holds the problem and solution abstractions, so every
  algorithm receives a `Problem` and returns a solution: swapping the metaheuristic does not
  touch the problem, and swapping the problem does not touch the metaheuristic.
- **Experiments driven by a file.** `config.cfg` fixes the seed, the evaluation budget, the
  number of runs and the market. Reproducing an experiment means editing that file, not
  recompiling.
- **Every run is written out.** Each practice leaves a CSV with one row per run, which is
  what feeds the box plots and the tables in the reports.
- **Random search is always in the comparison.** It is the floor: a metaheuristic that does
  not beat it is not doing any work.

## Layout

```
src/practice-1..4/     the four practices, each with its own CMakeLists and config.cfg
  common/              problem and solution abstractions shared by the algorithms
  inc/  src/           one header and one file per metaheuristic
  *.csv                one row per run, the raw material of the reports
docs/reports/          the four LaTeX reports, with their figures and tables
```

## Requirements

- CMake 3.28 or later (practice 4 asks for 3.0) and a C++ compiler with C++17.
- Python 3 with `numpy`, `pandas` and `matplotlib`, only for the box plot and analysis
  scripts.

## Build and run

Each practice is an independent project:

```bash
cmake -S src/practice-1 -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j"$(nproc)"
cd src/practice-1 && ../../build/main        # reads config.cfg and the data next to it
```

Practice 4 ships its own script, which builds everything and runs a smoke test:

```bash
cd src/practice-4 && bash probar.sh          # about a minute, exit code 0 if it passes
bash probar.sh --completo                    # the full CEC'17 sweep, 30 to 40 minutes
```

## Results

Ranking on IBEX 35 after 50 runs of each algorithm, from the CSV in `src/practice-3`.
Fitness is maximised, so higher is better:

| Algorithm | Mean | Best |
| --- | --- | --- |
| Memetic, local search on the best (AM-LSCH, AM-BEST) | **-4.4903** | -4.4902 |
| Memetic, random subset (AM-RAND) | -4.4905 | -4.4902 |
| Differential evolution | -4.4906 | -4.4902 |
| Generational genetic, BLX crossover | -4.4975 | -4.4904 |
| … eleven more between these two … | | |
| Greedy | -4.7941 | -4.7941 |
| Random search | -4.9113 | -4.7307 |

The population-based algorithms with a local search on top take the top of the table, and
the gap between them is smaller than the gap from any of them to the greedy solution. Random
search sits last, which is the point of keeping it in the comparison.

The box plots per market and the statistical tests are in `docs/reports/`.

## What I learned

- The budget is the experiment. Once every algorithm gets the same number of evaluations,
  the comparison stops being about who converges lower and becomes about who spends the
  budget better. That reframing is most of what these four practices teach.
- Hybridising helps up to a point. The memetic variants beat their genetic parents, but
  ILS-ES-CHC, which stacks three ideas, lands mid-table: the extra machinery costs
  evaluations that the search then does not have.
- **Limitations, kept as handed in:**
  - **Practice 3 does not compile as published.** Its `CMakeLists.txt` includes `common/`,
    but that folder was never committed for this practice; practices 1, 2 and 4 do have it.
    The CI builds the other three and leaves this one out instead of patching it.
  - The **large datasets are not in the repository**. `syp_100.csv` and `syp_500.csv`
    (33 MB together) and the CEC'17 matrices for 30, 50 and 100 dimensions (39 MB) belong to
    the subject and to the benchmark suite, and are not mine to republish. What stays is
    `ibex_35.csv` and the CEC'17 files up to 20 dimensions, which is enough to run.
  - Identifiers and comments mix English and Spanish, and each practice repeats the
    framework instead of sharing one copy: they were handed in as separate deliverables.

## Author and licence

Ismael Sallami Moreno. Released under the MIT licence (see `LICENSE`).
