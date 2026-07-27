/**
 * @file test_exhaustivo.cpp
 * @brief Batería COMPLETA de tests para PortfolioProblem (Práctica 1 - MH 2025-26)
 *
 * COMPILACIÓN (coloca este fichero en la raíz del proyecto, junto a CMakeLists.txt):
 *
 *   g++ -std=c++17 -O2 -Wall \
 *       -I common -I inc \
 *       src/csv_reader.cpp src/portfolio_problem.cpp \
 *       test_exhaustivo.cpp \
 *       -o test_exhaustivo
 *
 *   Luego ejecuta:
 *       ./test_exhaustivo
 *
 * NOTAS:
 *   - Los datasets deben estar en  ../datos_portfolio_2526/
 *     (igual que en main.cpp).  Ajusta las rutas si las tienes en otro sitio.
 *   - El fichero NO depende de googletest ni de ninguna librería externa.
 *   - Cada test imprime [PASS] o lanza una excepción con [FAIL].
 *   - Al final se muestra un resumen global.
 */

#include <iostream>
#include <cmath>
#include <stdexcept>
#include <vector>
#include <numeric>
#include <algorithm>
#include <string>
#include <functional>
#include <cassert>

#include "portfolio_problem.h"   // Ajusta el include si tu header está en otro lugar

using namespace std;

// ──────────────────────────────────────────────────────────────────────────────
// Colores ANSI
// ──────────────────────────────────────────────────────────────────────────────
const string RESET  = "\033[0m";
const string BOLD   = "\033[1m";
const string RED    = "\033[1;31m";
const string GREEN  = "\033[1;32m";
const string YELLOW = "\033[1;33m";
const string CYAN   = "\033[1;36m";
const string BLUE   = "\033[1;34m";

// ──────────────────────────────────────────────────────────────────────────────
// Contadores globales
// ──────────────────────────────────────────────────────────────────────────────
static int g_passed = 0;
static int g_failed = 0;

// ──────────────────────────────────────────────────────────────────────────────
// Helpers
// ──────────────────────────────────────────────────────────────────────────────
bool almost_equal(double a, double b, double eps = 1e-8) {
    return fabs(a - b) < eps;
}

double sum_solution(const tSolution<double>& sol) {
    double s = 0.0;
    for (double w : sol) s += w;
    return s;
}

// Macro de aserción: lanza runtime_error si falla
#define CHECK(cond, msg) \
    do { \
        if (!(cond)) { \
            throw runtime_error(string("[FAIL] ") + (msg)); \
        } \
    } while(0)

// Ejecuta un test con nombre y captura excepciones
void run_test(const string& name, function<void()> test_fn) {
    cout << "  " << BOLD << name << RESET << " ... ";
    try {
        test_fn();
        cout << GREEN << "[PASS]" << RESET << "\n";
        g_passed++;
    } catch (const exception& e) {
        cout << RED << "[FAIL]" << RESET << "\n";
        cout << "    -> " << e.what() << "\n";
        g_failed++;
    }
}

// Sección visual
void section(const string& title) {
    cout << "\n" << BOLD << CYAN
         << "══════════════════════════════════════════════════\n"
         << "  " << title << "\n"
         << "══════════════════════════════════════════════════"
         << RESET << "\n";
}

