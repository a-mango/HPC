#!/usr/bin/env python3

import pandas as pd
import matplotlib.pyplot as plt

# Load the CSV data
data = pd.read_csv("benchmark.csv", header=None, names=["Image", "Méthode", "Temps"])

# Pivot the data for easier plotting
pivot_data = data.pivot(index="Image", columns="Méthode", values="Temps")

# Plot the data
plt.figure(figsize=(8, 6), dpi=100)
for method in pivot_data.columns:
    plt.plot(pivot_data.index, pivot_data[method], marker="o", linestyle="--", label=method)

# Add labels and title in French
plt.xlabel("Image")
plt.ylabel("Temps d'exécution (s)")
plt.title("Temps d'exécution par image par version")

# Add grid and legend
plt.grid(True, linestyle="--")
plt.legend(title="Version")

# Save the plot
plt.savefig("benchmark.png")

# Show the plot
plt.show()
