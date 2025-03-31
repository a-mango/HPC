#!/usr/bin/env bash

set -e

DATE_CMD='date +"%Y-%m-%dT%H:%M:%S%z"'

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

# Run encode and both versions of decode
echo -e "\n===== ENCODE MAXPERF ====="
CMD_OPS="-C 1 -g FLOPS_DP -m $BIN_DIR/dtmf_encdec-fft encode input.txt output.wav" >"$(date +'%Y-%m-%dT%H:%M:%S%z')_maxperf_encode.log"
CMD="$CMD_NAME $CMD_OPS"
$CMD

echo -e "\n===== ENCODE MAXBAND ====="
CMD_OPS="-C 1 -g MEM_DP -m $BIN_DIR/dtmf_encdec-fft encode input.txt output.wav" >"$(date +'%Y-%m-%dT%H:%M:%S%z')_maxband_encode.log"
CMD="$CMD_NAME $CMD_OPS"
$CMD

echo -e "\n===== DECODE FFT MAXPERF ====="
CMD_OPS="-C 1 -g FLOPS_DP -m $BIN_DIR/dtmf_encdec-fft decode output.wav" >"$(date +'%Y-%m-%dT%H:%M:%S%z')_maxperf_decode-fft.log"
CMD="$CMD_NAME $CMD_OPS"
$CMD

echo -e "\n===== DECODE FFT MAXBAND ====="
CMD_OPS="-C 1 -g MEM_DP -m $BIN_DIR/dtmf_encdec-fft decode output.wav" >"$(date +'%Y-%m-%dT%H:%M:%S%z')_maxband_decode-fft.log"
CMD="$CMD_NAME $CMD_OPS"
$CMD

echo -e "\n===== DECODE GOERTZEL MAXPERF ====="
CMD_OPS="-C 1 -g FLOPS_DP -m $BIN_DIR/dtmf_encdec-goertzel decode output.wav" >"$(date +'%Y-%m-%dT%H:%M:%S%z')_maxperf_decode-goertzel.log"
CMD="$CMD_NAME $CMD_OPS"
$CMD

echo -e "\n===== DECODE GOERTZEL MAXBAND ====="
CMD_OPS="-C 1 -g MEM_DP -m $BIN_DIR/dtmf_encdec-goertzel decode output.wav" >"$(date +'%Y-%m-%dT%H:%M:%S%z')_maxband_decode-goertzel.log"
CMD="$CMD_NAME $CMD_OPS"
$CMD

rm -f input.txt output.wav
