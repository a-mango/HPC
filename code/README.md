# DTMF Encoder / Decoder

## Build dependencies

The program relies on the following system dependencies:

- A `GCC` version compatible with the `C23` standard
- `cmake`
- `libfftw3-dev`
- `libsndfile`
- Sanitizers (if desired): `libtsan`, `libtsan`, `liblsan`, `liblsan`
- `ffmpeg` and `ffprobe` (for testing)

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

### Sanitizers

Standard sanitizers are enabled when running the tests. To disable them, set
`ENABLE_ASAB` to `off` in the `build-and-test.sh` script.

### CMake options

The following CMake options are available:

- `ENABLE_TESTS`: enable the tests (default: `off`)
- `ENABLE_COV`: enable code coverage data generation (default: `off`)
- `ENABLE_SAN`: enable the sanitizers (default: `off`)

## Building the Project

### Debug Build

To build the project in debug mode, run the following commands:

```sh
mkdir -p build/debug
cd build/debug
cmake -DCMAKE_BUILD_TYPE=Debug ../..
make
```

### Release Build

To build the project in release mode, run the following commands:

```sh
mkdir -p build/release
cd build/release
cmake -DCMAKE_BUILD_TYPE=Release ../..
make
```

## Testing

To run the tests, you can use the helper script that will invoke `Gtest` with
the appropriate input:

```sh
./build-and-test.sh
```

If you're running the tests manually or through CI, don't forget to set the
following environment variables:

```bash
export DTMF_BINARY_PATH=$BIN_DIR/dtmf_encdec
export DTMF_TEST_SAMPLES_DIR=$SCRIPT_DIR/test/samples
export DTMF_TEST_CONFIG=$DTMF_TEST_SAMPLES_DIR/audio_files.tsv
```

The following command was used to generate the hashes used in the encoding
tests:

```bash
echo -n "$VALUE" > input.txt && ./bin/dtmf_encdec-fft encode input.txt output.wav 2>/dev/null && ffprobe output.wav 2>&1 | tail -n2 | md5sum
```

## Running the Program

After building the project, you can run the program using the following command:

```sh
./dtmf_encdec <input_file> <output_file>
```

Replace `<input_file>` with the path to the input file and `<output_file>` with
the path to the output file.

### Example

To encode a message from `input.txt` and save it to `output.wav`, use:

```sh
./dtmf_encdec encode input.txt output.wav
```

To decode a message from `output.wav` and save it to `decoded.txt`, use:

```sh
./dtmf_encdec decode output.wav decoded.txt
```

## Profiling

The program may be profiled using `gcov`. To do so, enable the `ENABLE_COV`
option during build then run `../scripts/profile.sh` to generate HTML coverage reports.

## Development

The `build-and-run.sh` script can be used to build the project and run an
example encoding and decoding operation. The script will take input from the
`bin/input.txt` file if present, otherwise it will generate one. The encoded
output will be saved to `bin/output.wav`, and the decoded output on `stdout`.
