#!/usr/bin/env python3

import pandas as pd
import matplotlib.pyplot as plt

data = pd.read_csv("benchmark.csv")

pivot_data = data.pivot_table(index="file", columns="version", values="mean_time", aggfunc="mean")

files = pivot_data.index.tolist()
x = range(len(files))

plt.figure(figsize=(8, 6), dpi=100)
for ver in pivot_data.columns:
    y = pivot_data[ver].values
    plt.plot(x, y, marker="o", linestyle="--", label=ver)

plt.xticks(x, files)
plt.xlabel("Fichier")
plt.ylabel("Temps moyen d'exécution (s)")
plt.title("Temps moyen d'exécution par fichier et par version")
grid_style = {"linestyle": "--", "linewidth": 0.5}
plt.grid(True, **grid_style)
plt.legend(title="Version")
plt.tight_layout()
plt.savefig("benchmark.png")

# Affichage du graphique
plt.show()
