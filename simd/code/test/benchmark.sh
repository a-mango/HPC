#!/usr/bin/env bash

set -e

# Check if /usr/bin/tuned-adm is available
[ -e /usr/bin/tuned-adm ] && {
  tuned-adm profile throughput-performance
}

# Check that $1 contains the name of the binary to benchmark
[[ $1 =~ ^[a-zA-Z0-9_\.\/]+$ ]] || {
  echo "Usage: $0 <binary_path>"
  exit 1
}

BIN=$1
BIN_NAME=$(basename "$BIN")
DIR_IN=img
DIR_OUT=benchmark
FILE_OUT=benchmark.csv

# Core counts for columns
cores_list=(1 2 4 8 16 32 64 128 256)

echo -e "\n===== BENCHMARK ====="
mkdir -p "$DIR_OUT"

# Prepare CSV header
if ! [ -e "$FILE_OUT" ]; then
  header="bin_name"
  for c in "${cores_list[@]}"; do
    header+=",$c"
  done
  echo "$header" >"$FILE_OUT"
fi

# Benchmark sample_640.png
echo -e "\n===== sample_640.png ====="
times=()
for c in "${cores_list[@]}"; do
  json_file=$(mktemp)
  hyperfine --warmup 3 --runs 5 --export-json "$json_file" \
    "taskset -c 2 $BIN $DIR_IN/sample_640.png $c $DIR_OUT/sample_640_$c.png" >/dev/null
  time=$(jq -r '.results[0].mean' "$json_file")
  rm "$json_file"
  times+=("$time")
  echo "cores=$c -> $time s"
done

# Append times as row to CSV
row="$BIN_NAME"
for t in "${times[@]}"; do
  row+=",$t"
done
echo "$row" >>"$FILE_OUT"

echo "Results saved to $FILE_OUT"
