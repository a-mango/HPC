#!/usr/bin/env bash

set -e

BIN_DIR=../bin
BIN_NAME=segmentation_original
DIR_IN=img
DIR_OUT=expected

generate_expected() {
  local img_in=$1
  # For sizes in 1, 2, 4, ..., 512
  for cluster_cnt in 1 2 4 8 16 32 64 128 256; do
    local img_out="${DIR_OUT}/${img_in%.png}.${cluster_cnt}.png"
    echo "${img_out}"
    # Generate expected image using the original program
    "${BIN_DIR}/${BIN_NAME}" "${DIR_IN}/${img_in}" "${cluster_cnt}" "${img_out}" >/dev/null
  done
}

echo -e "\n===== REFERENCE GENERATOR ====="
mkdir -p $DIR_OUT
# For images in sample_256.png, sample_640.png and sample_1024.png
# Generate reference images using the original program

echo -e "\n===== sample_256.png ====="
generate_expected "sample_256.png"

echo -e "\n===== sample_640.png ====="
generate_expected "sample_640.png"

echo -e "\n===== sample_1024.png ====="
generate_expected "sample_1024.png"