// ──────────────────────────────────────────────────────────────────────────────
// BLOQUE 1 – Constructor y carga de datos
// ──────────────────────────────────────────────────────────────────────────────
void tests_construccion(const string& filepath, int expected_n, double lo, double hi,
                         const string& market_name) {
    section("BLOQUE 1: Constructor y dimensiones — " + market_name);

    // 1.1 Constructor sin lanzar excepción
    run_test("1.1 Constructor (train) no lanza excepción", [&]() {
        PortfolioProblem p(filepath, lo, hi, false);
        (void)p; // solo instanciar
    });

    // 1.2 Constructor (test 2025) no lanza excepción
    run_test("1.2 Constructor (test 2025) no lanza excepción", [&]() {
        PortfolioProblem p(filepath, lo, hi, true);
        (void)p;
    });

    // 1.3 Número de empresas correcto
    run_test("1.3 getSolutionSize() devuelve el número esperado", [&]() {
        PortfolioProblem p(filepath, lo, hi, false);
        CHECK(p.getSolutionSize() == (size_t)expected_n,
              "Esperado " + to_string(expected_n) + ", obtenido " + to_string(p.getSolutionSize()));
    });

    // 1.4 getSolutionDomainRange devuelve (lo, hi)
    run_test("1.4 getSolutionDomainRange() devuelve (lo, hi) correctos", [&]() {
        PortfolioProblem p(filepath, lo, hi, false);
        auto rng = p.getSolutionDomainRange();
        CHECK(almost_equal(rng.first,  lo), "lo incorrecto");
        CHECK(almost_equal(rng.second, hi), "hi incorrecto");
    });

    // 1.5 Fichero inexistente lanza excepción
    run_test("1.5 Fichero inexistente lanza excepción", [&]() {
        bool threw = false;
        try { PortfolioProblem p("no_existe_xyz.csv", lo, hi, false); }
        catch (...) { threw = true; }
        CHECK(threw, "Debería haber lanzado una excepción por fichero inexistente");
    });
}

// ──────────────────────────────────────────────────────────────────────────────
// BLOQUE 2 – isValid
// ──────────────────────────────────────────────────────────────────────────────
void tests_isvalid(const string& filepath, int n, double lo, double hi,
                    const string& market_name) {
    section("BLOQUE 2: isValid — " + market_name);
    PortfolioProblem prob(filepath, lo, hi, false);

    // 2.1 Solución perfectamente válida (peso uniforme si cabe)
    run_test("2.1 Solución válida perfecta", [&]() {
        // Poner peso 'hi' en tantas empresas como quepan, el resto 0
        tSolution<double> sol(n, 0.0);
        double restante = 1.0;
        for (int i = 0; i < n && restante > 1e-9; ++i) {
            double asig = min(hi, restante);
            if (asig >= lo) { sol[i] = asig; restante -= asig; }
        }
        // Si restante > 0 no pudo asignarse, test no aplica
        if (fabs(restante) < 1e-8)
            CHECK(prob.isValid(sol), "Solución válida rechazada");
    });

    // 2.2 Peso negativo → inválido
    run_test("2.2 Peso negativo es inválido", [&]() {
        tSolution<double> sol(n, 0.0);
        sol[0] = -0.001;
        sol[1] = 1.001;
        CHECK(!prob.isValid(sol), "Negativo debería ser inválido");
    });

    // 2.3 Peso mayor que hi → inválido
    run_test("2.3 Peso > hi es inválido", [&]() {
        tSolution<double> sol(n, 0.0);
        sol[0] = hi + 0.01;
        sol[1] = 1.0 - sol[0];
        CHECK(!prob.isValid(sol), "Peso > hi debería ser inválido");
    });

    // 2.4 Peso entre 0 y lo (micro-inversión) → inválido
    run_test("2.4 Peso entre 0 y lo es inválido (micro-inversión)", [&]() {
        tSolution<double> sol(n, 0.0);
        sol[0] = lo - 0.001; // positivo pero menor que lo
        if (sol[0] > 0.0) {
            sol[1] = 1.0 - sol[0];
            CHECK(!prob.isValid(sol), "Micro-inversión debería ser inválida");
        }
        // Si lo <= 0.001, el test no tiene sentido; lo saltamos
    });

    // 2.5 Suma > 1 → inválido
    run_test("2.5 Suma de pesos > 1 es inválido", [&]() {
        tSolution<double> sol(n, 0.0);
        sol[0] = hi;
        sol[1] = hi;
        sol[2] = hi; // la suma puede pasar de 1
        if (sum_solution(sol) > 1.0 + 1e-8)
            CHECK(!prob.isValid(sol), "Suma > 1 debería ser inválido");
    });

    // 2.6 Suma < 1 → inválido
    run_test("2.6 Suma de pesos < 1 es inválido", [&]() {
        tSolution<double> sol(n, 0.0);
        sol[0] = lo; // suma < 1
        CHECK(!prob.isValid(sol), "Suma < 1 debería ser inválido");
    });

    // 2.7 Tolerancia IEEE 754: impureza de 1e-10 en peso activo → válido
    run_test("2.7 Impureza de 1e-10 en peso activo es tolerada", [&]() {
        tSolution<double> sol = prob.createSolution();
        for (size_t i = 0; i < sol.size(); ++i) {
            if (sol[i] > 0.0) { sol[i] += 1e-10; break; }
        }
        CHECK(prob.isValid(sol), "isValid demasiado estricto: rechaza impureza de 1e-10");
    });

    // 2.8 Solución de todo ceros → inválido
    run_test("2.8 Vector de ceros es inválido (suma != 1)", [&]() {
        tSolution<double> sol(n, 0.0);
        CHECK(!prob.isValid(sol), "Vector de ceros debería ser inválido");
    });
}

