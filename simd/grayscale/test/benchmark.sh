#!/usr/bin/env bash

set -e

# Check if /usr/bin/tuned-adm is available
[ -e /usr/bin/tuned-adm ] && {
  tuned-adm profile throughput-performance
}

BINS=(grayscale_c grayscale_simd)
BIN_DIR=../build
DIR_IN=img
DIR_OUT=benchmark
FILE_OUT=benchmark.csv

# Core counts for columns
cores_list=(1 2 4 8 16 32 64 128 256)

echo -e "\n===== BENCHMARK ====="
mkdir -p "$DIR_OUT"

# Prepare CSV header
if ! [ -e "$FILE_OUT" ]; then
  header="image,bin_name"
  for c in "${cores_list[@]}"; do
    header+=",$c"
  done
  echo "$header" >"$FILE_OUT"
fi

# Images to benchmark
images=("forest_2k.png" "forest_4k.png" "forest_8k.png")

# Benchmark each image with both binaries
for image in "${images[@]}"; do
  for bin in "${BINS[@]}"; do
    echo "- Benchmarking $image with $bin"
    times=()
    json_file=$(mktemp)
    hyperfine --shell=none --warmup 5 --runs 5 --export-json "$json_file" \
      "taskset -c 2 $BIN_DIR/$bin $DIR_IN/$image $DIR_OUT/${image}_${bin}.png" >/dev/null
    time=$(jq -r '.results[0].mean' "$json_file")
    rm "$json_file"
    times+=("$time")
    echo "  -> $time s"

    # Append times as row to CSV
    row="$image,$(basename "$bin")"
    for t in "${times[@]}"; do
      row+=",$t"
    done
    echo "$row" >>"$FILE_OUT"
  done
done

echo "Results saved to $FILE_OUT"

