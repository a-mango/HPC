#!/usr/bin/env bash

set -eo pipefail

# Check if the converter converts the images as per the expected images

# get script directory using realpath
SCRIPT_DIR=$(realpath "$(dirname "$0")")
BIN_DIR="$SCRIPT_DIR/../build"
BIN_NAME=segmentation
DIR_OUT=$(mktemp -d)
DIR_IN=img
DIR_EXPECTED=expected

generate_actuals() {
  local img_in=$1
  for cluster_cnt in 1 4 16 64 256; do
    local img_out="${DIR_OUT}/${img_in%.png}.${cluster_cnt}.png"
    # Generate expected image using the original program
    "${BIN_DIR}/${BIN_NAME}" "${DIR_IN}/${img_in}" "${cluster_cnt}" "${img_out}" >/dev/null
  done
}

check_expected() {
  img_in=$1

  for cluster_cnt in 1 4 16 64 256; do
    local img_actual="${DIR_OUT}/${img_in%.png}.${cluster_cnt}.png"
    local img_expected="${DIR_EXPECTED}/${img_in%.png}.${cluster_cnt}.png"
    local img_diff="${DIR_OUT}/${img_in%.png}.${cluster_cnt}.diff.png"
    # check with magick compare if the images are the same, otherwise write false in ok
    if !(magick compare -metric rmse "${img_actual}" "${img_expected}" "${img_diff}" >/dev/null 2>&1); then
      echo -e "\e[31mImage ${img_actual} failed the test\e[0m"
      echo "Diff file: ${img_diff}"
      return 1
    fi
  done

  echo -e "\e[32mImage ${img_in} successfully passed tests\e[0m"
  return 0
}

generate_actuals "sample_256.png"
check_expected "sample_256.png"
[ $? ] || exit 1

generate_actuals "sample_640.png"
check_expected "sample_640.png"
[ $? ] || exit 1

generate_actuals "sample_1024.png"
check_expected "sample_1024.png"
[ $? ] || exit 1