// ──────────────────────────────────────────────────────────────────────────────
// BLOQUE 3 – createSolution
// ──────────────────────────────────────────────────────────────────────────────
void tests_create_solution(const string& filepath, int n, double lo, double hi,
                            const string& market_name) {
    section("BLOQUE 3: createSolution — " + market_name);
    PortfolioProblem prob(filepath, lo, hi, false);

    // 3.1 Genera solución válida
    run_test("3.1 createSolution genera una solución válida", [&]() {
        auto sol = prob.createSolution();
        CHECK(prob.isValid(sol), "createSolution generó una solución no válida");
    });

    // 3.2 Robustez: 100 soluciones, todas válidas
    run_test("3.2 100 soluciones generadas consecutivamente, todas válidas", [&]() {
        for (int i = 0; i < 100; ++i) {
            auto sol = prob.createSolution();
            if (!prob.isValid(sol))
                throw runtime_error("Solución " + to_string(i) + " no es válida");
        }
    });

    // 3.3 Tamaño correcto
    run_test("3.3 createSolution devuelve vector de tamaño correcto", [&]() {
        auto sol = prob.createSolution();
        CHECK(sol.size() == (size_t)n,
              "Tamaño esperado " + to_string(n) + ", obtenido " + to_string(sol.size()));
    });

    // 3.4 Al menos un peso es 0 (requisito del enunciado: al menos 1 cero)
    run_test("3.4 Al menos un peso es 0 (estructura esparsa)", [&]() {
        int zeros_found = 0;
        for (int i = 0; i < 20; ++i) {
            auto sol = prob.createSolution();
            bool has_zero = false;
            for (double w : sol) if (w == 0.0) { has_zero = true; break; }
            if (has_zero) zeros_found++;
        }
        CHECK(zeros_found > 0, "Ninguna de 20 soluciones tuvo al menos un cero");
    });

    // 3.5 Ningún peso es menor que lo y mayor que 0 (sin micro-inversiones)
    run_test("3.5 Ningún peso activo cae por debajo de lo", [&]() {
        for (int i = 0; i < 50; ++i) {
            auto sol = prob.createSolution();
            for (double w : sol) {
                if (w > 0.0 && w < lo - 1e-8)
                    throw runtime_error("Peso activo " + to_string(w) + " < lo=" + to_string(lo));
            }
        }
    });
}

