#!/usr/bin/env bash

set -e

BUILD_DIR=build
BIN_DIR=bin
TEST_NAME=dtmf_test

export GTEST_COLOR=yes

cmake -S . -B $BUILD_DIR -DCMAKE_BUILD_TYPE=Debug
cmake --build $BUILD_DIR -- -j8

./$BIN_DIR/$TEST_NAME
