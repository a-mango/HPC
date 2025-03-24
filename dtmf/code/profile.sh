#!/usr/bin/env bash

set -e

SCRIPT_DIR=$(dirname "$(realpath $0)")
BASE_DIR=$SCRIPT_DIR/build/src/lib/CMakeFiles
GOE_DIR=$BASE_DIR/dtmf_encdec_lib-goertzel.dir
FFT_DIR=$BASE_DIR/dtmf_encdec_lib-fft.dir

GCOVR_OPTS="--decisions --calls --html-theme github.dark-blue --html-single-page"

pushd $GOE_DIR >/dev/null
gcovr $GCOVR_OPTS -r $SCRIPT_DIR/src/lib . --html-details $SCRIPT_DIR/test/cov-goertzel.html
popd >/dev/null

pushd $FFT_DIR >/dev/null
gcovr $GCOVR_OPTS -r $SCRIPT_DIR/src/lib . --html-details $SCRIPT_DIR/test/cov-fft.html
popd >/dev/null