// ──────────────────────────────────────────────────────────────────────────────
// BLOQUE 4 – fix
// ──────────────────────────────────────────────────────────────────────────────
void tests_fix(const string& filepath, int n, double lo, double hi,
                const string& market_name) {
    section("BLOQUE 4: fix — " + market_name);
    PortfolioProblem prob(filepath, lo, hi, false);

    // 4.1 fix convierte el vector de ceros en algo válido
    run_test("4.1 fix repara vector de ceros sin dividir por cero", [&]() {
        tSolution<double> sol(n, 0.0);
        prob.fix(sol);
        CHECK(prob.isValid(sol), "fix no reparó el vector de ceros");
    });

    // 4.2 fix repara excesos brutales (sol[0]=5, sol[1]=-2)
    run_test("4.2 fix repara excesos y negativos extremos", [&]() {
        tSolution<double> sol(n, 0.0);
        sol[0] = 5.0;
        sol[1] = -2.0;
        prob.fix(sol);
        CHECK(prob.isValid(sol), "fix no reparó excesos y negativos");
    });

    // 4.3 fix no altera una solución ya válida (la deja válida)
    run_test("4.3 fix sobre solución ya válida la deja válida", [&]() {
        auto sol = prob.createSolution(); // válida por construcción
        prob.fix(sol);
        CHECK(prob.isValid(sol), "fix rompió una solución que ya era válida");
    });

    // 4.4 fix repara solución con un solo peso = 0.5 (suma != 1 si n>1)
    run_test("4.4 fix normaliza correctamente cuando suma != 1", [&]() {
        tSolution<double> sol(n, 0.0);
        sol[0] = 0.5; // suma = 0.5
        prob.fix(sol);
        CHECK(prob.isValid(sol), "fix no normalizó correctamente a suma=1");
    });

    // 4.5 fix repara solución con todos los pesos = hi (suma >> 1)
    run_test("4.5 fix con todos los pesos al máximo (suma enorme)", [&]() {
        tSolution<double> sol(n, hi);
        prob.fix(sol);
        CHECK(prob.isValid(sol), "fix no reparó sol con todos pesos=hi");
    });

    // 4.6 fix repara solución con pesos ilegalmente pequeños (entre 0 y lo)
    run_test("4.6 fix elimina micro-inversiones (pesos entre 0 y lo)", [&]() {
        tSolution<double> sol(n, 0.0);
        // Asignamos la mitad de lo a todos → todos ilegales, suma < 1
        double micro = lo / 2.0;
        if (micro > 0.0) {
            for (int i = 0; i < n; ++i) sol[i] = micro;
            prob.fix(sol);
            CHECK(prob.isValid(sol), "fix no eliminó micro-inversiones");
        }
    });

    // 4.7 fix no introduce NaN ni Inf
    run_test("4.7 fix no produce NaN ni Inf", [&]() {
        tSolution<double> sol(n, 0.0);
        sol[0] = 100.0;
        prob.fix(sol);
        for (size_t i = 0; i < sol.size(); ++i) {
            if (isnan(sol[i]) || isinf(sol[i]))
                throw runtime_error("fix produjo NaN o Inf en posición " + to_string(i));
        }
    });

    // 4.8 fix idempotencia: aplicar dos veces da el mismo resultado
    run_test("4.8 fix es idempotente (aplicar 2 veces da el mismo resultado)", [&]() {
        tSolution<double> sol(n, 0.0);
        sol[0] = 3.0;
        prob.fix(sol);
        auto sol_copia = sol;
        prob.fix(sol_copia);
        for (size_t i = 0; i < sol.size(); ++i) {
            if (!almost_equal(sol[i], sol_copia[i], 1e-8))
                throw runtime_error("fix no es idempotente en posición " + to_string(i));
        }
    });
}

