#!/usr/bin/env bash

set -e

SCRIPT_DIR=$(dirname "$(realpath $0)")
BUILD_DIR=$SCRIPT_DIR/build
BIN_DIR=$SCRIPT_DIR/bin

TEST_NAME=dtmf_encdec_test
TEST_CMD="$BIN_DIR/$TEST_NAME"

export GTEST_COLOR=yes

export DTMF_BINARY_PATH=$BIN_DIR/dtmf_encdec
export DTMF_TEST_SAMPLES_DIR=$SCRIPT_DIR/test/samples
export DTMF_TEST_CONFIG=$DTMF_TEST_SAMPLES_DIR/audio_files.tsv

cmake -S . -B $BUILD_DIR -DCMAKE_BUILD_TYPE=Debug
cmake --build $BUILD_DIR -- -j8

if $TEST_CMD; then
  notify-send "Tests Passed" "All tests passed successfully" -t 5000
else
  notify-send "Tests Failed" "Some tests failed" -u critical -t 5000
fi
