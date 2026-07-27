#!/bin/bash
# ============================================================================
#  probar.sh — Script de prueba del software de la Practica 4 (LCA sobre CEC2017)
# ----------------------------------------------------------------------------
#  Comprueba que todo compila y produce resultados. Modos:
#
#    bash probar.sh              # rapido (defecto): build + portfolio + smoke CEC (~1 min)
#    bash probar.sh --rapido     # idem
#    bash probar.sh --build      # solo compila (CEC + portfolio)
#    bash probar.sh --portfolio  # solo compila y ejecuta el LCA en el Portfolio
#    bash probar.sh --completo   # barrido completo CEC (6 algos) + analisis (~30-40 min)
#    bash probar.sh -h|--help    # ayuda
#
#  Codigo de salida: 0 si todos los pasos PASAN, 1 si alguno FALLA.
# ============================================================================
set -u

# --- Colores -----------------------------------------------------------------
if [ -t 1 ]; then
  ROJO=$'\033[31m'; VERDE=$'\033[32m'; AMAR=$'\033[33m'; AZUL=$'\033[34m'; NEG=$'\033[1m'; FIN=$'\033[0m'
else
  ROJO=""; VERDE=""; AMAR=""; AZUL=""; NEG=""; FIN=""
fi

# --- Ejecutar siempre relativo a la ubicacion del script ---------------------
DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$DIR"

FALLOS=0
paso()  { echo; echo "${AZUL}${NEG}== $* ==${FIN}"; }
ok()    { echo "  ${VERDE}[PASA]${FIN} $*"; }
fail()  { echo "  ${ROJO}[FALLA]${FIN} $*"; FALLOS=$((FALLOS+1)); }
info()  { echo "  ${AMAR}[info]${FIN} $*"; }

ayuda() {
  cat <<'EOF'
probar.sh — Script de prueba del software de la Practica 4 (LCA sobre CEC2017)

Comprueba que todo compila y produce resultados. Modos:

  bash probar.sh              rapido (defecto): build + portfolio + smoke CEC (~1 min)
  bash probar.sh --rapido     idem
  bash probar.sh --build      solo compila (CEC + portfolio)
  bash probar.sh --portfolio  solo compila y ejecuta el LCA en el Portfolio
  bash probar.sh --completo   barrido completo CEC (6 algos) + analisis (~30-40 min)
  bash probar.sh -h|--help    esta ayuda

Codigo de salida: 0 si todos los pasos PASAN, 1 si alguno FALLA.
EOF
  exit 0
}

# ----------------------------------------------------------------------------
#  Pasos
# ----------------------------------------------------------------------------
build_cec() {
  paso "Compilacion CEC2017 (cmake + make)"
  if ! command -v cmake >/dev/null 2>&1; then fail "cmake no instalado"; return; fi
  mkdir -p build
  if (cd build && cmake .. >/tmp/probar_cmake.log 2>&1 && make -j"$(nproc)" >/tmp/probar_make.log 2>&1); then
    local n; n=$(ls build/test* 2>/dev/null | grep -v '\.' | wc -l)
    ok "compilados $n ejecutables + libcec17"
  else
    fail "fallo de compilacion (ver /tmp/probar_make.log)"; tail -15 /tmp/probar_make.log
  fi
}

build_portfolio() {
  paso "Compilacion Portfolio (LCA sobre Practicas 1-3)"
  if bash src/portfolio/build_portfolio.sh >/tmp/probar_portfolio_build.log 2>&1; then
    ok "compilado src/portfolio/testlca_portfolio"
  else
    fail "fallo al compilar el portfolio (ver /tmp/probar_portfolio_build.log)"
    tail -15 /tmp/probar_portfolio_build.log
  fi
}

run_portfolio() {
  paso "Ejecucion Portfolio (reproducibilidad, seed 42)"
  if [ ! -x src/portfolio/testlca_portfolio ]; then fail "binario no existe; ejecuta antes --build"; return; fi
  local out
  out=$(cd src/portfolio && ./testlca_portfolio 2>/dev/null)
  echo "$out" | grep -E "LCA \(" | sed 's/^/    /'
  # Valor determinista esperado: IBEX 35 ~ -4.580 (fitness con desviacion tipica, lambda=500)
  if echo "$out" | grep "LCA (IBEX 35)" | grep -q -- "-4.58"; then
    ok "fitness LCA reproducido (IBEX 35 ~ -4.580, coincide con la memoria)"
  else
    fail "el fitness del LCA no coincide con el valor esperado (-4.58 en IBEX 35)"
  fi
}

