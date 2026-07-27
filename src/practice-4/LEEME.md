# LEEME — Software Práctica 4 MH (Liver Cancer Algorithm)

Implementación del **Liver Cancer Algorithm (LCA, Houssein et al. 2023)** y sus
variantes sobre el benchmark **CEC2017**, junto con DE y PSO de referencia.

## Prueba rápida (script automático)

Para comprobar de un vistazo que todo compila y funciona:

```bash
cd software
bash probar.sh              # rápido (~1 min): compila todo + reproduce el LCA
                            # en el Portfolio + smoke-test de CEC2017
```

Otros modos: `bash probar.sh --build` (solo compilar), `--portfolio` (solo el
Portfolio), `--completo` (barrido entero CEC2017, ~30-40 min), `-h` (ayuda).
Devuelve código de salida 0 si todo pasa. El smoke-test respalda y restaura
`results_lca/`, así que no altera los resultados incluidos.

## Requisitos

- `cmake` (>= 3.0) y un compilador C++ (g++/clang).
- `python3` con `numpy` y `pandas` (para `analysis.py`).
- El directorio `input_data/` **debe** estar presente (datos del benchmark).

## Compilación

```bash
cd software
mkdir -p build && cd build
cmake .. && make
cd ..
```

Esto genera, dentro de `build/`, los ejecutables:
`testlca`, `testde`, `testpso`, `testlcasw`, `testlcachc`, `testlcamulti`
(más los ejemplos `test`, `testrandom`, `testsolis`) y la librería
`libcec17_test_func.so`.

> **Estructura del código:** las fuentes C/C++ (benchmark + algoritmos) están en
> `src/`. El `CMakeLists.txt` (raíz de `software/`) ya apunta a `src/`, así que la
> compilación no cambia: se ejecuta desde `software/`.

## Ejecución

**Importante:** ejecutar **desde `software/`** (no desde `build/`). Cada binario
lee `input_data/` del directorio actual y escribe los resultados en
`results_<alg>/` relativos al directorio actual.

```bash
cd software
export LD_LIBRARY_PATH=build:$LD_LIBRARY_PATH

# Los resultados se añaden (append). Para una corrida limpia, borrar antes:
rm -f results_lca/*.txt results_lcasw/*.txt results_lcachc/*.txt results_lcamulti/*.txt
rm -f results_de/*.txt results_pso/*.txt

# Cada binario barre D={10,30,50} x 31 semillas (1..31):
for a in lca lcasw lcachc lcamulti de pso; do ./build/test$a; done
```

Coste aproximado: un barrido completo (6 algoritmos x 3 dimensiones x 31 semillas)
tarda ~30-40 min con los procesos en paralelo. D=50 es el cuello de botella.

> Los directorios `results_*/` ya incluyen los datos crudos del estudio entregado
> (31 ejecuciones). Solo es necesario re-ejecutar si se quieren regenerar.

## Análisis estadístico

```bash
cd software
python3 analysis.py
```

Calcula el error medio por función y dimensión, el ranking de Friedman, la
evolución del ranking por hito y el test de Wilcoxon (aproximación normal; no
requiere `scipy`). Escribe las tablas LaTeX en `analysis_out/`.

## Gráficas de convergencia

```bash
cd software
python3 convergence.py
```

Genera las curvas de convergencia del error medio (escala logarítmica frente al
porcentaje de presupuesto) en `figuras/`: `conv_ref.png` (DE/LCA/PSO sobre F1,
F13, F21) y `conv_variants.png` (LCA base vs. variantes en F1). Requiere
`matplotlib`.

## Validación en el Portfolio (comparación con Prácticas 1–3)

El LCA aplicado al problema del Portfolio de las prácticas está en
`src/portfolio/`, autocontenido (incluye una copia del framework de las prácticas
en `framework/` y los datasets en `datos_portfolio_2526/`):

```bash
cd src/portfolio
bash build_portfolio.sh
./testlca_portfolio        # los 3 mercados; o `./testlca_portfolio 0|1|2`
```

Reproduce la fila «LCA» del Cuadro de validación del Portfolio (50 ejecuciones,
10000 evals, λ=500, semilla 42). La función de *fitness* usa el riesgo como
desviación típica (`beneficio − λ·σ`), igual que las Prácticas 2 y 3, para que la
escala sea comparable. Las filas de los demás algoritmos (Búsqueda Local, Genético,
Memético, DE, Enfriamiento Simulado, BMB, ILS, ILS-ES) se obtienen ejecutando el
software de las Prácticas 1–3 bajo el mismo protocolo (mismo λ, evals, runs y
semilla); no se incluyen aquí porque pertenecen a las entregas de esas prácticas.

## Comparación con el estado del arte (Tacolab)

`taco_export.py` genera los Excel en formato TACO (`tacolab.org`) a partir de
`results_*/`:

```bash
python3 taco_export.py     # -> taco_export/taco_<alg>.xlsx
```

Las salidas ya subidas a Tacolab (ranking de Friedman, Wilcoxon y medias frente a
AEO/DE/EBOwithCMAR/PSO/SSA/jSO en D=30) están en `tacolab/` junto con los Excel
empleados (`tacolab/excel/`).

## Configuración experimental

| Parámetro | Valor |
|-----------|-------|
| Funciones | 30 (CEC2017, F1–F30) |
| Dimensiones | D ∈ {10, 30, 50} |
| Presupuesto | 10000 × D evaluaciones |
| Rango | [-100, 100] |
| Ejecuciones | 31 (semillas 1..31) |
| Población | N=30 (LCA y variantes), N=50 (DE/PSO) |

## Mapa de ficheros

| Fichero | Parte | Descripción |
|---------|-------|-------------|
| `testlca.cc` | 2 | LCA base |
| `testde.cc` / `testpso.cc` | 2 | Referencias DE/rand/1/bin y PSO |
| `testlcasw.cc` | 3 | LCA memético + Solis-Wets |
| `testlcachc.cc` | 4 | LCA + reinicio CHC (diversidad) |
| `testlcamulti.cc` | 4 | LCA + crowding determinista (multimodal) |
| `cec17*.c/.h`, `cec17_test_func.c` | — | API del benchmark (no modificar) |
| `analysis.py` | — | Estadística y tablas LaTeX |
| `convergence.py` | 2/4 | Gráficas de convergencia (`figuras/`) |
| `src/portfolio/` | 2 | LCA sobre el Portfolio (comparación Prácticas 1–3) |
| `taco_export.py` | 2 | Genera los Excel formato TACO desde `results_*/` |
| `tacolab/` | 2 | Salidas de tacolab.org (ranking/Wilcoxon/medias) + Excel |
