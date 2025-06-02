#!/usr/bin/env python3

import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.colors as mcolors
import itertools

# Load CSV
df = pd.read_csv("reports/benchmark.csv", names=["version", "file", "kmer", "mean_time"], header=0)

# Filter out failed runs
df = df[df["mean_time"] != -1]

# Convert columns to numeric
df["file"] = pd.to_numeric(df["file"])
df["kmer"] = pd.to_numeric(df["kmer"])
df["mean_time"] = pd.to_numeric(df["mean_time"])

# Assign a base color to each version, and vary lightness for k-mer
versions = df["version"].unique()
base_cmap = plt.get_cmap("tab10")
version_base_colors = {v: base_cmap(i % 10) for i, v in enumerate(versions)}


def adjust_lightness(color, amount=1.0):
    import colorsys

    try:
        c = mcolors.cnames[color]
    except:
        c = color
    r, g, b = mcolors.to_rgb(c)
    h, l, s = colorsys.rgb_to_hls(r, g, b)
    l = max(0, min(1, amount * l))
    r, g, b = colorsys.hls_to_rgb(h, l, s)
    return (r, g, b)


kmer_min = df["kmer"].min()
kmer_max = df["kmer"].max()

plt.figure(figsize=(10, 7))

# Assign a color to each version
for (version, kmer), group in df.groupby(["version", "kmer"]):
    mean_ms = group["mean_time"].mean()
    # Scale lightness: lower kmer = lighter, higher kmer = darker
    lightness = 0.6 + 0.5 * (kmer - kmer_min) / (kmer_max - kmer_min) if kmer_max > kmer_min else 1.0
    color = adjust_lightness(version_base_colors[version], amount=lightness)
    plt.plot(
        group["file"],
        group["mean_time"],
        marker="o",
        label=f"{version}, k-mer={kmer} (mean: {mean_ms:.1f} ms)",
        color=color,
    )

plt.title("k-mer benchmark in runtime by input size")
plt.xlabel("Input size")
plt.ylabel("Mean time (ms)")
plt.legend()
plt.xscale("log")
plt.yscale("log")
plt.grid(True, which="both", ls="--")
plt.tight_layout()
plt.show()
