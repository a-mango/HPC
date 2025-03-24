#!/usr/bin/env bash

set -e

BUILD_DIR=build
BIN_DIR=bin
BIN_NAME=dtmf_encdec-goertzel

cmake -S . -B $BUILD_DIR -DCMAKE_BUILD_TYPE=Debug -DENABLE_ASAN=ON
cmake --build $BUILD_DIR -- -j8

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
