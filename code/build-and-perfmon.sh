#!/usr/bin/env bash

set -e

pretty_print() {
  jq -r '"MFLOPS/s:\t \(.MEM_DP.MEM_DP.Metric["DP [MFLOP/s]"].Values[0])\nMBytes/s:\t \(.MEM_DP.MEM_DP.Metric["Memory bandwidth [MBytes/s]"].Values[0]) MB/s\nOI:\t\t \(.MEM_DP.MEM_DP.Metric["Operational intensity [FLOP/Byte]"].Values[0]) FLOP/Byte"' "$1"
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

python3 -c 'print("ABCDEFGHIJKLMNOPQRSTUVWXYZ 0123456789 *!?,." * 50, end="")' >input.txt

NOW=$(date +'%Y-%m-%dT%H:%M:%S')
mkdir -p "$SCRIPT_DIR/../log/perfmon/$NOW"
pushd "$SCRIPT_DIR/../log/perfmon/$NOW"

echo "Running encode benchmark..."
CMD_OPS="-o encode.json -C 2 -g MEM_DP -m $BIN_DIR/dtmf_encdec-fft encode $SCRIPT_DIR/input.txt $SCRIPT_DIR/output.wav 1>/dev/null"
CMD="$CMD_NAME $CMD_OPS"
$CMD
pretty_print encode.json

echo "Running decode benchmark with FFT..."
CMD_OPS="-o decode-fft.json -C 2 -g MEM_DP -m $BIN_DIR/dtmf_encdec-fft decode $SCRIPT_DIR/output.wav 1>/dev/null"
CMD="$CMD_NAME $CMD_OPS"
$CMD
pretty_print decode-fft.json

echo "Running decode benchmark with Goertzel..."
CMD_OPS="-o decode-goe.json -C 2 -g MEM_DP -m $BIN_DIR/dtmf_encdec-goertzel decode $SCRIPT_DIR/output.wav 1>/dev/null"
CMD="$CMD_NAME $CMD_OPS"
$CMD
pretty_print decode-goe.json

echo "Done! Reports saved in log/perfmon/$NOW"

popd

rm -f input.txt output.wav
