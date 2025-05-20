#!/usr/bin/env bash

set -e

SCRIPT_DIR=$(dirname "$(realpath "$0")")
BUILD_DIR="$SCRIPT_DIR/src/build"
OUT_DIR="$SCRIPT_DIR/reports/perf"
BIN_CRE=create-sample
BIN_ANA=analyze
SIZE=10000000

mkdir -p "$OUT_DIR"

run_perf() {
  local bin_name=$1
  shift # Get remaining arguments as params
  local perf_data="$OUT_DIR/perf_${bin_name}.data"

  echo "Running perf against binary $bin_name"
  perf record -o "$perf_data" \
    --call-graph dwarf \
    "$BUILD_DIR/$bin_name" "$@"
}

echo "Starting Perf analysis..."

run_perf "$BIN_CRE" "$SIZE"
run_perf "$BIN_ANA"

echo "Perf data saved to: $OUT_DIR"
