#!/usr/bin/env python3
"""
Script para extraer datos de CSV y generar tablas LaTeX comparativas
entre ILS-ES e ILS-ES+CHC.
"""

import pandas as pd
import os
from pathlib import Path

def extract_results_from_csv(csv_path, algorithm_name):
    """
    Lee un CSV y extrae estadísticas para un algoritmo.
    
    Args:
        csv_path: Ruta del archivo CSV
        algorithm_name: Nombre del algoritmo a buscar (ej: 'ILS-ES', 'ILS-ES-CHC')
    
    Returns:
        Dict con estadísticas o None si no encuentra el algoritmo
    """
    try:
        df = pd.read_csv(csv_path)
        
        # Filtrar por algoritmo
        algo_data = df[df['alg'] == algorithm_name]['fitness']
        
        if len(algo_data) == 0:
            return None
        
        return {
            'mean': algo_data.mean(),
            'std': algo_data.std(),
            'min': algo_data.min(),
            'max': algo_data.max(),
            'count': len(algo_data)
        }
    except Exception as e:
        print(f"Error leyendo {csv_path}: {e}")
        return None


def generate_comparison_table(market_name, csv_path):
    """
    Genera una tabla LaTeX comparativa para un mercado.
    
    Args:
        market_name: Nombre del mercado (IBEX_35, SP_100, SP_500)
        csv_path: Ruta del CSV con resultados
    
    Returns:
        String con tabla LaTeX
    """
    
    # Extraer datos
    ils_es_data = extract_results_from_csv(csv_path, 'ILS-ES')
    ils_es_chc_data = extract_results_from_csv(csv_path, 'ILS-ES-CHC')
    
    if not ils_es_data or not ils_es_chc_data:
        return f"% Datos no encontrados para {market_name}\n"
    
    # Calcular mejora
    improvement_mean = ils_es_chc_data['mean'] - ils_es_data['mean']
    improvement_pct = (improvement_mean / ils_es_data['mean'] * 100) if ils_es_data['mean'] != 0 else 0
    
    # Determinar nombre corto del mercado
    market_short = {
        'IBEX_35': 'IBEX 35',
        'SP_100': r'S\&P 100',
        'SP_500': r'S\&P 500'
    }.get(market_name, market_name)
    
    table = f"""\\begin{{table}}[H]
  \\centering
  \\caption{{Comparación ILS-ES vs ILS-ES+CHC en {market_short}}}
  \\label{{tab:results_ilses_chc_{market_name.lower()}}}
  \\begin{{tabular}}{{|l|r|r|r|r|r|}}
    \\hline
    \\textbf{{Algoritmo}} & \\textbf{{Fitness}} & \\textbf{{Std}} & \\textbf{{Min}} & \\textbf{{Max}} & \\textbf{{Runs}} \\\\
    \\hline
    ILS-ES & {ils_es_data['mean']:.6f} & {ils_es_data['std']:.6f} & {ils_es_data['min']:.6f} & {ils_es_data['max']:.6f} & {ils_es_data['count']} \\\\
    \\hline
    ILS-ES+CHC & {ils_es_chc_data['mean']:.6f} & {ils_es_chc_data['std']:.6f} & {ils_es_chc_data['min']:.6f} & {ils_es_chc_data['max']:.6f} & {ils_es_chc_data['count']} \\\\
    \\hline
    \\textbf{{Mejora}} & \\textbf{{{improvement_mean:+.6f}}} & -- & -- & -- & \\textbf{{{improvement_pct:+.2f}\\%}} \\\\
    \\hline
  \\end{{tabular}}
\\end{{table}}

"""
    
    return table


def generate_all_comparison_tables():
    """
    Genera tablas comparativas para todos los mercados.
    """
    code_dir = Path(__file__).parent.parent / 'code'
    
    markets = {
        'IBEX_35': code_dir / 'IBEX_35_results.csv',
        'SP_100': code_dir / 'S&P_100_results.csv',
        'SP_500': code_dir / 'S&P_500_results.csv'
    }
    
    all_tables = ""
    
    for market_name, csv_path in markets.items():
        if csv_path.exists():
            table = generate_comparison_table(market_name, str(csv_path))
            all_tables += table
        else:
            print(f"Advertencia: No se encontró {csv_path}")
    
    return all_tables


def generate_analysis_summary():
    """
    Genera un resumen de análisis comparativo.
    """
    code_dir = Path(__file__).parent.parent / 'code'
    
    summary = "\\subsection{Resumen de Mejoras}\n\n"
    summary += "\\begin{itemize}\n"
    
    markets = {
        'IBEX_35': 'IBEX 35',
        'SP_100': r'S\&P 100',
        'SP_500': r'S\&P 500'
    }
    
    csv_paths = {
        'IBEX_35': code_dir / 'IBEX_35_results.csv',
        'SP_100': code_dir / 'S&P_100_results.csv',
        'SP_500': code_dir / 'S&P_500_results.csv'
    }
    
    for key, display_name in markets.items():
        csv_path = csv_paths[key]
        if csv_path.exists():
            ils_es = extract_results_from_csv(str(csv_path), 'ILS-ES')
            ils_es_chc = extract_results_from_csv(str(csv_path), 'ILS-ES-CHC')
            
            if ils_es and ils_es_chc:
                improvement = ils_es_chc['mean'] - ils_es['mean']
                improvement_pct = (improvement / ils_es['mean'] * 100) if ils_es['mean'] != 0 else 0
                
                summary += f"  \\item \\textbf{{{display_name}}}: "
                if improvement > 0:
                    summary += f"ILS-ES+CHC mejora en {improvement_pct:+.2f}\\%\n"
                else:
                    summary += f"ILS-ES tiene mejor desempeño en {-improvement_pct:.2f}\\%\n"
    
    summary += "\\end{itemize}\n\n"
    
    return summary


if __name__ == '__main__':
    print("Generando tablas comparativas...")
    tables = generate_all_comparison_tables()
    print(tables)
    
    print("\nGenerando resumen...")
    summary = generate_analysis_summary()
    print(summary)