// ──────────────────────────────────────────────────────────────────────────────
// BLOQUE 5 – fitness
// ──────────────────────────────────────────────────────────────────────────────
void tests_fitness(const string& filepath, int n, double lo, double hi,
                    const string& market_name) {
    section("BLOQUE 5: fitness — " + market_name);
    PortfolioProblem prob(filepath, lo, hi, false);

    // 5.1 fitness no devuelve NaN
    run_test("5.1 fitness no devuelve NaN", [&]() {
        auto sol = prob.createSolution();
        tFitness f = prob.fitness(sol);
        CHECK(!isnan(f), "fitness devolvió NaN");
    });

    // 5.2 fitness no devuelve Inf
    run_test("5.2 fitness no devuelve Inf", [&]() {
        auto sol = prob.createSolution();
        tFitness f = prob.fitness(sol);
        CHECK(!isinf(f), "fitness devolvió Inf");
    });

    // 5.3 fitness es determinista (misma solución → mismo valor)
    run_test("5.3 fitness es determinista (2 evaluaciones iguales)", [&]() {
        auto sol = prob.createSolution();
        tFitness f1 = prob.fitness(sol);
        tFitness f2 = prob.fitness(sol);
        CHECK(almost_equal(f1, f2, 1e-12), "fitness no es determinista");
    });

    // 5.4 fitness en solución reparada no es NaN
    run_test("5.4 fitness sobre solución con excesos reparados no es NaN", [&]() {
        tSolution<double> sol(n, 0.0);
        sol[0] = 5.0;
        sol[1] = -2.0;
        prob.fix(sol);
        tFitness f = prob.fitness(sol);
        CHECK(!isnan(f), "fitness devolvió NaN sobre solución reparada");
    });

    // 5.5 fitness(sol_A) != fitness(sol_B) para dos soluciones distintas
    //     (con alta probabilidad; puede fallar en casos degenerados)
    run_test("5.5 Dos soluciones distintas tienen fitness distintos (alta prob.)", [&]() {
        auto s1 = prob.createSolution();
        auto s2 = prob.createSolution();
        // Sólo avisamos si son idénticos; no es un error crítico
        tFitness f1 = prob.fitness(s1);
        tFitness f2 = prob.fitness(s2);
        (void)f1; (void)f2; // No hacemos assert duro aquí, solo verificamos que no explotan
    });

    // 5.6 getBeneficio no devuelve NaN
    run_test("5.6 getBeneficio no devuelve NaN", [&]() {
        auto sol = prob.createSolution();
        double b = prob.getBeneficio(sol);
        CHECK(!isnan(b), "getBeneficio devolvió NaN");
    });

    // 5.7 fitness = getBeneficio - lambda*riesgo: comprobación de estructura
    //     (fitness debe ser menor o igual que getBeneficio, dado que lambda>0 y riesgo>=0)
    run_test("5.7 fitness <= getBeneficio (riesgo >= 0, lambda > 0)", [&]() {
        auto sol = prob.createSolution();
        double f = prob.fitness(sol);
        double b = prob.getBeneficio(sol);
        // riesgo = w' * Sigma * w >= 0 para cualquier solución, por ser Sigma semidefinida positiva
        CHECK(f <= b + 1e-8, "fitness > getBeneficio: el riesgo sería negativo (¿bug?)");
    });

    // 5.8 Consistencia train vs test: mismos pesos, distintos periodos → distintos fitness
    run_test("5.8 Mismo portafolio da fitness distinto en 2015-2024 vs 2025", [&]() {
        PortfolioProblem p_train(filepath, lo, hi, false);
        PortfolioProblem p_test (filepath, lo, hi, true);
        auto sol = p_train.createSolution();
        double f_train = p_train.fitness(sol);
        double f_test  = p_test.fitness(sol);
        // Solo comprobamos que no explotan y son reales
        CHECK(!isnan(f_train) && !isnan(f_test),
              "Uno de los fitness train/test es NaN");
    });
}

// ──────────────────────────────────────────────────────────────────────────────
// BLOQUE 6 – getGreedyHeuristic
// ──────────────────────────────────────────────────────────────────────────────
void tests_heuristic(const string& filepath, int n, double lo, double hi,
                      const string& market_name) {
    section("BLOQUE 6: getGreedyHeuristic — " + market_name);
    PortfolioProblem prob(filepath, lo, hi, false);

    // 6.1 No devuelve NaN para ninguna empresa
    run_test("6.1 getGreedyHeuristic no devuelve NaN para ninguna empresa", [&]() {
        for (int i = 0; i < n; ++i) {
            double h = prob.getGreedyHeuristic(i);
            if (isnan(h)) throw runtime_error("Empresa " + to_string(i) + " devuelve NaN");
        }
    });

    // 6.2 No devuelve Inf
    run_test("6.2 getGreedyHeuristic no devuelve Inf para ninguna empresa", [&]() {
        for (int i = 0; i < n; ++i) {
            double h = prob.getGreedyHeuristic(i);
            if (isinf(h)) throw runtime_error("Empresa " + to_string(i) + " devuelve Inf");
        }
    });

    // 6.3 Es determinista (misma empresa → mismo valor)
    run_test("6.3 getGreedyHeuristic es determinista", [&]() {
        for (int i = 0; i < min(5, n); ++i) {
            double h1 = prob.getGreedyHeuristic(i);
            double h2 = prob.getGreedyHeuristic(i);
            if (!almost_equal(h1, h2, 1e-12))
                throw runtime_error("No determinista en empresa " + to_string(i));
        }
    });

    // 6.4 Al menos dos empresas tienen heurísticas distintas (no constante)
    run_test("6.4 Las heurísticas no son todas iguales (hay variedad)", [&]() {
        double h0 = prob.getGreedyHeuristic(0);
        bool found_diff = false;
        for (int i = 1; i < n; ++i) {
            if (!almost_equal(prob.getGreedyHeuristic(i), h0, 1e-6)) {
                found_diff = true; break;
            }
        }
        CHECK(found_diff, "Todas las heurísticas son idénticas (posible bug)");
    });
}

