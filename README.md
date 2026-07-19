# MH-Practices

Practices for the **Metaheuristics (MH)** course — University of Granada, 2025-26.

## Practices

| # | Folder | Topic | Algorithms |
|---|--------|-------|------------|
| 1 | `practice-1/` | Portfolio optimization (Markowitz) | Greedy, Local Search |
| 2 | `practice-2/` | Portfolio optimization (Markowitz) | Genetic (AGG, AGE), Memetic (AM) |
| 3 | `practice-3/` | Portfolio optimization (Markowitz) | ILS, Simulated Annealing (ES), CHC |
| 4 | `practice-4/` | CEC'17 benchmark functions | LCA (Ladybug Courtship Algorithm) + variants |

## Repository structure

```text
MH-Practices/
├── README.md
├── LICENSE
├── .gitignore
├── practice-1/
│   ├── informe/          # LaTeX report
│   ├── software/         # C++ source code
│   └── tests/            # Unit tests
├── practice-2/
│   ├── informe/          # LaTeX report
│   └── code/             # C++ source code
├── practice-3/
│   ├── informe/          # LaTeX report
│   ├── code/             # C++ source code
│   └── EXTRA/            # Extra work (plots, scripts)
└── practice-4/
    ├── informe/          # LaTeX report
    └── software/         # C++/C source + Python analysis scripts
```

## Tech stack

- **C++17** + **CMake** — main algorithms and solvers.
- **C** (CEC'17 benchmark) — test functions for practice 4.
- **Python** — result analysis, plots, convergence graphs.
- **LaTeX** — technical reports.

## Build (general)

Each practice has its own `CMakeLists.txt`. The general workflow is:

```bash
cd practice-N/software   # or practice-N/code
mkdir build && cd build
cmake ..
make
```

Check each practice's `LEEME.md` or `README.md` for specific instructions.

## Author

**Ismael Sallami Moreno** — Double Degree in Computer Science and Mathematics, University of Granada.

## License

See [LICENSE](LICENSE).
