#!/usr/bin/env python3

import csv
import math
import matplotlib.pyplot as plt

# Read data from benchmark.csv
with open("benchmark.csv", "r") as f:
    reader = csv.reader(f)
    rows = list(reader)

x = list(map(int, rows[0][1:]))

# Generate power-of-2 labels for x-axis
exponents = [int(math.log2(val)) for val in x]
labels = [rf"$2^{{{e}}}$" for e in exponents]

plt.figure(figsize=(800 / 100, 600 / 100), dpi=100)

# Plot each implementation's results
for row in rows[1:]:
    label = row[0]
    y = list(map(float, row[1:]))
    plt.plot(x, y, marker="o", linestyle="--", label=label)

plt.xscale("log", base=2)
plt.xticks(x, labels)
plt.yscale("log")
plt.xlabel("Quantité de bins")
plt.ylabel("Temps moyen (s)")
plt.title("Temps moyen par bin par version")
plt.grid(True, which="both", linestyle="--")
plt.legend()
plt.tight_layout()

# Save PNG file
plt.savefig("benchmark.png")
plt.show()
