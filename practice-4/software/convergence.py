#!/usr/bin/env python3
"""Genera las graficas de convergencia para la memoria (Parte 2).

Lee los directorios results_<alg>/ (mismo formato que analysis.py) y dibuja la
evolucion del error medio (sobre las 31 ejecuciones) en funcion del porcentaje
de presupuesto consumido (hitos 1,2,3,5,...,100 %), en escala logaritmica.

Produce dos figuras PNG en figuras/:
  - conv_ref.png      : DE / LCA / PSO en F1, F13, F21 (D=30).
  - conv_variants.png : LCA base vs LCA-SW / LCA-CHC / LCA-Multi en F1 (D=30).

No usa scipy; solo numpy, pandas y matplotlib.
"""

from pathlib import Path
import numpy as np
import pandas as pd
import matplotlib
matplotlib.use("Agg")  # backend sin pantalla
import matplotlib.pyplot as plt

MILESTONES = [1, 2, 3, 5, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100]

ALGS = {
    "de": "DE",
    "lca": "LCA",
    "pso": "PSO",
    "lcasw": "LCA-SW",
    "lcachc": "LCA-CHC",
    "lcamulti": "LCA-Multi",
}

BASE = Path(__file__).resolve().parent
OUT = BASE / "figuras"


def curve(alg, fid, dim):
    """Error medio (sobre runs) por hito para (alg, fid, dim)."""
    f = BASE / f"results_{alg}" / f"results_{fid}_{dim}.txt"
    df = pd.read_csv(f)
    g = df.groupby("milestone")["error"].mean()
    return [float(g.get(ms, np.nan)) for ms in MILESTONES]


def plot_group(ax, algs, fid, dim, styles):
    for alg in algs:
        y = curve(alg, fid, dim)
        ax.plot(MILESTONES, y, marker="o", markersize=3, linewidth=1.4,
                label=ALGS[alg], **styles.get(alg, {}))
    ax.set_yscale("log")
    ax.set_xlabel(r"\% del presupuesto" if False else "% del presupuesto")
    ax.set_ylabel("Error medio (log)")
    ax.set_title(f"F{fid:02d}  (D={dim})")
    ax.grid(True, which="both", linestyle=":", linewidth=0.5, alpha=0.6)
    ax.legend(fontsize=8)


def fig_ref():
    """DE/LCA/PSO en una funcion representativa de cada tipo (D=30)."""
    dim = 30
    funcs = [(1, "unimodal"), (13, "hibrida"), (21, "compuesta")]
    styles = {
        "de":  {"color": "#1f77b4"},
        "lca": {"color": "#d62728"},
        "pso": {"color": "#2ca02c"},
    }
    fig, axes = plt.subplots(1, 3, figsize=(13, 4))
    for ax, (fid, _) in zip(axes, funcs):
        plot_group(ax, ["de", "lca", "pso"], fid, dim, styles)
    fig.tight_layout()
    fig.savefig(OUT / "conv_ref.png", dpi=160)
    plt.close(fig)
    print("escrito", OUT / "conv_ref.png")


def fig_variants():
    """LCA base vs variantes (LCA-SW/CHC/Multi) en F1 (D=30)."""
    dim = 30
    fid = 1
    styles = {
        "lca":      {"color": "#d62728"},
        "lcasw":    {"color": "#1f77b4"},
        "lcachc":   {"color": "#ff7f0e", "linestyle": "--"},
        "lcamulti": {"color": "#9467bd"},
    }
    fig, ax = plt.subplots(figsize=(7, 4.5))
    plot_group(ax, ["lca", "lcasw", "lcachc", "lcamulti"], fid, dim, styles)
    ax.set_title(f"F{fid:02d} unimodal (D={dim}) — LCA base vs. variantes")
    fig.tight_layout()
    fig.savefig(OUT / "conv_variants.png", dpi=160)
    plt.close(fig)
    print("escrito", OUT / "conv_variants.png")


def main():
    OUT.mkdir(exist_ok=True)
    fig_ref()
    fig_variants()


if __name__ == "__main__":
    main()
