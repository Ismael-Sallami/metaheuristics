#!/usr/bin/env python3
"""Analisis estadistico de resultados CEC2017 para la Practica 4 (LCA).

Lee los directorios results_<alg>/ generados por los ejecutables, calcula:
  - error medio (sobre las 31 ejecuciones) en el hito 100% por funcion y dim
  - ranking medio de Friedman por dimension
  - evolucion del ranking de Friedman a lo largo de los hitos
  - test de Wilcoxon de rango con signo (aprox. normal) por pares

No usa scipy: el test de Wilcoxon se implementa con la aproximacion normal
con correccion de continuidad (n=30 funciones, suficiente).
"""

import math
from pathlib import Path
import numpy as np
import pandas as pd

DIMS = [10, 30, 50]
FUNCS = list(range(1, 31))
MILESTONES = [1, 2, 3, 5, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100]

# nombre de directorio -> etiqueta legible
ALGS = {
    "de": "DE",
    "lca": "LCA",
    "pso": "PSO",
    "lcasw": "LCA-SW",
    "lcachc": "LCA-CHC",
    "lcamulti": "LCA-Multi",
}

BASE = Path(__file__).resolve().parent


def load_alg(alg):
    """Devuelve dict[(dim, fid, milestone)] = error medio sobre runs."""
    d = {}
    directory = BASE / f"results_{alg}"
    for fid in FUNCS:
        for dim in DIMS:
            f = directory / f"results_{fid}_{dim}.txt"
            if not f.exists():
                continue
            df = pd.read_csv(f)
            g = df.groupby("milestone")["error"].mean()
            for ms, val in g.items():
                d[(dim, fid, int(ms))] = float(val)
    return d


def mean_matrix(data, algs, dim, milestone=100):
    """Matriz funciones x algoritmos del error medio."""
    rows = []
    for fid in FUNCS:
        row = [data[a].get((dim, fid, milestone), np.nan) for a in algs]
        rows.append(row)
    return np.array(rows)  # shape (30, len(algs))


def friedman_ranks(mat):
    """Ranking medio de Friedman (1 = mejor) por columna. Menor error mejor."""
    n, k = mat.shape
    ranks = np.zeros_like(mat, dtype=float)
    for i in range(n):
        order = mat[i].argsort()  # asc: menor error primero
        r = np.empty(k, dtype=float)
        # ranking con empates promediados
        vals = mat[i]
        sorted_idx = np.argsort(vals)
        sv = vals[sorted_idx]
        tmp = np.arange(1, k + 1, dtype=float)
        # promedio en empates
        j = 0
        while j < k:
            jj = j
            while jj + 1 < k and sv[jj + 1] == sv[j]:
                jj += 1
            avg = tmp[j:jj + 1].mean()
            for m in range(j, jj + 1):
                r[sorted_idx[m]] = avg
            j = jj + 1
        ranks[i] = r
    return ranks.mean(axis=0)


def norm_cdf(x):
    return 0.5 * (1.0 + math.erf(x / math.sqrt(2.0)))


def wilcoxon(a, b):
    """Wilcoxon signed-rank, aprox normal con correccion de continuidad.
    a, b: arrays de error medio por funcion (menor = mejor).
    Devuelve (p_value, signo) donde signo='+' si a mejor que b,
    '-' si a peor, '=' si no significativo a 0.05."""
    a = np.asarray(a, float)
    b = np.asarray(b, float)
    d = a - b
    d = d[d != 0]
    n = len(d)
    if n == 0:
        return 1.0, "="
    absd = np.abs(d)
    order = np.argsort(absd)
    sa = absd[order]
    ranks = np.empty(n, float)
    tmp = np.arange(1, n + 1, dtype=float)
    j = 0
    while j < n:
        jj = j
        while jj + 1 < n and sa[jj + 1] == sa[j]:
            jj += 1
        avg = tmp[j:jj + 1].mean()
        ranks[order[j:jj + 1]] = avg
        j = jj + 1
    w_pos = ranks[d > 0].sum()  # a > b  => a peor (mas error)
    w_neg = ranks[d < 0].sum()  # a < b  => a mejor
    W = min(w_pos, w_neg)
    mean = n * (n + 1) / 4.0
    std = math.sqrt(n * (n + 1) * (2 * n + 1) / 24.0)
    if std == 0:
        return 1.0, "="
    z = (W - mean + 0.5) / std
    p = 2.0 * norm_cdf(z)
    p = min(p, 1.0)
    if p >= 0.05:
        sign = "="
    else:
        sign = "+" if w_neg > w_pos else "-"  # a mejor si suma ranks negativos mayor
    return p, sign


def fmt(x):
    return f"{x:.3e}"


