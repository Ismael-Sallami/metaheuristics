# LEEME - Directorio `software/`

> Práctica 2 Metaheurística - Ismael Sallami Moreno

Este fichero incluye una reseña breve de los ficheros entregados en este directorio.

## Ficheros en la raíz

- `CMakeLists.txt`: Configuración principal de compilación del proyecto con CMake.
- `main.cpp`: Programa principal; ejecuta los experimentos sobre los tres mercados.
- `main`: Binario ejecutable generado al compilar el programa principal.
- `genera_boxplots.py`: Script Python para generar gráficos/boxplots a partir de resultados. Cabe destacar que este algoritmo se ha mejorado para usar otro diseño y generar varias gráficas teniendo en cuenta el trabajo obligatorio y luego el extra, así como separa en varias imágenes si hay muchos algoritmos a analizar para no sobrecargar las imágenes.
- `LEEME.md`: Documento descriptivo de los ficheros del directorio.
- `config.cfg`: Fichero de configuración con los parámetros de ejecución (semilla, número de evaluaciones, número de ejecuciones y parámetros de AG/AM/DE).

## Carpeta `common/` (framework base)

- `common/mh.h`: Interfaz base de metaheurísticas del framework.
- `common/mhtrayectory.h`: Estructura para gestionar trayectorias/históricos de búsqueda.
- `common/problem.h`: Definición abstracta de problema de optimización.
- `common/solution.h`: Definición de solución genérica del framework.
- `common/random.hpp`: Utilidades de generación aleatoria del framework.
- `common/util.h`: Funciones auxiliares comunes.

## Carpeta `inc/` (cabeceras del proyecto)

- `inc/csv_reader.h`: Lectura de ficheros CSV de datos financieros.
- `inc/portfolio_problem.h`: Definición del problema Portfolio (Markowitz).
- `inc/greedy.h`: Interfaz del algoritmo voraz (Greedy).
- `inc/randomsearch.h`: Interfaz de búsqueda aleatoria.
- `inc/localsearch.h`: Interfaz de búsqueda local.
- `inc/localsearch_best.h`: Interfaz de búsqueda local de mejora (best improvement).
- `inc/localsearch_multistart.h`: Interfaz de búsqueda local multiarranque.
- `inc/pincrem.h`: Definiciones auxiliares para estrategia incremental/constructiva.
- `inc/ea_common.h`: Utilidades comunes para algoritmos evolutivos poblacionales.
- `inc/genetic_algorithm.h`: Interfaz AGG/AGE (Aritmético, BLX, y variante gaussiana).
- `inc/memetic_algorithm.h`: Interfaz de AM-All, AM-Rand y AM-Best.
- `inc/memetic_lsch.h`: Interfaz de AM adaptativo con cadena de BL (LSCh).
- `inc/soft_local_search.h`: Operador de BL suave reutilizable por AM.
- `inc/differential_evolution.h`: Interfaz para DE/rand/1/bin.
- `inc/gaussian_mutation.h`: Operador de mutación gaussiana para extra.

## Carpeta `src/` (implementación)

- `src/csv_reader.cpp`: Implementación de lectura y parseo de CSV.
- `src/portfolio_problem.cpp`: Implementación de evaluación/restricciones del portfolio.
- `src/greedy.cpp`: Greedy algorithm implementation.
- `src/localsearch.cpp`: Local search implementation.
- `src/localsearch_best.cpp`: Best-improvement local search implementation.
- `src/localsearch_multistart.cpp`: Multi-start local search implementation.
- `src/genetic_algorithm.cpp`: AGG/AGE implementation.
- `src/memetic_algorithm.cpp`: AM-All, AM-Rand, and AM-Best implementation.
- `src/memetic_lsch.cpp`: Adaptive AM-LSCh implementation.
- `src/soft_local_search.cpp`: Soft local search implementation for AM.
- `src/differential_evolution.cpp`: DE/rand/1/bin implementation.
- `src/gaussian_mutation.cpp`: Gaussian mutation implementation.

## Carpeta `datos_portfolio_2526/`

- `datos_portfolio_2526/ibex_35.csv`: Input data for the IBEX 35 market.
- `datos_portfolio_2526/syp_100.csv`: Input data for the S&P 100 market.
- `datos_portfolio_2526/syp_500.csv`: Input data for the S&P 500 market.
