#!/usr/bin/env bash

set -e

SCRIPT_DIR=$(dirname "$(realpath $0)")
BIN_DIR=$SCRIPT_DIR/bin
CMD_NAME=likwid-perfctr
CMD_OPS="-C 0 -g FLOPS_DP -m $BIN_DIR/dtmf_encdec_test input.txt output.wav"
CMD="$CMD_NAME $CMD_OPS"

BUILD_MODE=Release
BUILD_OPTS="-DCMAKE_BUILD_TYPE=$BUILD_MODE -DENABLE_PERFMON=ON"
MAKE_OPTS="-j8"

cmake -S . -B $BUILD_DIR $BUILD_OPTS
cmake --build $BUILD_DIR -- $MAKE_OPTS

echo -n "1234567890 ABCDEFGHIJKLMNOPQRSTUVWXYZ .!?,#" >input.txt

$CMD

rm -f input.txt output.wav
