#!/usr/bin/env bash

set -e

if [ "$#" -eq 1 ] && [ "$1" != "Debug" ] && [ "$1" != "Release" ]; then
  echo "Usage: $0 <Release (default)|Debug>"
  exit 1
fi

SCRIPT_DIR=$(dirname "$(realpath $0)")
BUILD_DIR=$SCRIPT_DIR/build
BIN_DIR=$SCRIPT_DIR/bin
TEST_NAME=dtmf_encdec_test
TEST_CMD="$BIN_DIR/$TEST_NAME $TEST_OPS"

BUILD_MODE=${1:-Release}
BUILD_OPTS="-DCMAKE_BUILD_TYPE=$BUILD_MODE -DENABLE_COV=ON -DENABLE_TESTS=ON -DENABLE_SAN=ON"
MAKE_OPTS="-j8"

export GTEST_COLOR=yes

TEMP_DIR=$(mktemp -d)
unzip -q "$SCRIPT_DIR/test/samples.zip" -d "$TEMP_DIR"

export DTMF_TEST_SAMPLES_DIR=$TEMP_DIR
export DTMF_TEST_BINARY_PATH=$BIN_DIR/dtmf_encdec
export DTMF_TEST_PARAMS_DECODE_TSV=$SCRIPT_DIR/test/params_decode.tsv
export DTMF_TEST_PARAMS_ENCODE_TSV=$SCRIPT_DIR/test/params_encode.tsv

cmake -S . -B $BUILD_DIR $BUILD_OPTS
cmake --build $BUILD_DIR -- $MAKE_OPTS

if $TEST_CMD; then
  notify-send "Tests Passed" "All tests passed successfully" -t 5000
else
  notify-send "Tests Failed" "Some tests failed" -u critical -t 5000
fi

rm -rf "$TEMP_DIR"
