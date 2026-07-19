#!/bin/bash
# Compila el LCA aplicado al problema del Portfolio (Practicas 1-3).
# Reutiliza el framework de la Practica 1 incluido en framework/.
# Uso:  cd src/portfolio && bash build_portfolio.sh && ./testlca_portfolio
set -e
DIR="$(cd "$(dirname "$0")" && pwd)"
g++ -std=c++17 -O2 \
  -I"$DIR/framework/common" -I"$DIR/framework/inc" \
  "$DIR/testlca_portfolio.cpp" \
  "$DIR/framework/src/portfolio_problem.cpp" \
  "$DIR/framework/src/csv_reader.cpp" \
  -o "$DIR/testlca_portfolio"
echo "OK -> ejecutar desde src/portfolio/ para que encuentre datos_portfolio_2526/"
