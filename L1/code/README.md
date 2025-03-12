# DTMF Encoder / Decoder

## Build dependencies

The program relies on the following libraries:

- `libfftw3-dev`
- `libsndfile`

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
