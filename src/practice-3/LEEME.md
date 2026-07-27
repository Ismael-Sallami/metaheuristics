# LEEME - Directorio `software/`

> Práctica 3 Metaheurística - Ismael Sallami Moreno

Este fichero incluye una reseña breve de los ficheros entregados en este directorio.

## Ficheros en la raíz

- `CMakeLists.txt`: Configuración principal de compilación del proyecto con CMake.
- `main.cpp`: Programa principal; ejecuta los experimentos de la Práctica 3 sobre los tres mercados.
- `main`: Binario ejecutable generado al compilar el programa principal.
- `genera_boxplots.py`: Script Python para generar gráficos/boxplots de distribución a partir de los resultados CSV de los algoritmos.
- `eficiencia_datos.py`: Script Python de análisis secundario de datos de eficiencia.
- `LEEME.md`: Documento descriptivo de los ficheros del directorio (este fichero).
- `config.cfg`: Fichero de configuración con los parámetros de ejecución (semilla, número de evaluaciones, número de ejecuciones y parámetros específicos de ES, BMB, ILS, ILS-ES, CHC/GADEGD).

## Carpeta `common/` (framework base)

- `common/mh.h`: Interfaz base de metaheurísticas del framework.
- `common/mhtrayectory.h`: Estructura para gestionar trayectorias/históricos de búsqueda.
- `common/problem.h`: Definición abstracta de problema de optimización.
- `common/solution.h`: Definición de solución genérica del framework.
- `common/random.hpp`: Utilidades de generación aleatoria del framework.
- `common/util.h`: Funciones auxiliares comunes.

## Carpeta `inc/` (cabeceras del proyecto)

- `inc/csv_reader.h`: Lectura de ficheros CSV de datos financieros.
- `inc/config_reader.h`: Utilidad para parsear el fichero de configuración de parámetros `config.cfg`.
- `inc/portfolio_problem.h`: Definición del problema Portfolio (Markowitz).
- `inc/greedy.h`: Interfaz del algoritmo voraz (Greedy).
- `inc/randomsearch.h`: Interfaz de búsqueda aleatoria.
- `inc/localsearch.h`: Interfaz de búsqueda local.
- `inc/localsearch_best.h`: Interfaz de búsqueda local de primer mejor.
- `inc/localsearch_multistart.h`: Interfaz de búsqueda local multiarranque.
- `inc/simulated_annealing.h`: Interfaz del algoritmo Enfriamiento Simulado (ES).
- `inc/bmb.h`: Interfaz del algoritmo Búsqueda Multiarranque Básica (BMB).
- `inc/ils.h`: Interfaz de la Búsqueda Local Reiterada (ILS).
- `inc/ils_es.h`: Interfaz de la hibridación ILS con Enfriamiento Simulado (ILS-ES).
- `inc/ils_es_chc.h`: Interfaz del trabajo extra voluntario: ILS-ES con reinicialización por catástrofe CHC.
- `inc/pincrem.h`: Definiciones auxiliares para estrategia incremental/constructiva.
- `inc/ea_common.h`: Utilidades comunes para algoritmos evolutivos poblacionales (Práctica 2).
- `inc/genetic_algorithm.h`: Interfaz AGG/AGE (Práctica 2).
- `inc/memetic_algorithm.h`: Interfaz de AM-All, AM-Rand y AM-Best (Práctica 2).
- `inc/memetic_lsch.h`: Interfaz de AM adaptativo con cadena de BL (LSCh) (Práctica 2).
- `inc/soft_local_search.h`: Operador de BL suave reutilizable por AM.
- `inc/differential_evolution.h`: Interfaz para DE/rand/1/bin (Práctica 2).
- `inc/gaussian_mutation.h`: Operador de mutación gaussiana (Práctica 2).

## Carpeta `src/` (implementación)

- `src/csv_reader.cpp`: Implementación de lectura y parseo de CSV.
- `src/config_reader.cpp`: Implementación del lector del archivo de configuración.
- `src/portfolio_problem.cpp`: Implementación de evaluación/restricciones del portfolio.
- `src/greedy.cpp`: Implementación del algoritmo Greedy.
- `src/localsearch.cpp`: Implementación de búsqueda local.
- `src/localsearch_best.cpp`: Implementación de búsqueda local de primer mejor.
- `src/localsearch_multistart.cpp`: Implementación de búsqueda local multiarranque.
- `src/simulated_annealing.cpp`: Implementación de Enfriamiento Simulado (ES) Cauchy.
- `src/bmb.cpp`: Implementación de Búsqueda Multiarranque Básica (BMB).
- `src/ils.cpp`: Implementación de Búsqueda Local Reiterada (ILS) con mutación por barajado.
- `src/ils_es.cpp`: Implementación de la hibridación ILS con ES (ILS-ES).
- `src/ils_es_chc.cpp`: Implementación del algoritmo voluntario (ILS-ES con catástrofe CHC).
- `src/genetic_algorithm.cpp`: Implementación de AGG/AGE.
- `src/memetic_algorithm.cpp`: Implementación de AM-All, AM-Rand, y AM-Best.
- `src/memetic_lsch.cpp`: Implementación de AM adaptativo con LSCh.
- `src/soft_local_search.cpp`: Implementación de BL suave para AM.
- `src/differential_evolution.cpp`: Implementación de Evolución Diferencial.
- `src/gaussian_mutation.cpp`: Implementación de mutación gaussiana.

## Carpeta `datos_portfolio_2526/`

- `datos_portfolio_2526/ibex_35.csv`: Datos históricos de cotización del IBEX 35 (30 empresas).
- `datos_portfolio_2526/syp_100.csv`: Datos históricos de cotización del S&P 100 (97 empresas).
- `datos_portfolio_2526/syp_500.csv`: Datos históricos de cotización del S&P 500 (457 empresas).
