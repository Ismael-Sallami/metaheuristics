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
    
    # Mapeo de mercados
    markets = {
        'IBEX_35': ('IBEX 35', code_dir / 'IBEX_35_results.csv'),
        'SP_100': ('S&P 100', code_dir / 'S&P_100_results.csv'),
        'SP_500': ('S&P 500', code_dir / 'S&P_500_results.csv')
    }
    
    # Crear figura con 3 subplots
    fig, axes = plt.subplots(1, 3, figsize=(15, 5), dpi=150)
    
    for idx, (market_key, (market_name, csv_path)) in enumerate(markets.items()):
        ax = axes[idx]
        
        if not csv_path.exists():
            print(f"Advertencia: No encontrado {csv_path}")
            continue
        
        # Leer CSV
        df = pd.read_csv(csv_path)
        
        # Extraer datos para los dos algoritmos
        ils_es_data = df[df['alg'] == 'ILS-ES']['fitness']
        ils_es_chc_data = df[df['alg'] == 'ILS-ES-CHC']['fitness']
        
        if len(ils_es_data) == 0 or len(ils_es_chc_data) == 0:
            print(f"Advertencia: Datos incompletos para {market_name}")
            continue
        
        # Crear boxplot
        data_to_plot = [ils_es_data, ils_es_chc_data]
        bp = ax.boxplot(data_to_plot, labels=['ILS-ES', 'ILS-ES+CHC'],
                       patch_artist=True, showmeans=True)
        
        # Colorear cajas
        colors = ['#FF9999', '#66B2FF']
        for patch, color in zip(bp['boxes'], colors):
            patch.set_facecolor(color)
        
        # Configurar
        ax.set_ylabel('Fitness (log-retorno)')
        ax.set_title(f'{market_name} (n={len(ils_es_data)} runs)')
        ax.grid(True, alpha=0.3, axis='y')
        
        # Añadir valores de media
        mean_ils = ils_es_data.mean()
        mean_chc = ils_es_chc_data.mean()
        ax.text(0.5, 0.95, f'μ={mean_ils:.4f}', 
               transform=ax.transAxes, fontsize=8, 
               verticalalignment='top', horizontalalignment='center')
        ax.text(1.5, 0.95, f'μ={mean_chc:.4f}', 
               transform=ax.transAxes, fontsize=8, 
               verticalalignment='top', horizontalalignment='center')
    
    plt.tight_layout()
    plt.savefig(f'{output_dir}/boxplot_comparativo_ilses_chc.png', dpi=300, bbox_inches='tight')
    print(f"✓ Gráfica guardada: {output_dir}/boxplot_comparativo_ilses_chc.png")
    plt.close()


def generate_statistical_comparison(code_dir):
    """
    Genera un resumen estadístico comparativo.
    """
    
    print("\n" + "="*70)
    print("COMPARACIÓN ESTADÍSTICA: ILS-ES vs ILS-ES+CHC")
    print("="*70)
    
    markets = {
        'IBEX_35': ('IBEX 35', code_dir / 'IBEX_35_results.csv'),
        'SP_100': ('S&P 100', code_dir / 'S&P_100_results.csv'),
        'SP_500': ('S&P 500', code_dir / 'S&P_500_results.csv')
    }
    
    for market_key, (market_name, csv_path) in markets.items():
        if not csv_path.exists():
            continue
        
        df = pd.read_csv(csv_path)
        
        ils_es = df[df['alg'] == 'ILS-ES']['fitness']
        ils_es_chc = df[df['alg'] == 'ILS-ES-CHC']['fitness']
        
        if len(ils_es) == 0 or len(ils_es_chc) == 0:
            continue
        
        print(f"\n{market_name}:")
        print(f"  ILS-ES:     mean={ils_es.mean():.6f}, std={ils_es.std():.6f}, "
              f"min={ils_es.min():.6f}, max={ils_es.max():.6f}")
        print(f"  ILS-ES+CHC: mean={ils_es_chc.mean():.6f}, std={ils_es_chc.std():.6f}, "
              f"min={ils_es_chc.min():.6f}, max={ils_es_chc.max():.6f}")
        
        diff = ils_es_chc.mean() - ils_es.mean()
        pct = (diff / ils_es.mean() * 100) if ils_es.mean() != 0 else 0
        
        print(f"  Mejora: {diff:+.6f} ({pct:+.2f}%)")
        print(f"  Reducción de variabilidad: {ils_es.std() - ils_es_chc.std():+.6f}")


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
