#!/usr/bin/env bash

set -e

if [ "$#" -eq 1 ] && [ "$1" != "Debug" ] && [ "$1" != "Release" ]; then
  echo "Usage: $0 <Release (default)|Debug>"
  exit 1
fi

BUILD_DIR=build
BUILD_MODE=${1:-Release}
BUILD_OPTS="-DCMAKE_BUILD_TYPE=$BUILD_MODE -DCMAKE_EXPORT_COMPILE_COMMANDS=1"
MAKE_OPTS="-j8"

echo -e "\n===== PROGRAM ====="
cmake -S . -B $BUILD_DIR $BUILD_OPTS
cmake --build $BUILD_DIR -- $MAKE_OPTS
