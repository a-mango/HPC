/**
 * @file test_decode.cpp
 * @brief Test suite for the decode functionality of the DTMF encoder/decoder.
 * @details This test suite uses GTest to test the decode functionality of the program.
 *          Sample audio files are decoded using both the Goertzel and FFT versions.
 *          The sample audio files are stored in the test/samples directory and the
 *          audio_files.tsv file contains the expected output for each file.
 * @author Aubry Mangold <aubry.mangold@heig-vd.ch>
 * @date 2025-03-23
 */

#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <cctype>
#include <cstdio>
#include <format>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

const std::string BINARY_PATH     = "./bin/dtmf_encdec";
const std::string BINARY_GOERTZEL = BINARY_PATH + "-goertzel";
const std::string BINARY_FFT      = BINARY_PATH + "-fft";
const std::string SAMPLES_DIR     = "./test/samples";
const std::string TSV_FILE        = SAMPLES_DIR + "/audio_files.tsv";

/**
 * @brief Execute a shell command and return the output.
 *
 * @param cmd The command to execute.
 * @return The output of the command.
 */
std::string exec(const std::string& cmd) {
    std::array<char, 128>                 buffer;
    std::string                           result;
    std::string                           full_cmd = cmd + " 2>/dev/null";  // Redirect stderr to /dev/null
    std::unique_ptr<FILE, int (*)(FILE*)> pipe(popen(full_cmd.c_str(), "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    result.erase(std::remove(result.begin(), result.end(), '\n'), result.end());

    return result;
}

/**
 * @class AudioTestParam
 * @brief Structure to hold the test parameters for audio files.
 *
 */
struct AudioTestParam {
    std::string file_path;
    std::string expected_output;
};


/**
 * @brief Print the test parameters to the output stream.
 *
 * @param param The test parameters.
 * @param os The output stream.
 */
void PrintTo(const AudioTestParam& param, std::ostream* os) {
    *os << "file_path: " << param.file_path << ", expected_output: " << param.expected_output;
}

/**
 * @class AudioTestFFT
 * @brief Test fixture class for the FFT version of the program.
 *
 */
class AudioTestFFT : public ::testing::TestWithParam<AudioTestParam> {};

/**
 * @brief Test the expected output of the FFT version of the program.
 */
TEST_P(AudioTestFFT, TestExpectedOutput) {
    AudioTestParam param   = GetParam();
    std::string    command = std::format("{} decode {}/{}", BINARY_FFT, SAMPLES_DIR, param.file_path);
    std::string    output;

    try {
        output = exec(command);
    } catch (const std::runtime_error& e) {
        std::cerr << "Error executing command: " << e.what() << std::endl;
    }

    // Compare the actual output with the expected output
    EXPECT_EQ(output, param.expected_output) << param.file_path;
}

/**
 * @class AudioTestGoertzel
 * @brief Test fixture class for the Goertzel version of the program.
 *
 */
class AudioTestGoertzel : public ::testing::TestWithParam<AudioTestParam> {};

/**
 * @brief Test the expected output of the Goertzel version of the program.
 */
TEST_P(AudioTestGoertzel, TestExpectedOutput) {
    AudioTestParam param   = GetParam();
    std::string    command = std::format("{} decode {}/{}", BINARY_GOERTZEL, SAMPLES_DIR, param.file_path);
    std::string    output;

    try {
        output = exec(command);
    } catch (const std::runtime_error& e) {
        std::cerr << "Error executing command: " << e.what() << std::endl;
    }

    // Compare the actual output with the expected output
    EXPECT_EQ(output, param.expected_output) << param.file_path;
}

/**
 * @brief Load the test parameters from the TSV file.
 *
 * @param tsv_file The path to the TSV file.
 * @return A vector of AudioTestParam objects.
 */
std::vector<AudioTestParam> loadTestParams(const std::string& tsv_file) {
    std::ifstream               file(tsv_file);
    std::vector<AudioTestParam> params;
    std::string                 line, file_path, expected_output;

    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << tsv_file << std::endl;
        return params;
    }

    std::getline(file, line);

    while (std::getline(file, line)) {
        std::istringstream ss(line);
        if (std::getline(ss, file_path, '\t') && std::getline(ss, expected_output, '\t')) {
            params.push_back({file_path, expected_output});
        } else {
            std::cerr << "Failed to parse line: " << line << std::endl;
        }
    }
    return params;
}

/**
 * @brief The test parameters for the audio files.
 */
std::vector<AudioTestParam> audio_test_params = loadTestParams(TSV_FILE);

/**
 * @brief Main function to run the tests.
 */
int main(int argc, char** argv) {
    std::cout << "Loaded " << audio_test_params.size() << " test parameters." << std::endl;
    for (const auto& param : audio_test_params) {
        std::cout << "File: " << param.file_path << ", Expected Output: " << param.expected_output << std::endl;
    }

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

// Instantiate the test suites
INSTANTIATE_TEST_SUITE_P(AudioTestsFFT, AudioTestFFT, ::testing::ValuesIn(audio_test_params));
INSTANTIATE_TEST_SUITE_P(AudioTestsGoertzel, AudioTestGoertzel, ::testing::ValuesIn(audio_test_params));

// Allow uninstantiated parameterized tests
GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(AudioTestFFT);
GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(AudioTestGoertzel);
