#!/usr/bin/env bash

set -e

if [ "$#" -eq 1 ] && [ "$1" != "Debug" ] && [ "$1" != "Release" ]; then
  echo "Usage: $0 <Release (default)|Debug>"
  exit 1
fi

BUILD_DIR=build
BIN_DIR=bin
BIN_NAME=dtmf_encdec-goertzel

BUILD_MODE=${1:-Release}
BUILD_OPTS="-DCMAKE_BUILD_TYPE=$BUILD_MODE -DENABLE_SAN=ON"
MAKE_OPTS="-j8"

cmake -S . -B $BUILD_DIR $BUILD_OPTS
cmake --build $BUILD_DIR -- $MAKE_OPTS

echo -e "\n===== PROGRAM ====="
echo -e "$BIN_NAME"

# Show the input
echo -e "\n===== INPUT ====="
[ -f $BIN_DIR/input.txt ] || echo -n "1234567890 ABCDEFGHIJKLMNOPQRSTUVWXYZ .!?,#" >$BIN_DIR/input.txt
cat $BIN_DIR/input.txt

# Run the encoder
echo -e "\n===== ENCODE ====="
./$BIN_DIR/$BIN_NAME encode $BIN_DIR/input.txt $BIN_DIR/output.wav

echo -e "\nffprobe output:"
ffprobe -hide_banner $BIN_DIR/output.wav

# Run the decoder
echo -e "\n===== DECODE ====="
./$BIN_DIR/$BIN_NAME decode $BIN_DIR/output.wav