smoke_cec() {
  paso "Smoke-test CEC2017 (testlca, ~40 s con timeout)"
  if [ ! -x build/testlca ]; then fail "build/testlca no existe; ejecuta antes --build"; return; fi
  # testlca escribe (append) en results_lca/. Respaldamos para no contaminar los
  # resultados curados de la entrega; se restauran al terminar el smoke-test.
  local bak; bak="$(mktemp -d)"
  [ -d results_lca ] && cp -a results_lca/. "$bak/" 2>/dev/null
  local out
  out=$(LD_LIBRARY_PATH="build:${LD_LIBRARY_PATH:-}" timeout 40 ./build/testlca 2>/dev/null)
  # restaurar results_lca al estado previo
  if [ -d results_lca ]; then rm -f results_lca/*.txt 2>/dev/null; cp -a "$bak/." results_lca/ 2>/dev/null; fi
  rm -rf "$bak"
  local n; n=$(echo "$out" | grep -c "media:")
  if [ "$n" -gt 0 ]; then
    echo "$out" | grep "media:" | head -3 | sed 's/^/    /'
    ok "testlca evalua y emite resultados ($n funciones completadas en 40 s; results_lca/ restaurado)"
  else
    fail "testlca no produjo resultados (revisa input_data/ y libcec17)"
  fi
}

barrido_completo() {
  paso "Barrido COMPLETO CEC2017 (6 algoritmos x 3 dims x 31 runs) — LENTO"
  info "esto tarda ~30-40 min; cada algoritmo escribe en results_<alg>/"
  for a in lca lcasw lcachc lcamulti de pso; do
    if [ ! -x "build/test$a" ]; then fail "falta build/test$a"; continue; fi
    info "ejecutando test$a ..."
    if LD_LIBRARY_PATH="build:${LD_LIBRARY_PATH:-}" ./build/"test$a" >/dev/null 2>&1; then
      ok "test$a completado"
    else
      fail "test$a fallo en ejecucion"
    fi
  done
  paso "Analisis estadistico (analysis.py / convergence.py)"
  if python3 -c "import numpy, pandas" >/dev/null 2>&1; then
    python3 analysis.py    >/tmp/probar_analysis.log 2>&1 && ok "analysis.py (tablas en analysis_out/)" || fail "analysis.py fallo"
    if python3 -c "import matplotlib" >/dev/null 2>&1; then
      python3 convergence.py >/tmp/probar_conv.log 2>&1 && ok "convergence.py (figuras en figuras/)" || fail "convergence.py fallo"
    else
      info "matplotlib no instalado: se omiten las figuras de convergencia"
    fi
  else
    info "numpy/pandas no instalados: se omite el analisis estadistico"
  fi
}

# ----------------------------------------------------------------------------
#  Dispatcher
# ----------------------------------------------------------------------------
MODO="${1:---rapido}"
echo "${NEG}Practica 4 MH — Liver Cancer Algorithm : script de prueba${FIN}"
echo "modo: $MODO    (carpeta: $DIR)"

case "$MODO" in
  -h|--help)   ayuda ;;
  --build)     build_cec; build_portfolio ;;
  --portfolio) build_portfolio; run_portfolio ;;
  --completo)  build_cec; build_portfolio; run_portfolio; barrido_completo ;;
  --rapido|"") build_cec; build_portfolio; run_portfolio; smoke_cec ;;
  *)           echo "${ROJO}Modo desconocido: $MODO${FIN}"; ayuda ;;
esac

# ----------------------------------------------------------------------------
#  Resumen
# ----------------------------------------------------------------------------
echo
if [ "$FALLOS" -eq 0 ]; then
  echo "${VERDE}${NEG}>>> TODO CORRECTO ($MODO): 0 fallos.${FIN}"
  exit 0
else
  echo "${ROJO}${NEG}>>> $FALLOS paso(s) FALLARON ($MODO).${FIN}"
  exit 1
fi