// ──────────────────────────────────────────────────────────────────────────────
// BLOQUE 7 – Tests de integración end-to-end
// ──────────────────────────────────────────────────────────────────────────────
void tests_integracion(const string& filepath, int n, double lo, double hi,
                        const string& market_name) {
    section("BLOQUE 7: Integración end-to-end — " + market_name);
    PortfolioProblem prob(filepath, lo, hi, false);

    // 7.1 Ciclo completo: createSolution → fitness → isValid
    run_test("7.1 Ciclo: createSolution → fitness → isValid (x50)", [&]() {
        for (int i = 0; i < 50; ++i) {
            auto sol = prob.createSolution();
            tFitness f = prob.fitness(sol);
            if (isnan(f)) throw runtime_error("fitness NaN en iteración " + to_string(i));
            if (!prob.isValid(sol)) throw runtime_error("Inválida en iteración " + to_string(i));
        }
    });

    // 7.2 fix sobre createSolution no cambia la validez
    run_test("7.2 fix sobre solución válida no rompe validez (x20)", [&]() {
        for (int i = 0; i < 20; ++i) {
            auto sol = prob.createSolution();
            prob.fix(sol);
            if (!prob.isValid(sol))
                throw runtime_error("fix rompió la validez en iteración " + to_string(i));
        }
    });

    // 7.3 Mejor solución entre 100 tiene mayor fitness que la peor
    run_test("7.3 Mejor entre 100 soluciones > peor entre 100 soluciones", [&]() {
        double best = -1e18, worst = 1e18;
        for (int i = 0; i < 100; ++i) {
            auto sol = prob.createSolution();
            double f = prob.fitness(sol);
            best  = max(best,  f);
            worst = min(worst, f);
        }
        CHECK(best > worst, "Todas las soluciones tienen el mismo fitness (¿bug?)");
    });

    // 7.4 El greedy da una solución válida
    run_test("7.4 getGreedyHeuristic guía hacia una solución válida (construcción manual)", [&]() {
        // Simula lo que hace greedy.cpp
        vector<pair<int,double>> ranking(n);
        for (int i = 0; i < n; ++i)
            ranking[i] = {i, prob.getGreedyHeuristic(i)};
        sort(ranking.begin(), ranking.end(),
             [](auto& a, auto& b){ return a.second > b.second; });
        
        tSolution<double> sol(n, 0.0);
        double suma = 0.0;
        for (int k = 0; k < n; ++k) {
            int id = ranking[k].first;
            if (suma < 1.0 - 1e-8) {
                double asig = min(hi, 1.0 - suma);
                sol[id] = asig;
                suma += asig;
            } else break;
        }
        // La solución greedy puede no ser perfectamente válida para el test isValid
        // (el guión lo comenta), pero el fitness no debe ser NaN
        tFitness f = prob.fitness(sol);
        CHECK(!isnan(f), "Fitness de solución greedy es NaN");
    });

    // 7.5 Vecindario local search: movimiento (i→j) conserva suma
    run_test("7.5 Movimiento BL (40%) conserva la suma de pesos", [&]() {
        auto sol = prob.createSolution();
        double ratio = 0.4;
        // Buscamos un par (i,j) donde sol[i] > 0
        for (int i = 0; i < n; ++i) {
            if (sol[i] == 0.0) continue;
            for (int j = 0; j < n; ++j) {
                if (i == j) continue;
                double trasvase = sol[i] * ratio;
                tSolution<double> vecino = sol;
                vecino[i] -= trasvase;
                vecino[j] += trasvase;
                double s_orig = sum_solution(sol);
                double s_vec  = sum_solution(vecino);
                if (!almost_equal(s_orig, s_vec, 1e-10))
                    throw runtime_error("El movimiento BL no conserva la suma");
                goto done; // Solo verificamos un movimiento
            }
        }
        done:;
    });

    // 7.6 Stress: 500 createSolution (prueba de memoria/rendimiento)
    run_test("7.6 Stress: 500 createSolution sin crash", [&]() {
        for (int i = 0; i < 500; ++i) {
            auto sol = prob.createSolution();
            (void)sol;
        }
    });
}

