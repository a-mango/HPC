#!/usr/bin/env bash

set -e

# Check if /usr/bin/tuned-adm is available
[ -e /usr/bin/tuned-adm ] && {
  tuned-adm profile throughput-performance
}

BUILD_DIR=build
BIN_DIR=bin
BIN_NAME=dtmf_encdec-fft
DIR_IN=test/samples
DIR_OUT=benchmark
FILE_OUT=benchmark.csv

# Input files to benchmark
files=("alphabet_10.wav" "alphabet_100.wav" "alphabet_1000.wav")

# Prepare CSV header
if ! [ -e "$FILE_OUT" ]; then
  echo "version,file,mean_time" >"$FILE_OUT"
fi

# Benchmark each file for decode operation
for file in "${files[@]}"; do
  echo "- Benchmarking $file with decode operation"
  json_file=$(mktemp)
  hyperfine --shell=none --warmup 1 --runs 2 --export-json "$json_file" \
    "./$BIN_DIR/$BIN_NAME decode $DIR_IN/$file"
  mean_time=$(jq -r '.results[0].mean' "$json_file")
  rm "$json_file"

  # Append result to CSV
  echo "dtmf, $file,$mean_time" >>"$FILE_OUT"
done

echo "Results saved to $FILE_OUT"
