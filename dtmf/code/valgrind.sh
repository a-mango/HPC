#!/usr/bin/env bash

set -e

SCRIPT_DIR=$(dirname "$(realpath "$0")")
BUILD_DIR="$SCRIPT_DIR/build"
BIN_DIR="$SCRIPT_DIR/bin"
OUT_DIR="$SCRIPT_DIR/reports/perf"
BIN_GOE=dtmf_encdec-goertzel
BIN_FFT=dtmf_encdec-fft
BIN_ENC=dtmf_encdec-enc
DATASET_SIZE=10
BUILD_OPTS="-DCMAKE_BUILD_TYPE=Release -DENABLE_DEBUG_SYMBOLS=ON"
MAKE_OPTS="-j8"
TOOLS=("memcheck" "cachegrind" "callgrind")

mkdir -p "$OUT_DIR"

cmake -S . -B $BUILD_DIR $BUILD_OPTS
cmake --build $BUILD_DIR -- $MAKE_OPTS
cp $BIN_DIR/$BIN_GOE $BIN_DIR/$BIN_ENC

run_valgrind() {
  local tool=$1
  local bin_name=$2
  shift 2
  local log_file="$OUT_DIR/${bin_name}_${tool}.log"
  local out_file="$OUT_DIR/${bin_name}_${tool}.out"

  CMD="$BIN_DIR/$bin_name"

  echo "Running $tool on $bin_name..."

  case $tool in
  memcheck)
    valgrind --tool=$tool \
      --leak-check=full \
      --show-leak-kinds=all \
      --log-file="$log_file" \
      $CMD $@
    ;;
  cachegrind)
    valgrind --tool=$tool \
      --cachegrind-out-file="$out_file" \
      --log-file="$log_file" \
      $CMD $@
    cg_annotate "$out_file" >"${log_file}.annotated"
    ;;
  callgrind)
    valgrind --tool=$tool \
      --callgrind-out-file="$out_file" \
      --log-file="$log_file" \
      $CMD $@
    callgrind_annotate --auto=yes "$out_file" >"${log_file}.annotated"
    ;;
  esac

  echo "$tool analysis completed for $bin_name"
  echo "Log file: $log_file"
}

analyze_binary() {
  local bin_name=$1
  shift
  for tool in "${TOOLS[@]}"; do
    run_valgrind "$tool" "$bin_name" "$@"
  done
}

echo "Starting Valgrind analysis..."

input_file=input.txt
output_file=output.wav

echo "Generating sample data..."
python -c "print('ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789' * $DATASET_SIZE, end='')" >$input_file

analyze_binary $BIN_ENC "encode $input_file $output_file"
analyze_binary $BIN_GOE "decode $output_file"
analyze_binary $BIN_FFT "decode $output_file"

echo "Valgrind data saved to $OUT_DIR"
echo "Display results with:"
echo "1. Memory leaks:        less $OUT_DIR/*_memcheck.log"
echo "2. Cache misses:        cg_annotate $OUT_DIR/*_cachegrind.out"
echo "3. Call graph:          kcachegrind $OUT_DIR/*_callgrind.out"
