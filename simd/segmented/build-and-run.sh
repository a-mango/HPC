#!/usr/bin/env bash

set -e

if [ "$#" -eq 1 ] && [ "$1" != "Debug" ] && [ "$1" != "Release" ]; then
  echo "Usage: $0 <Release (default)|Debug>"
  exit 1
fi

BUILD_DIR=build
BIN_DIR=build
BIN_NAME=segmentation

PARAM_IMG_IN=./test/img/sample_640.png
PARAM_CLUSTER_CNT=4
PARAM_IMG_OUT=./segmented.png

CMD="$BIN_DIR/$BIN_NAME $PARAM_IMG_IN $PARAM_CLUSTER_CNT $PARAM_IMG_OUT"

BUILD_MODE=${1:-Release}
BUILD_OPTS="-DCMAKE_BUILD_TYPE=$BUILD_MODE"
MAKE_OPTS="-j8"

cmake -S . -B $BUILD_DIR $BUILD_OPTS
cmake --build $BUILD_DIR -- $MAKE_OPTS

echo -e "\n===== PROGRAM ====="
echo -e "$CMD"

echo -e "\n===== RUN ====="
$CMD
