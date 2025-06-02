#!/usr/bin/env python3
import subprocess
import os
import csv
import pandas as pd
import seaborn as sns
import matplotlib.pyplot as plt
from time import sleep
import re

BINARIES = [("./seg_original", "Original"), ("./seg_simd", "SIMD")]
BIN_COUNTS = [2, 4, 8, 16, 32, 64]
IN_IMAGE = "image.png"
OUT_IMAGE = "out.png"
TRIALS = 5
OUTPUT_CSV = "power_measurements.csv"
PERF_EVENT = "power/energy-pkg/"


def run_global_perf(executable, bin_count):
    cmd = ["sudo", "perf", "stat", "-e", PERF_EVENT, executable, IN_IMAGE, str(bin_count), OUT_IMAGE]
    result = subprocess.run(cmd, stderr=subprocess.PIPE, text=True)
    energy_lines = [l for l in result.stderr.split("\n") if "Joules" in l and PERF_EVENT in l]

    if not energy_lines:
        return float("nan")

    match = re.search(r"(\d+\.\d+)\s+Joules", energy_lines[0])
    return float(match.group(1)) if match else float("nan")


def run_local_perf(executable, bin_count):
    subprocess.run(["mkfifo", "perf_fifo.ctl"])
    subprocess.run(["mkfifo", "perf_fifo.ack"])

    try:
        with subprocess.Popen(
            ["exec {ctl_fd}<>perf_fifo.ctl && exec {ack_fd}<>perf_fifo.ack"], shell=True, executable="/bin/bash"
        ):
            env = os.environ.copy()
            env["PERF_CTL_FD"] = "3"
            env["PERF_ACK_FD"] = "4"

            cmd = [
                "sudo",
                "perf",
                "stat",
                "-e",
                PERF_EVENT,
                "--control",
                "fd:3,4",
                "--delay=1",
                "--",
                executable,
                IN_IMAGE,
                str(bin_count),
                OUT_IMAGE,
            ]

            result = subprocess.run(cmd, env=env, stderr=subprocess.PIPE, text=True)
            energy_lines = [l for l in result.stderr.split("\n") if "Joules" in l and PERF_EVENT in l]

            if not energy_lines:
                return float("nan")

            match = re.search(r"(\d+\.\d+)\s+Joules", energy_lines[0])
            return float(match.group(1)) if match else float("nan")
    finally:
        subprocess.run(["rm", "perf_fifo.ctl", "perf_fifo.ack"])


def run_powercap(executable, bin_count):
    cmd = ["sudo", executable, IN_IMAGE, str(bin_count), OUT_IMAGE]
    result = subprocess.run(cmd, stdout=subprocess.PIPE, text=True)
    powercap_line = [l for l in result.stdout.split("\n") if "[PowerCap]" in l][0]
    return float(powercap_line.split()[-2])


def run_measurements():
    with open(OUTPUT_CSV, "w", newline="") as csvfile:
        writer = csv.writer(csvfile)
        writer.writerow(["Version", "BinCount", "MeasurementType", "Trial", "EnergyJoules"])

        for exe_path, version in BINARIES:
            for bin_count in BIN_COUNTS:
                print(f"\n=== Testing {version} with {bin_count} bins ===")

                # Global Perf
                for trial in range(1, TRIALS + 1):
                    try:
                        energy = run_global_perf(exe_path, bin_count)
                    except:
                        energy = float("nan")
                    writer.writerow([version, bin_count, "GlobalPerf", trial, energy])
                    sleep(1)

                # Local Perf
                for trial in range(1, TRIALS + 1):
                    energy = run_local_perf(exe_path, bin_count)
                    writer.writerow([version, bin_count, "LocalPerf", trial, energy])
                    print(f"LocalPerf trial {trial}: {energy:.2f} J")
                    sleep(1)


def plot_results():
    df = pd.read_csv(OUTPUT_CSV)

    plt.figure(figsize=(14, 8))
    sns.set_style("whitegrid")

    g = sns.FacetGrid(
        df,
        col="MeasurementType",
        hue="Version",
        col_order=["GlobalPerf", "LocalPerf"],
        height=5,
        aspect=1.2,
    )
    g.map(sns.lineplot, "BinCount", "EnergyJoules", marker="o", ci="sd")

    g.set_axis_labels("Number of Bins", "Energy Consumption (Joules)")
    g.set_titles("{col_name}")
    g.add_legend(title="Implementation")

    plt.suptitle("Energy consumption by bin count and measurement type", y=1.02)
    plt.savefig("energy_comparison.png", bbox_inches="tight")
    plt.close()


if __name__ == "__main__":
    run_measurements()
    plot_results()
