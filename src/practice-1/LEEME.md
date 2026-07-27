# LEEME - Directorio `software/`

> Práctica 1 Metaheurística - Ismael Sallami Moreno

Este fichero incluye una reseña breve de los ficheros entregados en este directorio.

## Ficheros en la raíz

- `CMakeLists.txt`: Configuración principal de compilación del proyecto con CMake.
- `main.cpp`: Programa principal; ejecuta los experimentos sobre los tres mercados.
- `main`: Binario ejecutable generado al compilar el programa principal.
- `genera_boxplots.py`: Script Python para generar gráficos/boxplots a partir de resultados.
- `LEEME.md`: Documento descriptivo de los ficheros del directorio.

## Ficheros de resultados generados

- `IBEX_35_resultados.csv`: Resultados numéricos de ejecuciones sobre IBEX 35.
- `S_P_100_resultados.csv`: Resultados numéricos de ejecuciones sobre S&P 100.
- `S_P_500_resultados.csv`: Resultados numéricos de ejecuciones sobre S&P 500.
- `IBEX_35_resultados.png`: Gráfico generado para resultados de IBEX 35.
- `IBEX_35_resultados_v1.png`: Variante de gráfico generado para resultados de IBEX 35.
- `S_P_100_resultados.png`: Gráfico generado para resultados de S&P 100.
- `S_P_100_resultados_v1.png`: Variante de gráfico generado para resultados de S&P 100.
- `S_P_500_resultados.png`: Gráfico generado para resultados de S&P 500.
- `S_P_500_resultados_v1.png`: Variante de gráfico generado para resultados de S&P 500.

### Nota sobre la generación de gráficos

Para el análisis estándar exigido en el guion de la práctica se han utilizado los gráficos
originales identificados con el sufijo `v1`, que muestran únicamente los algoritmos base
solicitados.

Para el análisis voluntario ampliado, se han regenerado los diagramas de caja (boxplots)
incluyendo la totalidad de los algoritmos desarrollados (base y voluntarios). Además, se ha
optimizado el formato de estos nuevos gráficos para evitar la saturación visual, permitiendo
una comparativa directa de todas las trayectorias bajo el mismo presupuesto de `10.000`
evaluaciones.

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

## Carpeta `src/` (implementación)

- `src/csv_reader.cpp`: Implementación de lectura y parseo de CSV.
- `src/portfolio_problem.cpp`: Implementación de evaluación/restricciones del portfolio.
- `src/greedy.cpp`: Implementación del algoritmo Greedy.
- `src/localsearch.cpp`: Implementación del algoritmo de búsqueda local.
- `src/localsearch_best.cpp`: Implementación de búsqueda local de mejora (best improvement).
- `src/localsearch_multistart.cpp`: Implementación de búsqueda local multiarranque.

## Carpeta `datos_portfolio_2526/`

- `datos_portfolio_2526/ibex_35.csv`: Datos de entrada del mercado IBEX 35.
- `datos_portfolio_2526/syp_100.csv`: Datos de entrada del mercado S&P 100.
- `datos_portfolio_2526/syp_500.csv`: Datos de entrada del mercado S&P 500.

## Nota sobre artefactos de compilación

Los ficheros y carpetas autogenerados por CMake (por ejemplo `CMakeFiles/`,
`CMakeCache.txt`, `cmake_install.cmake` y `Makefile`) no forman parte del contenido
fuente entregable y no se documentan como parte funcional del proyecto.
