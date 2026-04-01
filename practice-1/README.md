# Práctica 1 - Metaheurística (Problema del Portfolio)

Proyecto de la asignatura **Metaheurística (UGR, 2025-26)** centrado en la optimización de carteras tipo **Markowitz**.

El repositorio contiene:
- **software/**: implementación en C++ de los algoritmos y ejecución de experimentos.
- **tests/**: batería de pruebas para validar `PortfolioProblem`.
- **informe/**: memoria en LaTeX con formulación, diseño, experimentos y trabajo voluntario.

---

## 1) Estructura del proyecto

```text
practice-1/
├── informe/                 # Memoria en LaTeX
├── software/                # Código fuente C++ y utilidades
│   ├── common/              # Framework base de metaheurísticas
│   ├── inc/                 # Cabeceras del proyecto
│   ├── src/                 # Implementaciones
│   ├── datos_portfolio_2526/# Datasets (IBEX35, S&P100, S&P500)
│   ├── CMakeLists.txt
│   ├── config.cfg
│   ├── main.cpp
│   └── genera_boxplots.py
└── tests/
    └── test.cpp             # Tests sin GoogleTest (autocontenidos)
```

---

## 2) Qué resuelve el software

Se modela un problema de inversión con restricciones de pesos:
- cada activo debe cumplir límites `lo` y `hi` (si está activo),
- la suma total de pesos debe ser 1,
- se optimiza una función tipo Markowitz (beneficio penalizado por riesgo).

El ejecutable:
1. entrena con datos **2015-2024**,
2. evalúa la misma cartera en **2025**,
3. compara algoritmos y exporta resultados a CSV.

### Algoritmos implementados
- `GreedySearch` (determinista)
- `RandomSearch`
- `LocalSearch` (BL)
- `LocalSearchBest` (BL Best)
- `LocalSearchMultiStart` (BL Multi)

---

## 3) Requisitos

### Compilación y ejecución C++
- `g++` con soporte C++17 o superior
- `cmake` (recomendado 3.28+, aunque suele compilar con versiones cercanas)
- `make`

### Gráficas (opcional)
- Python 3
- paquetes: `pandas`, `seaborn`, `matplotlib`

### Memoria (opcional)
- `pdflatex`
- `bibtex`

---

## 4) Compilar y ejecutar (software)

Desde `software/`:

```bash
cd software
cmake -S . -B build
cmake --build build -j
```

Esto genera el ejecutable `build/main`.

### Ejecución

```bash
./build/main
```

También puedes sobreescribir la semilla por línea de comandos:

```bash
./build/main 12345
```

> Nota: en `main.cpp` los límites de evaluación y número de ejecuciones están fijados actualmente a `10000` y `50` respectivamente (`MAX_EVALUACIONES` y `NUM_EJECUCIONES` en código), aunque `config.cfg` también contiene esos campos.

---

## 5) Configuración (`software/config.cfg`)

Parámetros principales:
- `SEED`: semilla base.
- `LAMBDA`: peso de la penalización por riesgo.
- `LS_RATIO`: intensidad del movimiento en búsqueda local.
- `USE_CUSTOM_MARKET`: `0` usa mercados por defecto, `1` usa mercado personalizado.
- `CUSTOM_NAME`, `CUSTOM_PATH`, `CUSTOM_LO`, `CUSTOM_HI`: configuración del mercado personalizado.

Con `USE_CUSTOM_MARKET=0`, se ejecutan por defecto:
- IBEX 35 (`lo=0.005`, `hi=0.08`)
- S&P 100 (`lo=0.005`, `hi=0.05`)
- S&P 500 (`lo=0.005`, `hi=0.02`)

---

## 6) Resultados y gráficas

Tras ejecutar el binario en `software/`, se generan CSV por mercado (p. ej. `IBEX_35_resultados.csv`) con columnas:
- `alg`
- `fitness`

Para generar boxplots a partir de los CSV:

```bash
cd software
python3 genera_boxplots.py
```

Se crean PNG con el mismo nombre base que cada CSV.

---

## 7) Tests

Los tests están en `tests/test.cpp` y son autocontenidos (sin framework externo).

Compilación de ejemplo (desde la raíz del proyecto):

```bash
g++ -std=c++17 -O2 -Wall \
  -I software/common -I software/inc \
  software/src/csv_reader.cpp software/src/portfolio_problem.cpp \
  tests/test.cpp \
  -o tests/test_runner
```

Ejecución:

```bash
./tests/test_runner
```

La batería cubre, entre otros, estos bloques:
- constructor/carga de datos,
- validación de soluciones,
- reparación (`fix`),
- fitness y robustez numérica,
- integración end-to-end.

---

## 8) Memoria (`informe/`)

Para compilar la memoria PDF:

```bash
cd informe
make informe
```

Para limpiar auxiliares:

```bash
make clean
```

El documento principal es `informe/main.tex`, que incluye las secciones en `informe/secciones/`.

---

## 9) Autoría

Proyecto desarrollado por **Ismael Sallami Moreno** para la práctica de Metaheurística (curso 2025-26).