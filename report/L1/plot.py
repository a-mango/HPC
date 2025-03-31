#!/usr/bin/env python3

import matplotlib.pyplot as plt
import pandas as pd

# Load the CSV file
df = pd.read_csv("benchmark.csv")

# Plot Goertzel Decoding Time
plt.plot(
    df["Size"], df["Goertzel Decoding Time (ms)"], label="Goertzel Decoding Time (ms)"
)

# Plot FFT Decoding Time
plt.plot(df["Size"], df["FFT Decoding Time (ms)"], label="FFT Decoding Time (ms)")

# Add labels and title
plt.xlabel("Message character count")
plt.ylabel("Time (ms)")
plt.title("Decoding time against message size per algorithm")
plt.legend()

# Save the plot
plt.savefig("benchmark.png")