def main():
    data = {a: load_alg(a) for a in ALGS}

    # ---------- comprobar que hay datos ----------
    for a in ALGS:
        got = sum(1 for k in data[a] if k[2] == 100)
        print(f"[{ALGS[a]}] celdas hito100: {got} (esperado {len(FUNCS)*len(DIMS)})")
    print()

    ref_algs = ["de", "lca", "pso"]
    var_algs = ["lca", "lcasw", "lcachc", "lcamulti"]

    out = BASE / "analysis_out"
    out.mkdir(exist_ok=True)

    # ============ PARTE 2: DE / LCA / PSO ============
    for dim in DIMS:
        mat = mean_matrix(data, ref_algs, dim, 100)
        ranks = friedman_ranks(mat)
        print(f"=== D={dim} | Friedman (DE,LCA,PSO) ===")
        for a, r in zip(ref_algs, ranks):
            print(f"  {ALGS[a]:10s} rank={r:.3f}")
        # mejor por funcion
        best_counts = {a: 0 for a in ref_algs}
        for i in range(len(FUNCS)):
            bi = np.nanargmin(mat[i])
            best_counts[ref_algs[bi]] += 1
        # Wilcoxon LCA vs DE, LCA vs PSO
        lca_col = mat[:, ref_algs.index("lca")]
        de_col = mat[:, ref_algs.index("de")]
        pso_col = mat[:, ref_algs.index("pso")]
        p_de, s_de = wilcoxon(lca_col, de_col)
        p_pso, s_pso = wilcoxon(lca_col, pso_col)
        print(f"  Wilcoxon LCA vs DE : p={p_de:.4f} ({s_de})")
        print(f"  Wilcoxon LCA vs PSO: p={p_pso:.4f} ({s_pso})")
        print(f"  Best counts: {best_counts}")
        print()

        # tabla LaTeX media (DE,LCA,PSO)
        lines = ["\\begin{tabular}{lrrr}", "\\toprule",
                 " & DE & LCA & PSO \\\\", "\\midrule"]
        for i, fid in enumerate(FUNCS):
            lines.append(f"F{fid:02d} & {fmt(mat[i,0])} & {fmt(mat[i,1])} & {fmt(mat[i,2])} \\\\")
        lines += ["\\midrule",
                  f"Rank & {ranks[0]:.2f} & {ranks[1]:.2f} & {ranks[2]:.2f} \\\\",
                  "\\bottomrule", "\\end{tabular}"]
        (out / f"mean_ref_D{dim}.tex").write_text("\n".join(lines))

    # evolucion ranking por hito (DE,LCA,PSO), todas las dims combinadas y por dim
    for dim in DIMS:
        print(f"=== D={dim} | Evolucion ranking Friedman por hito (DE,LCA,PSO) ===")
        header = "alg  " + " ".join(f"{m:>6}" for m in MILESTONES)
        print(header)
        rank_evol = {a: [] for a in ref_algs}
        for ms in MILESTONES:
            mat = mean_matrix(data, ref_algs, dim, ms)
            ranks = friedman_ranks(mat)
            for a, r in zip(ref_algs, ranks):
                rank_evol[a].append(r)
        for a in ref_algs:
            print(f"{ALGS[a]:4s} " + " ".join(f"{v:6.2f}" for v in rank_evol[a]))
        print()

    # ============ PARTES 3-4: variantes ============
    for dim in DIMS:
        mat = mean_matrix(data, var_algs, dim, 100)
        ranks = friedman_ranks(mat)
        print(f"=== D={dim} | Friedman variantes (LCA, LCA-SW, LCA-CHC, LCA-Multi) ===")
        for a, r in zip(var_algs, ranks):
            print(f"  {ALGS[a]:10s} rank={r:.3f}")
        base_col = mat[:, 0]
        for k, a in enumerate(var_algs[1:], start=1):
            p, s = wilcoxon(mat[:, k], base_col)
            print(f"  Wilcoxon {ALGS[a]} vs LCA: p={p:.4f} ({s})")
        print()

        labels = [ALGS[a] for a in var_algs]
        lines = ["\\begin{tabular}{l" + "r" * len(var_algs) + "}", "\\toprule",
                 " & " + " & ".join(labels) + " \\\\", "\\midrule"]
        for i, fid in enumerate(FUNCS):
            cells = " & ".join(fmt(mat[i, k]) for k in range(len(var_algs)))
            lines.append(f"F{fid:02d} & {cells} \\\\")
        lines += ["\\midrule",
                  "Rank & " + " & ".join(f"{r:.2f}" for r in ranks) + " \\\\",
                  "\\bottomrule", "\\end{tabular}"]
        (out / f"mean_var_D{dim}.tex").write_text("\n".join(lines))

    print(f"Tablas LaTeX escritas en {out}")


if __name__ == "__main__":
    main()
