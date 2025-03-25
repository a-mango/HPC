#!/usr/bin/env bash

set -e

SCRIPT_DIR=$(dirname "$(realpath $0)")
BUILD_DIR=$SCRIPT_DIR/build
BIN_DIR=$SCRIPT_DIR/bin
TEST_NAME=dtmf_encdec_test
TEST_CMD="$BIN_DIR/$TEST_NAME"

BUILD_OPTS="-DCMAKE_BUILD_TYPE=Release -DENABLE_COV=ON -DENABLE_TESTS=ON -DENABLE_SAN=ON"
MAKE_OPTS="-j8"

export GTEST_COLOR=yes

export DTMF_TEST_BINARY_PATH=$BIN_DIR/dtmf_encdec
export DTMF_TEST_SAMPLES_DIR=$SCRIPT_DIR/test/samples
export DTMF_TEST_PARAMS_DECODE_TSV=$SCRIPT_DIR/test/params_decode.tsv
export DTMF_TEST_PARAMS_ENCODE_TSV=$SCRIPT_DIR/test/params_encode.tsv

cmake -S . -B $BUILD_DIR $BUILD_OPTS
cmake --build $BUILD_DIR -- $MAKE_OPTS

if $TEST_CMD; then
    notify-send "Tests Passed" "All tests passed successfully" -t 5000
else
    notify-send "Tests Failed" "Some tests failed" -u critical -t 5000
fi
