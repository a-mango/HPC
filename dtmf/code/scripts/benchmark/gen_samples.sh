#!/usr/bin/env bash

set -e

SCRIPT_DIR=$(dirname "$(realpath $0)")
BIN_DIR=

# VALUE(python -c "print(''*1000)")
# ~/D/h/H/lab/d/c/t/samples main• ↑26 $ echo -n "$VALUE" > input.txt && ../../bin/dtmf_encdec-fft encode input.txt alphabet_1000.wav

VALUE=(printf 'ABCDEFGHIJKLMNOPQRSTUVWXYZ%.0s' {1..5})
echo $VALUE
