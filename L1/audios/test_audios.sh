#!/bin/bash

BIN_DIR=../code/bin/
BIN_NAME=dtmf_encdec
BIN_GOE=$BIN_NAME-goertzel
BIN_FFT=$BIN_NAME-fft

# Test all audio samples
echo -e "\n===== TESTING AUDIO SAMPLES ====="
for i in $(fd -e wav .); do
  echo "FILE $i"

  echo -en "\tGOE >>>> "
  ./$BIN_DIR/$BIN_GOE decode "$i" |& tail -n 1

  echo -en "\tFFT >>>> "
  ./$BIN_DIR/$BIN_FFT decode "$i" |& tail -n 1
done