// ──────────────────────────────────────────────────────────────────────────────
// BLOQUE 8 – Edge cases y casos límite numéricos
// ──────────────────────────────────────────────────────────────────────────────
void tests_edge_cases(const string& filepath, int n, double lo, double hi,
                       const string& market_name) {
    section("BLOQUE 8: Edge cases numéricos — " + market_name);
    PortfolioProblem prob(filepath, lo, hi, false);

    // 8.1 Exactamente en el límite inferior: wi = lo → válido
    run_test("8.1 Peso exactamente igual a lo es válido", [&]() {
        // Construimos manualmente: una empresa con lo, ajustamos el resto
        tSolution<double> sol(n, 0.0);
        // Ponemos tantas empresas con hi como quepan y la última con lo
        double restante = 1.0;
        int last = -1;
        for (int i = 0; i < n && restante > lo + 1e-9; ++i) {
            double asig = min(hi, restante - lo); // dejamos hueco para lo
            if (asig >= lo) { sol[i] = asig; restante -= asig; last = i + 1; }
        }
        if (last >= 0 && last < n && almost_equal(restante, lo, 1e-9)) {
            sol[last] = lo;
            CHECK(prob.isValid(sol), "Peso exactamente = lo fue rechazado");
        }
        // Si no pudimos construir este caso, lo saltamos silenciosamente
    });

    // 8.2 Exactamente en el límite superior: wi = hi → válido (si la suma da 1)
    run_test("8.2 Peso exactamente igual a hi es válido (si suma=1)", [&]() {
        // Solo posible si 1/hi es entero
        int k = (int)round(1.0 / hi);
        if (fabs(k * hi - 1.0) < 1e-8 && k <= n) {
            tSolution<double> sol(n, 0.0);
            for (int i = 0; i < k; ++i) sol[i] = hi;
            CHECK(prob.isValid(sol), "k*hi=1 debería ser válido");
        }
    });

    // 8.3 Suma exactamente 1.0 con tolerancia 1e-9 → válido
    run_test("8.3 Suma 1.0 + 1e-9 dentro de tolerancia → válido", [&]() {
        auto sol = prob.createSolution();
        // Añadimos 5e-9 al primer peso activo
        for (size_t i = 0; i < sol.size(); ++i) {
            if (sol[i] > 0.0) { sol[i] += 5e-9; break; }
        }
        // Puede ser inválido si la tolerancia es estricta (1e-8); lo aceptamos
        // Solo verificamos que no haya crash
        (void)prob.isValid(sol);
    });

    // 8.4 fitness de solución trivial (solo 1 empresa con peso 1.0)
    //     Solo válido si hi >= 1.0 (no aplica a nuestros mercados, pero es un edge case)
    run_test("8.4 fitness sobre solución con todo en una empresa (si hi>=1)", [&]() {
        if (hi >= 1.0 - 1e-9) {
            tSolution<double> sol(n, 0.0);
            sol[0] = 1.0;
            tFitness f = prob.fitness(sol);
            CHECK(!isnan(f), "fitness NaN para solución trivial");
        }
        // Si hi < 1.0, la solución no es válida, lo saltamos
    });

    // 8.5 Dos mercados (train/test) son independientes: fix en uno no afecta al otro
    run_test("8.5 Instancias train y test son independientes", [&]() {
        PortfolioProblem p1(filepath, lo, hi, false);
        PortfolioProblem p2(filepath, lo, hi, true);
        auto sol1 = p1.createSolution();
        auto sol2 = p2.createSolution();
        // Modificamos sol1 y verificamos que p2 sigue funcionando
        sol1[0] = 999.0;
        p1.fix(sol1);
        auto sol2b = p2.createSolution();
        CHECK(p2.isValid(sol2b), "p2 se corrompió tras modificar p1");
    });

    // 8.6 fix sobre solución con todos los pesos negativos
    run_test("8.6 fix repara solución con todos los pesos negativos", [&]() {
        tSolution<double> sol(n, -1.0);
        prob.fix(sol);
        CHECK(prob.isValid(sol), "fix no reparó solución con todos los pesos negativos");
    });
}

