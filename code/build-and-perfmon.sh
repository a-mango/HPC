#!/usr/bin/env bash

set -e

SCRIPT_DIR=$(dirname "$(realpath $0)")
BUILD_DIR=$SCRIPT_DIR/build
BIN_DIR=$SCRIPT_DIR/bin
CMD_NAME=likwid-perfctr

BUILD_MODE=Release
BUILD_OPTS="-DCMAKE_BUILD_TYPE=$BUILD_MODE -DENABLE_PERFMON=ON"
MAKE_OPTS="-j8"

cmake -S . -B $BUILD_DIR $BUILD_OPTS
cmake --build $BUILD_DIR -- $MAKE_OPTS

echo -n "1234567890 ABCDEFGHIJKLMNOPQRSTUVWXYZ .!?,#" >input.txt

NOW=$(date +'%Y-%m-%dT%H:%M:%S')
mkdir -p "$SCRIPT_DIR/../log/perfmon/$NOW"
pushd "$SCRIPT_DIR/../log/perfmon/$NOW"

# Run encode and both versions of decode
echo -e "\n===== ENCODE MAXPERF ====="
CMD_OPS="-o maxperf_encode.json -C 2 -g FLOPS_DP -m $BIN_DIR/dtmf_encdec-fft encode $SCRIPT_DIR/input.txt $SCRIPT_DIR/output.wav"
CMD="$CMD_NAME $CMD_OPS"
$CMD

echo -e "\n===== ENCODE MAXBAND ====="
CMD_OPS="-o maxband_encode.json -C 2 -g MEM_DP -m $BIN_DIR/dtmf_encdec-fft encode $SCRIPT_DIR/input.txt $SCRIPT_DIR/output.wav"
CMD="$CMD_NAME $CMD_OPS"
$CMD

echo -e "\n===== DECODE FFT MAXPERF ====="
CMD_OPS="-o maxperf_decode-fft.json -C 2 -g FLOPS_DP -m $BIN_DIR/dtmf_encdec-fft decode $SCRIPT_DIR/output.wav"
CMD="$CMD_NAME $CMD_OPS"
$CMD

echo -e "\n===== DECODE FFT MAXBAND ====="
CMD_OPS="-o maxband_decode-fft.json -C 2 -g MEM_DP -m $BIN_DIR/dtmf_encdec-fft decode $SCRIPT_DIR/output.wav"
CMD="$CMD_NAME $CMD_OPS"
$CMD

echo -e "\n===== DECODE GOERTZEL MAXPERF ====="
CMD_OPS="-o maxperf_decode-goertzel.json -C 2 -g FLOPS_DP -m $BIN_DIR/dtmf_encdec-goertzel decode $SCRIPT_DIR/output.wav"
CMD="$CMD_NAME $CMD_OPS"
$CMD

echo -e "\n===== DECODE GOERTZEL MAXBAND ====="
CMD_OPS="-o maxband_decode-goertzel.json -C 2 -g MEM_DP -m $BIN_DIR/dtmf_encdec-goertzel decode $SCRIPT_DIR/output.wav"
CMD="$CMD_NAME $CMD_OPS"
$CMD

popd

rm -f input.txt output.wav
