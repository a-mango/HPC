#!/usr/bin/env bash

set -e

SCRIPT_DIR=$(dirname "$(realpath "$0")")
BUILD_DIR="$SCRIPT_DIR/src/build"
OUT_DIR="$SCRIPT_DIR/valgrind_data"
BIN_CRE=create-sample
BIN_ANA=analyze
SIZE=100000

# Valgrind configuration
TOOLS=("memcheck" "cachegrind" "callgrind")
EXTRA_FLAGS="--track-origins=yes --show-error-list=yes"

mkdir -p "$OUT_DIR"

run_valgrind() {
  local bin_name=$1
  local tool=$2
  local log_file="$OUT_DIR/${bin_name}_${tool}.log"
  local out_file="$OUT_DIR/${bin_name}_${tool}.out"

  if [[ $bin_name == "$BIN_CRE" ]]; then
    CMD="$BUILD_DIR/$bin_name $SIZE"
  else
    CMD="$BUILD_DIR/$bin_name"
  fi

  echo -e "\nRunning $tool on $bin_name..."

  case $tool in
  memcheck)
    valgrind --tool=$tool \
      --leak-check=full \
      --show-leak-kinds=all \
      --log-file="$log_file" \
      $CMD
    ;;
  cachegrind)
    valgrind --tool=$tool \
      --cachegrind-out-file="$out_file" \
      --log-file="$log_file" \
      $CMD
    cg_annotate "$out_file" >"${log_file}.annotated"
    ;;
  callgrind)
    valgrind --tool=$tool \
      --callgrind-out-file="$out_file" \
      --log-file="$log_file" \
      $CMD
    callgrind_annotate --auto=yes "$out_file" >"${log_file}.annotated"
    ;;
  esac

  echo "$tool analysis completed for $bin_name"
  echo "Log file: $log_file"
}

analyze_binary() {
  local bin_name=$1
  for tool in "${TOOLS[@]}"; do
    run_valgrind "$bin_name" "$tool"
  done
}

echo "Starting Valgrind analysis..."

analyze_binary "$BIN_CRE"
analyze_binary "$BIN_ANA"

echo "Valgrind data saved to $OUT_DIR"
