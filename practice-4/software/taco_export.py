#!/usr/bin/env python3
"""Exporta los resultados a formato Excel de TACO (tacolab.org).

Formato exigido por readdata.py de TACO:
  columnas: alg, milestone, dimension, F1..F30
  cada fila = un run de un algoritmo a un milestone (en una dimension)
  valores no negativos, no cero, no nulos -> los ceros se sustituyen por EPS.

Genera en taco_export/:
  - taco_all.xlsx        (todos los algoritmos apilados, columna alg)
  - taco_<alg>.xlsx      (uno por algoritmo; TACO toma alg del nombre)
"""

from pathlib import Path
import pandas as pd

BASE = Path(__file__).resolve().parent
OUT = BASE / "taco_export"
EPS = 1e-8                      # TACO prohibe ceros/negativos
NFUNCS = 30
DIMS = [10, 30, 50]
ALGS = ["lca", "lcasw", "lcachc", "lcamulti", "de", "pso"]
FCOLS = [f"F{i}" for i in range(1, NFUNCS + 1)]


def build_rows(alg):
    """Devuelve lista de filas dict para un algoritmo (todas las dims)."""
    rows = []
    for dim in DIMS:
        # acc[milestone][run_idx][fid] = error
        acc = {}
        nruns_seen = {}
        for fid in range(1, NFUNCS + 1):
            f = BASE / f"results_{alg}" / f"results_{fid}_{dim}.txt"
            if not f.exists():
                continue
            df = pd.read_csv(f)
            # contador de run por milestone (la k-esima aparicion = run k)
            counter = {}
            for ms, err in zip(df["milestone"], df["error"]):
                ms = int(ms)
                r = counter.get(ms, 0)
                counter[ms] = r + 1
                acc.setdefault(ms, {}).setdefault(r, {})[fid] = float(err)
        # construir filas
        for ms in sorted(acc.keys()):
            for r in sorted(acc[ms].keys()):
                fila = {"alg": alg, "milestone": ms, "dimension": dim}
                ok = True
                for fid in range(1, NFUNCS + 1):
                    v = acc[ms][r].get(fid)
                    if v is None:
                        ok = False
                        break
                    fila[f"F{fid}"] = v if v > EPS else EPS
                if ok:
                    rows.append(fila)
    return rows


def main():
    OUT.mkdir(exist_ok=True)
    cols = ["alg", "milestone", "dimension"] + FCOLS
    all_rows = []
    for alg in ALGS:
        rows = build_rows(alg)
        if not rows:
            print(f"  [aviso] sin datos para {alg}")
            continue
        df = pd.DataFrame(rows, columns=cols)
        df.to_excel(OUT / f"taco_{alg}.xlsx", index=False)
        print(f"  taco_{alg}.xlsx  ({len(df)} filas)")
        all_rows.extend(rows)
    df_all = pd.DataFrame(all_rows, columns=cols)
    df_all.to_excel(OUT / "taco_all.xlsx", index=False)
    print(f"  taco_all.xlsx    ({len(df_all)} filas, {df_all['alg'].nunique()} algos)")


if __name__ == "__main__":
    main()
