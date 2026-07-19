import pandas as pd
import seaborn as sns
import os
import matplotlib.pyplot as plt

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
        
if __name__ == '__main__':
    main()
