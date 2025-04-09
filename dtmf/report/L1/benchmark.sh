#!/bin/bash

set -e

WORKING_DIR=benchmark
mkdir -p $WORKING_DIR

pushd $WORKING_DIR || exit

sizes=(10 100 1000 10000)
output_csv="../benchmark.csv"
fft_binary="../../code/bin/dtmf_encdec-fft"
goertzel_binary="../../code/bin/dtmf_encdec-goertzel"

if [ ! -f "$fft_binary" ] || [ ! -f "$goertzel_binary" ]; then
  echo "Binary files not found. Please build the project first."
  exit 1
fi

echo "Size,Goertzel Decoding Time (ms),FFT Decoding Time (ms)" >$output_csv

generate_input_file() {
  local size=$1
  local filename=$2
  local chars="ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789 .!?,"
  local message=""
  for ((i = 0; i < size; i++)); do
    message+="${chars:RANDOM%${#chars}:1}"
  done
  echo -n "$message" >"$filename"
}

# Generate input files and encode them
for size in "${sizes[@]}"; do
  input_file="input_${size}.bin"
  encoded_file="encoded_${size}.bin"

  echo "Generating input file of size $size..."
  generate_input_file $size $input_file

  echo "Encoding input file using FFT..."
  $fft_binary encode $input_file $encoded_file >/dev/null 2>&1
done

# Decode the encoded files
for size in "${sizes[@]}"; do
  encoded_file="encoded_${size}.bin"

  echo "Decoding encoded file using Goertzel..."
  /usr/bin/time -f "%e" -o time_output.txt $goertzel_binary decode $encoded_file decoded_goertzel_${size}.bin >/dev/null 2>&1
  goertzel_decode_time=$(awk '{print $1 * 1000}' time_output.txt)
  echo "Goertzel decoding time: $goertzel_decode_time ms"

  echo "Decoding encoded file using FFT..."
  /usr/bin/time -f "%e" -o time_output.txt $fft_binary decode $encoded_file decoded_fft_${size}.bin >/dev/null 2>&1
  fft_decode_time=$(awk '{print $1 * 1000}' time_output.txt)
  echo "FFT decoding time: $fft_decode_time ms"

  echo "$size,$goertzel_decode_time,$fft_decode_time" >>$output_csv

done

echo "Cleaning up generated files..."
for size in "${sizes[@]}"; do
  encoded_file="encoded_${size}.bin"
  rm -f "input_${size}.bin" "$encoded_file" decoded_goertzel_"${size}".bin decoded_fft_"${size}".bin time_output.txt
done

echo "Execution times recorded in $output_csv"

popd || exit

rmdir $WORKING_DIR

# Invoke the plot script
./plot.py
