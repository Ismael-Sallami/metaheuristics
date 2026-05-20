#!/usr/bin/env python3
"""
Script para generar gráficas de convergencia comparativas
entre ILS-ES clásico e ILS-ES con Reinicialización Voraz (CHC/GADEGD).
"""

import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import os

def plot_convergence_comparison(csv_files, output_dir='../informe/figuras/'):
    """
    Genera gráficas de convergencia para comparar ILS vs ILS-CHC.
    
    Args:
        csv_files: dict con claves 'ils_es' y 'ils_es_chc' apuntando a ficheros CSV
        output_dir: directorio donde guardar las figuras
    """
    os.makedirs(output_dir, exist_ok=True)
    
    # Leer datos
    data_ils = pd.read_csv(csv_files['ils_es'])
    data_chc = pd.read_csv(csv_files['ils_es_chc'])
    
    # Preparar datos: agrupar por iteración/evaluación y calcular fitness medio
    data_ils_grouped = data_ils.groupby('evaluacion')['fitness'].agg(['mean', 'std']).reset_index()
    data_chc_grouped = data_chc.groupby('evaluacion')['fitness'].agg(['mean', 'std']).reset_index()
    
    # Crear figura con subplots por mercado
    markets = ['IBEX_35', 'SP_100', 'SP_500']
    fig, axes = plt.subplots(1, 3, figsize=(16, 5), dpi=150)
    
    for idx, market in enumerate(markets):
        ax = axes[idx]
        
        # Filtrar datos por mercado
        ils_market = data_ils_grouped[data_ils_grouped.get('market', 'IBEX_35') == market]
        chc_market = data_chc_grouped[data_chc_grouped.get('market', 'IBEX_35') == market]
        
        # Usar todos los datos si no hay filtro de mercado
        if ils_market.empty:
            ils_market = data_ils_grouped
        if chc_market.empty:
            chc_market = data_chc_grouped
        
        # Plotear
        ax.plot(ils_market['evaluacion'], ils_market['mean'], 
                label='ILS-ES clásico', linewidth=2, marker='o', markersize=4, alpha=0.8)
        ax.fill_between(ils_market['evaluacion'], 
                        ils_market['mean'] - ils_market['std'],
                        ils_market['mean'] + ils_market['std'],
                        alpha=0.2)
        
        ax.plot(chc_market['evaluacion'], chc_market['mean'], 
                label='ILS-ES + Reinicialización (CHC)', linewidth=2, marker='s', markersize=4, alpha=0.8)
        ax.fill_between(chc_market['evaluacion'],
                        chc_market['mean'] - chc_market['std'],
                        chc_market['mean'] + chc_market['std'],
                        alpha=0.2)
        
        ax.set_xlabel('Evaluaciones')
        ax.set_ylabel('Fitness promedio')
        ax.set_title(f'Convergencia -- {market}')
        ax.legend()
        ax.grid(True, alpha=0.3)
    
    plt.tight_layout()
    plt.savefig(f'{output_dir}/convergencia_comparativa.png', dpi=300, bbox_inches='tight')
    print(f"✓ Gráfica guardada: {output_dir}/convergencia_comparativa.png")
    plt.close()


def plot_stagnation_detection(csv_chc, output_dir='../informe/figuras/'):
    """
    Visualiza los puntos de reinicialización (divergencia) en CHC.
    """
    os.makedirs(output_dir, exist_ok=True)
    
    data = pd.read_csv(csv_chc)
    
    # Detectar reinicializaciones (cambios bruscos negativos seguidos de recuperación)
    fig, ax = plt.subplots(figsize=(12, 6), dpi=150)
    
    ax.plot(data['evaluacion'], data['fitness'], linewidth=1.5, alpha=0.7, label='Trayectoria')
    
    # Marcar reinicializaciones (donde 'es_reinicio' == 1, si existe columna)
    if 'es_reinicio' in data.columns:
        reinits = data[data['es_reinicio'] == 1]
        ax.scatter(reinits['evaluacion'], reinits['fitness'], 
                  color='red', s=100, marker='x', linewidth=2, label='Reinicialización CHC')
    
    ax.set_xlabel('Evaluaciones')
    ax.set_ylabel('Fitness')
    ax.set_title('Trayectoria ILS-ES + CHC: Puntos de Reinicialización')
    ax.legend()
    ax.grid(True, alpha=0.3)
    
    plt.tight_layout()
    plt.savefig(f'{output_dir}/reinicializaciones_chc.png', dpi=300, bbox_inches='tight')
    print(f"✓ Gráfica guardada: {output_dir}/reinicializaciones_chc.png")
    plt.close()


def generate_diversity_plot(csv_file, output_dir='../informe/figuras/'):
    """
    Genera gráfica de diversidad genética a lo largo del tiempo.
    """
    os.makedirs(output_dir, exist_ok=True)
    
    data = pd.read_csv(csv_file)
    
    fig, ax = plt.subplots(figsize=(12, 6), dpi=150)
    
    if 'diversidad' in data.columns:
        ax.plot(data['evaluacion'], data['diversidad'], 
                linewidth=2, marker='o', markersize=4, color='green', alpha=0.8)
        ax.set_xlabel('Evaluaciones')
        ax.set_ylabel('Diversidad (entropía)')
        ax.set_title('Evolución de la Diversidad Genética -- ILS-ES + CHC')
        ax.grid(True, alpha=0.3)
        
        plt.tight_layout()
        plt.savefig(f'{output_dir}/diversidad_temporal.png', dpi=300, bbox_inches='tight')
        print(f"✓ Gráfica guardada: {output_dir}/diversidad_temporal.png")
        plt.close()
    else:
        print("⚠ No se encontró columna 'diversidad' en el CSV.")


if __name__ == '__main__':
    # Ejemplo de uso (requiere que existan los ficheros CSV con resultados)
    # csv_files = {
    #     'ils_es': 'EXTRA/resultados_ils_es.csv',
    #     'ils_es_chc': 'EXTRA/resultados_ils_es_chc.csv'
    # }
    # plot_convergence_comparison(csv_files)
    
    print("Script convergencia_comparativa.py listo para usar.")
    print("Uso: plot_convergence_comparison({'ils_es': path1, 'ils_es_chc': path2})")
