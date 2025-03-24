#!/usr/bin/env bash

set -e

BUILD_DIR=build
BIN_DIR=bin
TEST_NAME=dtmf_encdec_test
CMD="./$BIN_DIR/$TEST_NAME"

export GTEST_COLOR=yes

cmake -S . -B $BUILD_DIR -DCMAKE_BUILD_TYPE=Debug
cmake --build $BUILD_DIR -- -j8

if $CMD; then
  notify-send "Tests Passed" "All tests passed successfully" -r=cmake
else
  notify-send "Tests Failed" "Some tests failed" -u critical -r=cmake
fi
