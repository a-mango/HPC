#!/usr/bin/env bash

# Check if /usr/bin/tuned-adm is available
[ -e /usr/bin/tuned-adm ] && {
  tuned-adm profile throughput-performance
}

BUILD_DIR=build
BIN_NAME="k-mer"
DIR_IN=inputs
DIR_OUT=reports
FILE_OUT="$DIR_OUT/benchmark.csv"

mkdir -p "$DIR_OUT"

# Input files to benchmark
input_sizes=(1024 2048 4096 8192 16384 32768 65536)
kmer_sizes=(2 4 8 16 32 64 128 256 512)

# Prepare CSV header
if ! [ -e "$FILE_OUT" ]; then
  echo "version,file,kmer,mean_time" >"$FILE_OUT"
fi

# Generate input files if they do not exist
for size in "${input_sizes[@]}"; do
  file="$DIR_IN/$size"
  if [ ! -f "$file" ]; then
    python3 -c "import random; print(''.join(random.choices('ACGT', k=$size)))" >"$file"
  fi
done

# Benchmark for each file
for size in "${input_sizes[@]}"; do
  for k in "${kmer_sizes[@]}"; do
    echo "- Benchmarking k-mer program with input_size=$size, k-mer=$k"
    json_file=$(mktemp)
    if hyperfine --shell=none --warmup 2 --runs 3 --export-json "$json_file" --show-output "./$BUILD_DIR/$BIN_NAME $DIR_IN/$size $k"; then
      mean_time=$(jq -r '.results[0].mean' "$json_file")
      rm "$json_file"
      # Append result to CSV
      echo "k-mer,$size,$k,$mean_time" >>"$FILE_OUT"
    else
      echo "Benchmarking failed for input size $size and k-mer $k"
      echo "k-mer,$size,$k,-1" >>"$FILE_OUT"
      rm -f "$json_file"
    fi
  done
done

echo "Results saved to $FILE_OUT"
