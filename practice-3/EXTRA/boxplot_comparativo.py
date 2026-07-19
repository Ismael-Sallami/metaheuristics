#!/usr/bin/env python3
"""
Script para generar boxplots comparativos entre ILS-ES e ILS-ES+CHC
en los tres mercados (IBEX 35, S&P 100, S&P 500).
"""

import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
from pathlib import Path


def generate_comparative_boxplots(code_dir, output_dir='../informe/figuras/'):
    """
    Genera boxplots comparativos para ILS-ES vs ILS-ES+CHC.

    Args:
        code_dir: Directorio donde están los CSV
        output_dir: Directorio de salida para figuras
    """

    # Crear directorio de salida
    Path(output_dir).mkdir(parents=True, exist_ok=True)

    # Mapeo de mercados: clave → (nombre legible, ruta CSV, n activos)
    markets = {
        'IBEX_35': ('IBEX 35',  code_dir / 'IBEX_35_results.csv',  30),
        'SP_100':  ('S&P 100',  code_dir / 'S&P_100_results.csv',  97),
        'SP_500':  ('S&P 500',  code_dir / 'S&P_500_results.csv',  457),
    }

    fig, axes = plt.subplots(1, 3, figsize=(18, 7), dpi=150)
    fig.suptitle('Comparación ILS-ES vs ILS-ES+CHC — Distribución de Fitness (2015-2024)',
                 fontsize=12, fontweight='bold', y=1.02)

    colors = ['#FF9999', '#66B2FF']
    labels = ['ILS-ES', 'ILS-ES+CHC']

    # Etiquetas tal como aparecen en los CSV
    alg_keys = ['ILS-ES', 'ILS-ES-CHC']

    for idx, (market_key, (market_name, csv_path, n_assets)) in enumerate(markets.items()):
        ax = axes[idx]

        if not csv_path.exists():
            print(f"[WARN] No encontrado: {csv_path}")
            ax.set_title(f'{market_name}\n(datos no disponibles)')
            ax.set_visible(True)
            continue

        df = pd.read_csv(csv_path)

        # Diagnóstico: mostrar etiquetas disponibles
        available = df['alg'].unique().tolist()
        print(f"[INFO] {market_name}: etiquetas en CSV → {available}")

        data_series = []
        ok = True
        for key in alg_keys:
            serie = df[df['alg'] == key]['fitness'].dropna()
            if len(serie) == 0:
                print(f"[ERROR] Sin datos para '{key}' en {market_name}. "
                      f"Etiquetas disponibles: {available}")
                ok = False
                break
            data_series.append(serie)

        if not ok:
            ax.set_title(f'{market_name}\n(datos incompletos)')
            continue

        # ── Boxplot ──────────────────────────────────────────────────────────
        bp = ax.boxplot(data_series, tick_labels=labels,
                        patch_artist=True, showmeans=True,
                        meanprops=dict(marker='D', markeredgecolor='black',
                                       markerfacecolor='gold', markersize=6))

        for patch, color in zip(bp['boxes'], colors):
            patch.set_facecolor(color)
            patch.set_alpha(0.8)

        # ── Anotaciones de media (coordenadas de datos, no de ejes) ──────────
        for i, (serie, label) in enumerate(zip(data_series, labels), start=1):
            mean_val = serie.mean()
            std_val  = serie.std()
            # Posición x=i (1-indexed como matplotlib), y = por encima del bigote superior
            y_pos = ax.get_ylim()[1] if ax.get_ylim()[1] < 0 else serie.max()
            ax.annotate(f'μ={mean_val:.4f}\nσ={std_val:.4f}',
                        xy=(i, mean_val),
                        xytext=(i, serie.quantile(0.75) + 0.5 * serie.std()),
                        fontsize=7, ha='center',
                        bbox=dict(boxstyle='round,pad=0.2', fc='white', alpha=0.7))

        # ── Estética ──────────────────────────────────────────────────────────
        ax.set_ylabel('Fitness (log-retorno)', fontsize=9)
        ax.set_title(f'{market_name} (n={n_assets})', fontsize=10, fontweight='bold')
        ax.grid(True, alpha=0.3, axis='y', linestyle='--')
        ax.tick_params(axis='x', labelsize=9)

        # Línea de referencia: media ILS-ES
        ax.axhline(data_series[0].mean(), color='red', linestyle=':', alpha=0.5,
                   linewidth=1, label=f'Media ILS-ES = {data_series[0].mean():.4f}')
        ax.legend(fontsize=7, loc='lower right')

        print(f"[OK] {market_name}: ILS-ES n={len(data_series[0])}, "
              f"ILS-ES+CHC n={len(data_series[1])}")

    plt.tight_layout()
    output_path = Path(output_dir) / 'boxplot_comparativo_ilses_chc.png'
    plt.savefig(output_path, dpi=300, bbox_inches='tight')
    print(f"\n✓ Gráfica guardada: {output_path}")
    plt.close()


def generate_statistical_comparison(code_dir):
    """
    Genera un resumen estadístico comparativo.
    """

    print("\n" + "=" * 70)
    print("COMPARACIÓN ESTADÍSTICA: ILS-ES vs ILS-ES+CHC")
    print("=" * 70)

    markets = {
        'IBEX_35': ('IBEX 35',  code_dir / 'IBEX_35_results.csv'),
        'SP_100':  ('S&P 100',  code_dir / 'S&P_100_results.csv'),
        'SP_500':  ('S&P 500',  code_dir / 'S&P_500_results.csv'),
    }

    for market_key, (market_name, csv_path) in markets.items():
        if not csv_path.exists():
            continue

        df = pd.read_csv(csv_path)

        ils_es     = df[df['alg'] == 'ILS-ES']['fitness'].dropna()
        ils_es_chc = df[df['alg'] == 'ILS-ES-CHC']['fitness'].dropna()

        if len(ils_es) == 0 or len(ils_es_chc) == 0:
            print(f"\n{market_name}: datos insuficientes")
            continue

        diff = ils_es_chc.mean() - ils_es.mean()
        pct  = (diff / abs(ils_es.mean()) * 100) if ils_es.mean() != 0 else 0

        print(f"\n{market_name}:")
        print(f"  ILS-ES:     mean={ils_es.mean():.6f},     std={ils_es.std():.6f}, "
              f"min={ils_es.min():.6f}, max={ils_es.max():.6f}")
        print(f"  ILS-ES+CHC: mean={ils_es_chc.mean():.6f}, std={ils_es_chc.std():.6f}, "
              f"min={ils_es_chc.min():.6f}, max={ils_es_chc.max():.6f}")
        print(f"  Δ fitness: {diff:+.6f} ({pct:+.2f}%)")
        print(f"  Δ std:     {ils_es.std() - ils_es_chc.std():+.6f}")


if __name__ == '__main__':
    code_dir = Path(__file__).parent.parent / 'code'

    if not code_dir.exists():
        print(f"Error: No se encontró directorio {code_dir}")
        exit(1)

    print("Generando boxplots comparativos...")
    generate_comparative_boxplots(code_dir)

    print("\nGenerando comparación estadística...")
    generate_statistical_comparison(code_dir)

    print("\n✓ Análisis completado")
