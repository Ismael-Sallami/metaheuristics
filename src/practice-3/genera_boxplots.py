import pandas as pd
import seaborn as sns
import os
import matplotlib.pyplot as plt
import re

def main():
    sns.set_theme(style="whitegrid")
    fnames = [f for f in os.listdir(".") if f.endswith(".csv")]
    
    LABEL_MAP = {
        "RANDOMSEARCH":    "Random",
        "GREEDYSEARCH":    "Greedy",
        "LOCALSEARCH":     "BL",
        "LOCALSEARCHBEST": "BL Best",
        "LOCALSEARCHMULTI":"BL Multi",
        "AGG-ARIT":        "AGG-Arit",
        "AGG-BLX":         "AGG-BLX",
        "AGE-ARIT":        "AGE-Arit",
        "AGE-BLX":         "AGE-BLX",
        "AM-ALL":          "AM-All",
        "AM-RAND":         "AM-Rand",
        "AM-BEST":         "AM-Best",
        "AGG-GAUSS":       "AGG-Gauss",
        "AM-LSCH":         "AM-LSCh",
        "DE":              "DE",
        "ES":              "ES",
        "BMB":             "BMB",
        "ILS":             "ILS",
        "ILS-ES":          "ILS-ES",
    }

    CATEGORIES = {
        "pr1_obligatorios": ["Greedy", "Random", "BL"],
        "pr1_extras": ["BL Best", "BL Multi"],
        "pr2_obligatorios": ["AGG-Arit", "AGG-BLX", "AGE-Arit", "AGE-BLX", "AM-All", "AM-Rand", "AM-Best"],
        "pr2_extras": ["AGG-Gauss", "AM-LSCh", "DE"],
        "pr3_obligatorios": ["ES", "BMB", "ILS", "ILS-ES"]
    }

    os.makedirs("boxplots_generados", exist_ok=True)

    for fname in fnames:
        print(f"Procesando {fname}...")
        df = pd.read_csv(fname)
        df['alg'] = df['alg'].str.upper().map(lambda x: LABEL_MAP.get(x, x))

        # Deduce market slug
        base = os.path.splitext(fname)[0]
        market_slug = re.sub(r"[^A-Z0-9&]+", "_", base.upper()).strip("_")

        for cat_name, alg_list in CATEGORIES.items():
            df_cat = df[df['alg'].isin(alg_list)]
            if not df_cat.empty:
                n_algs = df_cat['alg'].nunique()
                plot_width = max(8, n_algs * 1.5)
                
                plt.figure(figsize=(plot_width, 6))
                ax = sns.boxplot(data=df_cat, x="alg", y="fitness")
                plt.xticks(rotation=35, ha='right')
                plt.title(f"{market_slug} - {cat_name}")
                plt.tight_layout()
                
                out_path = f"boxplots_generados/{market_slug}_{cat_name}.png"
                plt.savefig(out_path, dpi=300)
                plt.close()
                print(f"  Guardado: {out_path}")

if __name__ == '__main__':
    main()
