import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.patches as mpatches
import numpy as np
import os

# ─────────────────────────────────────────────
# DATOS
# ─────────────────────────────────────────────
ALGORITMOS = ['Greedy', 'Random', 'BL', 'AGG-Arit', 'AGG-BLX', 'AGE-Arit', 'AGE-BLX', 'AM-All', 'AM-Rand', 'AM-Best']

data_ibex = {
    'alg': ALGORITMOS,
    'fitness': [-4.794, -4.911, -4.614, -4.663, -4.496, -4.587, -4.499, -4.591, -4.490, -4.490],
    'tiempo':  [0.000,  0.024,  0.001,  0.009,  0.010,  0.010,  0.011,  0.008,  0.009,  0.009],
}
data_sp100 = {
    'alg': ALGORITMOS,
    'fitness': [-3.305, -3.400, -3.116, -3.120, -2.879, -3.157, -2.937, -3.110, -2.822, -2.814],
    'tiempo':  [0.000,  0.098,  0.033,  0.071,  0.075,  0.071,  0.075,  0.073,  0.074,  0.074],
}
data_sp500 = {
    'alg': ALGORITMOS,
    'fitness': [-3.370, -3.648, -3.428, -2.961, -2.791, -3.176, -3.012, -3.204, -2.738, -2.731],
    'tiempo':  [0.000,  1.851,  1.526,  1.504,  1.490,  1.478,  1.483,  1.573,  1.537,  1.538],
}

# Cada algoritmo: color + marcador únicos
ALG_STYLE = {
    'Greedy':   {'color': '#9E9E9E', 'marker': 'o'},
    'Random':   {'color': '#607D8B', 'marker': 's'},
    'BL':       {'color': '#1565C0', 'marker': 'D'},
    'AGG-Arit': {'color': '#E65100', 'marker': '^'},
    'AGG-BLX':  {'color': '#FF8F00', 'marker': 'v'},
    'AGE-Arit': {'color': '#B71C1C', 'marker': 'P'},
    'AGE-BLX':  {'color': '#F4511E', 'marker': 'X'},
    'AM-All':   {'color': '#2E7D32', 'marker': '*'},
    'AM-Rand':  {'color': '#00897B', 'marker': 'h'},
    'AM-Best':  {'color': '#558B2F', 'marker': 'p'},
}

CATEGORIES = [
    ('Baselines',         ['Greedy', 'Random'],              '#9E9E9E'),
    ('Búsqueda Local',    ['BL'],                            '#1565C0'),
    ('AG Generacional',   ['AGG-Arit', 'AGG-BLX'],          '#E65100'),
    ('AG Estacionario',   ['AGE-Arit', 'AGE-BLX'],          '#B71C1C'),
    ('Memético',          ['AM-All', 'AM-Rand', 'AM-Best'],  '#2E7D32'),
]


def plot_dataset(dataset_name, df, output_path):
    plt.rcParams.update({
        'font.family': 'DejaVu Sans',
        'font.size': 11,
        'axes.spines.top': False,
        'axes.spines.right': False,
    })

    fig, ax = plt.subplots(figsize=(13, 8), dpi=150)
    fig.patch.set_facecolor('#FAFAFA')
    ax.set_facecolor('#F7F7F7')

    ax.grid(True, linestyle='--', linewidth=0.55, alpha=0.4, color='#BBBBBB', zorder=0)
    ax.set_axisbelow(True)

    # ── Dibujar puntos ──────────────────────────────────────
    handles = []
    for _, row in df.iterrows():
        st = ALG_STYLE[row['alg']]
        size = 180 if row['alg'] == 'AM-All' else 160  # * ligeramente mayor para '*'
        if row['alg'] in ('AM-All',):
            size = 260
        sc = ax.scatter(
            row['tiempo'], row['fitness'],
            s=size,
            c=st['color'],
            marker=st['marker'],
            edgecolors='white',
            linewidth=1.3,
            zorder=5,
            label=row['alg'],
        )

    # ── Márgenes ────────────────────────────────────────────
    x_vals = df['tiempo'].values
    y_vals = df['fitness'].values
    xmin, xmax = x_vals.min(), x_vals.max()
    ymin, ymax = y_vals.min(), y_vals.max()
    xpad = max((xmax - xmin) * 0.10, 0.001)
    ypad = (ymax - ymin) * 0.14
    ax.set_xlim(xmin - xpad, xmax + xpad)
    ax.set_ylim(ymin - ypad, ymax + ypad)

    # ── Leyenda agrupada por categoría ─────────────────────
    legend_entries = []
    for cat_name, cat_algs, cat_color in CATEGORIES:
        # Separador de categoría (título en negrita, sin símbolo)
        legend_entries.append(
            mpatches.Patch(color='none', label=f'$\\bf{{{cat_name}}}$')
        )
        for alg in cat_algs:
            st = ALG_STYLE[alg]
            ms = 10 if alg == 'AM-All' else 8
            legend_entries.append(
                plt.Line2D([0], [0],
                    marker=st['marker'],
                    color='none',
                    markerfacecolor=st['color'],
                    markeredgecolor='white',
                    markeredgewidth=0.8,
                    markersize=ms,
                    label=f'  {alg}',
                )
            )

    legend = ax.legend(
        handles=legend_entries,
        loc='upper left',
        bbox_to_anchor=(1.01, 1.0),
        frameon=True,
        framealpha=0.97,
        edgecolor='#CCCCCC',
        fontsize=9.5,
        handlelength=1.0,
        handletextpad=0.6,
        borderpad=0.9,
        labelspacing=0.45,
        title='Algoritmos',
        title_fontsize=10,
    )
    legend.get_frame().set_linewidth(1.0)

    # ── Ejes y título ────────────────────────────────────────
    ax.set_xlabel('Tiempo de ejecución (s)', fontsize=12, fontweight='bold', labelpad=8)
    ax.set_ylabel('Fitness promedio (2015–2024)', fontsize=12, fontweight='bold', labelpad=8)
    ax.set_title(
        f'Eficiencia: Tiempo vs Fitness  ·  {dataset_name}',
        fontsize=14, fontweight='bold', pad=14, color='#1A1A1A',
    )
    ax.tick_params(axis='both', labelsize=10, length=4, width=1)

    fig.text(
        0.99, 0.01,
        'Mejor fitness = más cercano a 0',
        ha='right', va='bottom', fontsize=8, color='#AAAAAA', style='italic',
    )

    os.makedirs(os.path.dirname(os.path.abspath(output_path)), exist_ok=True)
    fig.savefig(output_path, dpi=300, bbox_inches='tight',
                facecolor='#FAFAFA', edgecolor='none')
    print(f'✓ Guardado: {output_path}')
    plt.close(fig)


# ─────────────────────────────────────────────
# EJECUTAR
# ─────────────────────────────────────────────
datasets = [
    ('IBEX 35',  pd.DataFrame(data_ibex)),
    ('S&P 100',  pd.DataFrame(data_sp100)),
    ('S&P 500',  pd.DataFrame(data_sp500)),
]

for name, df in datasets:
    safe = name.replace('&', '').replace(' ', '_')
    plot_dataset(name, df, f'../informe/figuras/eficiencia_{safe}.png')

print('\n✓ Todos los gráficos generados correctamente.')
