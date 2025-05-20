#!/usr/bin/env bash

set -e

SCRIPT_DIR=$(dirname "$(realpath "$0")")
BUILD_DIR="$SCRIPT_DIR/build"
BIN_DIR="$SCRIPT_DIR/bin"
OUT_DIR="$SCRIPT_DIR/reports/perf"
BIN_GOE=dtmf_encdec-goertzel
BIN_FFT=dtmf_encdec-fft
BIN_ENC=dtmf_encdec-enc
BUILD_OPTS="-DCMAKE_BUILD_TYPE=Release -DENABLE_DEBUG_SYMBOLS=ON"
MAKE_OPTS="-j8"

mkdir -p "$OUT_DIR"

cmake -S . -B $BUILD_DIR $BUILD_OPTS
cmake --build $BUILD_DIR -- $MAKE_OPTS
cp $BIN_DIR/$BIN_GOE $BIN_DIR/$BIN_ENC

run_perf() {
  local bin_name=$1
  shift # Get remaining arguments as params
  local perf_data="$OUT_DIR/perf_${bin_name}.data"

  echo "Running perf against binary $bin_name"
  perf record -o "$perf_data" \
    --call-graph dwarf \
    "$BIN_DIR/$bin_name" $@
}

echo "Starting Perf analysis..."

input_file=input.txt
input_file_2=input2.txt
output_file=output.wav

echo "Generating sample data..."
python -c "print('ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789' * 1000, end='')" >$input_file
python -c "print('ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789' * 100, end='')" >$input_file_2

# Run Perf while generating the audio sample
run_perf $BIN_ENC encode $input_file $output_file
$BIN_DIR/$BIN_ENC encode $input_file_2 $output_file
run_perf $BIN_GOE decode $output_file
run_perf $BIN_FFT decode $output_file

echo "Perf data saved to: $OUT_DIR"
