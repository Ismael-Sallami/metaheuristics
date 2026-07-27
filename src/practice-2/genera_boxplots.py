import pandas as pd
import seaborn as sns
import os
import matplotlib.pyplot as plt
import re

def main():
    sns.set_theme(style="whitegrid")
    fnames:list[str] = [f for f in os.listdir(".") if ".csv" in f]
    for fname in fnames:
        df = pd.read_csv(fname)
        df['alg'] = df['alg'].str.upper()

        n_algs = df['alg'].nunique()
        plot_width = max(8, n_algs * 1.2)
        plot_height = 6

        p = sns.catplot(
            data=df,
            x="alg",
            y="fitness",
            kind="box",
            height=plot_height,
            aspect=plot_width / plot_height,
        )
        p.set(xlabel="Algoritmo", ylabel="Fitness")

        for ax in p.axes.flat:
            ax.tick_params(axis='x', labelrotation=35)
            for label in ax.get_xticklabels():
                label.set_horizontalalignment('right')

        p.fig.tight_layout()
        foutput = fname.replace(".csv", ".png")
        p.savefig(foutput, dpi=300, bbox_inches="tight")
        plt.close(p.fig)

        # --- salida organizada como en genera_boxplots.py ---
        # Mapear nombres a etiquetas legibles (mantener consistencia)
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
        }

        PR1_OBLIGATORIOS = ["Greedy", "Random", "BL"]
        PR1_EXTRAS = ["BL Best", "BL Multi"]
        # PR2 categorías (obligatorios y extras)
        PR2_OBLIGATORIOS = [
            "AGG-Arit", "AGG-BLX", "AGE-Arit", "AGE-BLX",
            "AM-All", "AM-Rand", "AM-Best"
        ]
        PR2_EXTRAS = ["AGG-Gauss", "AM-LSCh", "DE"]

        # Preparar df con etiquetas legibles
        df2 = df.copy()
        df2['alg'] = df2['alg'].map(lambda x: LABEL_MAP.get(x, x))

        # Asegurar carpeta de salida
        os.makedirs("boxplots_generados", exist_ok=True)

        # Deduce market slug a partir del nombre de fichero (sin extensión)
        base = os.path.splitext(fname)[0]
        market_slug = re.sub(r"[^A-Z0-9&]+", "_", base.upper()).strip("_")

        # Generar PR1 obligatorios
        df_pr1_obs = df2[df2['alg'].isin(PR1_OBLIGATORIOS)]
        if not df_pr1_obs.empty:
            plot_width = max(8, df_pr1_obs['alg'].nunique() * 1.2)
            p2 = sns.catplot(
                data=df_pr1_obs,
                x="alg",
                y="fitness",
                kind="box",
                height=6,
                aspect=plot_width / 6,
            )
            for ax in p2.axes.flat:
                ax.tick_params(axis='x', labelrotation=35)
                for label in ax.get_xticklabels():
                    label.set_horizontalalignment('right')
            p2.fig.tight_layout()
            out1 = f"boxplots_generados/{market_slug}_pr1_obligatorios.png"
            p2.savefig(out1, dpi=300, bbox_inches="tight")
            plt.close(p2.fig)

        # Generar PR1 extras
        df_pr1_ex = df2[df2['alg'].isin(PR1_EXTRAS)]
        if not df_pr1_ex.empty:
            plot_width = max(8, df_pr1_ex['alg'].nunique() * 1.2)
            p3 = sns.catplot(
                data=df_pr1_ex,
                x="alg",
                y="fitness",
                kind="box",
                height=6,
                aspect=plot_width / 6,
            )
            for ax in p3.axes.flat:
                ax.tick_params(axis='x', labelrotation=35)
                for label in ax.get_xticklabels():
                    label.set_horizontalalignment('right')
            p3.fig.tight_layout()
            out2 = f"boxplots_generados/{market_slug}_pr1_extras.png"
            p3.savefig(out2, dpi=300, bbox_inches="tight")
            plt.close(p3.fig)

        # Generar PR2 obligatorios
        df_pr2_obs = df2[df2['alg'].isin(PR2_OBLIGATORIOS)]
        if not df_pr2_obs.empty:
            plot_width = max(8, df_pr2_obs['alg'].nunique() * 1.2)
            p4 = sns.catplot(
                data=df_pr2_obs,
                x="alg",
                y="fitness",
                kind="box",
                height=6,
                aspect=plot_width / 6,
            )
            for ax in p4.axes.flat:
                ax.tick_params(axis='x', labelrotation=35)
                for label in ax.get_xticklabels():
                    label.set_horizontalalignment('right')
            p4.fig.tight_layout()
            out3 = f"boxplots_generados/{market_slug}_pr2_obligatorios.png"
            p4.savefig(out3, dpi=300, bbox_inches="tight")
            plt.close(p4.fig)

        # Generar PR2 extras
        df_pr2_ex = df2[df2['alg'].isin(PR2_EXTRAS)]
        if not df_pr2_ex.empty:
            plot_width = max(8, df_pr2_ex['alg'].nunique() * 1.2)
            p5 = sns.catplot(
                data=df_pr2_ex,
                x="alg",
                y="fitness",
                kind="box",
                height=6,
                aspect=plot_width / 6,
            )
            for ax in p5.axes.flat:
                ax.tick_params(axis='x', labelrotation=35)
                for label in ax.get_xticklabels():
                    label.set_horizontalalignment('right')
            p5.fig.tight_layout()
            out4 = f"boxplots_generados/{market_slug}_pr2_extras.png"
            p5.savefig(out4, dpi=300, bbox_inches="tight")
            plt.close(p5.fig)
        
if __name__ == '__main__':
    main()
