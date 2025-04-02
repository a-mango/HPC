#!/usr/bin/env bash

set -euo pipefail

pretty_print() {
  grep -e "Region" -e "DP \[MFLOP\/s\]" -e "Memory bandwidth \[MBytes\/s\]" -e "Operational intensity \[FLOP\/Byte\]" $1 | grep -v -e "AVX" -e "Raw" -e "HWThread"
}

SCRIPT_DIR=$(dirname "$(realpath $0)")
BUILD_DIR=$SCRIPT_DIR/build
BIN_DIR=$SCRIPT_DIR/bin
CMD_NAME=likwid-perfctr

BUILD_MODE=Release
BUILD_OPTS="-DCMAKE_BUILD_TYPE=$BUILD_MODE -DENABLE_PERFMON=ON"
MAKE_OPTS="-j8"

cmake -S . -B "$BUILD_DIR" "$BUILD_OPTS"
cmake --build "$BUILD_DIR" -- "$MAKE_OPTS"

python3 -c 'print("ABCDEFGHIJKLMNOPQRSTUVWXYZ 0123456789 *!?,." * 30, end="")' >input.txt

NOW=$(date +'%Y-%m-%dT%H:%M:%S')
mkdir -p "$SCRIPT_DIR/../log/perfmon/$NOW"
pushd "$SCRIPT_DIR/../log/perfmon/$NOW"

echo "Running encode benchmark..."
CMD_OPS="-o encode.csv -C 2 -g MEM_DP -m $BIN_DIR/dtmf_encdec-fft encode $SCRIPT_DIR/input.txt $SCRIPT_DIR/output.wav"
CMD="$CMD_NAME $CMD_OPS"
$CMD
pretty_print encode.csv

echo "Running decode benchmark with FFT..."
CMD_OPS="-o decode-fft.csv -C 2 -g MEM_DP -m $BIN_DIR/dtmf_encdec-fft decode $SCRIPT_DIR/output.wav"
CMD="$CMD_NAME $CMD_OPS"
$CMD
pretty_print decode-fft.csv

echo "Running decode benchmark with Goertzel..."
CMD_OPS="-o decode-goe.csv -C 2 -g MEM_DP -m $BIN_DIR/dtmf_encdec-goertzel decode $SCRIPT_DIR/output.wav"
CMD="$CMD_NAME $CMD_OPS"
$CMD
pretty_print decode-goe.csv

echo "Done! Reports saved in log/perfmon/$NOW"

popd

rm -f input.txt output.wav