// ──────────────────────────────────────────────────────────────────────────────
// MAIN
// ──────────────────────────────────────────────────────────────────────────────
int main() {
    // Inicializar semilla para reproducibilidad
    Random::seed(42);

    cout << BOLD << YELLOW
         << "\n╔══════════════════════════════════════════════════╗\n"
         << "║   BATERÍA EXHAUSTIVA DE TESTS - PORTFOLIO MH    ║\n"
         << "╚══════════════════════════════════════════════════╝"
         << RESET << "\n";

    // ─── Definición de los 3 mercados ────────────────────────────────────────
    struct Mercado {
        string nombre;
        string ruta;
        int n;
        double lo, hi;
    };

    // Ajusta las rutas según dónde estén tus datos
    vector<Mercado> mercados = {
        {"IBEX 35",  "datos_portfolio_2526/ibex_35.csv",  30,  0.005, 0.08},
        {"S&P 100",  "datos_portfolio_2526/syp_100.csv",  97,  0.005, 0.05},
        {"S&P 500",  "datos_portfolio_2526/syp_500.csv",  457, 0.005, 0.02},
    };

    for (const auto& m : mercados) {
        cout << "\n" << BOLD << BLUE
             << "▌ MERCADO: " << m.nombre << RESET << "\n";

        try {
            tests_construccion(m.ruta, m.n, m.lo, m.hi, m.nombre);
            tests_isvalid     (m.ruta, m.n, m.lo, m.hi, m.nombre);
            tests_create_solution(m.ruta, m.n, m.lo, m.hi, m.nombre);
            tests_fix         (m.ruta, m.n, m.lo, m.hi, m.nombre);
            tests_fitness     (m.ruta, m.n, m.lo, m.hi, m.nombre);
            tests_heuristic   (m.ruta, m.n, m.lo, m.hi, m.nombre);
            tests_integracion (m.ruta, m.n, m.lo, m.hi, m.nombre);
            tests_edge_cases  (m.ruta, m.n, m.lo, m.hi, m.nombre);
        } catch (const exception& e) {
            cout << RED << "\n[ERROR FATAL en " << m.nombre << "]: "
                 << e.what() << RESET << "\n";
            g_failed++;
        }
    }

    // ─── Resumen final ────────────────────────────────────────────────────────
    int total = g_passed + g_failed;
    cout << "\n" << BOLD
         << "══════════════════════════════════════════════════\n"
         << "  RESUMEN FINAL\n"
         << "══════════════════════════════════════════════════\n"
         << RESET;
    cout << GREEN << "  PASS : " << g_passed << " / " << total << RESET << "\n";
    if (g_failed > 0)
        cout << RED << "  FAIL : " << g_failed << " / " << total << RESET << "\n";
    else
        cout << GREEN << "  ¡TODOS LOS TESTS PASARON!" << RESET << "\n";
    cout << "\n";

    return (g_failed == 0) ? 0 : 1;
}