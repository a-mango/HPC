# DTMF Encoder / Decoder

## Build dependencies

The program relies on the following system dependencies:

- A `GCC` version compatible with the `C23` standard (probably `gcc-13` or
  later)
- `cmake` >= `3.22`
- `libsndfile`
- `libfftw3-dev`
- `libtsan`, `libtsan`, `liblsan` and `liblsan` for sanitizing
- `git submodule` for the `googletest` library
- `ffmpeg` and `ffprobe` for testing
- `likwid` for performance profiling

Additionally, you'll have to initialise the Git submodules:

```sh
git submodule update --init
```

### C23 on Ubuntu

If you're running Ubuntu or one of its derivatives, you can install the `gcc-13`
toolchain using the following commands:

```sh
sudo add-apt-repository ppa:ubuntu-toolchain-r/test -y
sudo apt-get update
sudo apt-get install -y gcc-13 g++-13
```

Then set the `CC` and `CXX` environment variables to point to the new compilers
**before** invoking `cmake`:

```sh
export CC=/usr/bin/gcc-13
export CXX=/usr/bin/g++-13
```

**Important**: if you don't set the `CC` and `CCX` variables before invoking
`cmake` and it leads to errors, try deleting the `build` directory

### CMake options

The following CMake options are available:

- `ENABLE_TESTS`: enable the tests (default: `off`)
- `ENABLE_COV`: enable code coverage data generation (default: `off`)
- `ENABLE_SAN`: enable the sanitizers (default: `off`)

## Building the project

### Debug build

To build the project in debug mode, run the following commands:

```sh
mkdir -p build/debug
cd build/debug
cmake -DCMAKE_BUILD_TYPE=Debug ../..
make
```

### Release build

To build the project in release mode, run the following commands:

```sh
mkdir -p build/release
cd build/release
cmake -DCMAKE_BUILD_TYPE=Release ../..
make
```

## Running the program

After building the project, you can run the program using the following command:

```sh
./dtmf_encdec <input_file> <output_file>
```

Replace `<input_file>` with the path to the input file and `<output_file>` with
the path to the output file.

### Example usage

To encode a message from `input.txt` and save it to `output.wav`, use:

```sh
./dtmf_encdec encode input.txt output.wav
```

To decode a message from `output.wav` and save it to `decoded.txt`, use:

```sh
./dtmf_encdec decode output.wav decoded.txt
```

## Testing

### Setup

You might have to manually pull the `googletest` git submodule if `cmake` fails
to do so automatically.Using the following command:

```sh
git submodule update --init --recursive
```

The `samples.zip` archive that contains the test samples is not directly
included with the program. You have to manually copy it to the `test` directory.

### Running the tests

To run the tests, use the [helper script](build-and-test.sh) that will invoke
`Gtest` with the appropriate input:

```sh
./build-and-test.sh
```

If you're running the tests manually or through CI, don't forget to set the
following environment variables:

```bash
export DTMF_TEST_SAMPLES_DIR=$TEMP_DIR
export DTMF_TEST_BINARY_PATH=$BIN_DIR/dtmf_encdec
export DTMF_TEST_PARAMS_DECODE_TSV=$SCRIPT_DIR/test/params_decode.tsv
export DTMF_TEST_PARAMS_ENCODE_TSV=$SCRIPT_DIR/test/params_encode.tsv
```

The following command was used to generate the hashes used in the encoding tests
(it's not very sophisticated but it works):

```bash
echo -n "$VALUE" > input.txt && ./bin/dtmf_encdec-fft encode input.txt output.wav 2>/dev/null && ffprobe output.wav 2>&1 | tail -n2 | md5sum
```

## Profiling

The program may be profiled using `gcov`. To do so, enable the `ENABLE_COV`
option during build then run `../scripts/profile.sh` to generate HTML coverage
reports.

## Sanitizers

Standard sanitizers are enabled when running the tests. To disable them, set
`ENABLE_ASAB` to `off` (you might have to do so in the `build-and-test.sh`
script).

## Performance monitoring

The project uses the `likwid` library for performance monitoring. To use it, you
need to enable the `ENABLE_PERFMON` `cmake` option during the build process, or
use the provided [`build-and-perfmon.sh`](build-and-perfmon.sh) script.

## Development

The [`build-and-run.sh`](build-and-run.sh) script can be used to build the
project and run an example encoding and decoding operation. The script will take
input from the `bin/input.txt` file if present, otherwise it will generate one.
The encoded output will be saved to `bin/output.wav`, and the decoded output on
`stdout`.

## Known issues

- There is a nasty heap-based buffer overflow in the
  [FFT implementation](./src/lib/dtmf_decode_fft.c) of the detection that is
  only picked up by _ASAN_ with the `glitchy` (as of now).
