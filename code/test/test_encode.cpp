/**
 * @file test_encode.cpp
 * @brief Test suite for the encode functionality of the DTMF encoder/decoder.
 * @details This test suite uses GTest to test the encoding functionality of the program.
 * @author Aubry Mangold <aubry.mangold@heig-vd.ch>
 * @date 2025-03-25
 */

#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

std::string env_encode(const std::string& var, const std::string& default_val) {
    const char* val = std::getenv(var.c_str());
    return val == nullptr ? default_val : std::string(val);
}

const std::string BINARY_PATH     = env_encode("DTMF_TEST_BINARY_PATH", "bin/dtmf_encdec");
const std::string SAMPLES_DIR     = env_encode("DTMF_TEST_SAMPLES_DIR", "test/samples");
const std::string TSV_FILE        = env_encode("DTMF_TEST_PARAMS_ENCODE_TSV", "test/params_encode.tsv");
const std::string BINARY_GOERTZEL = BINARY_PATH + "-goertzel";
const std::string BINARY_FFT      = BINARY_PATH + "-fft";

/**
 * @brief Execute a shell command and return the output.
 *
 * @param cmd The command to execute.
 * @return The output of the command and the exit status.
 */
std::pair<std::string, int> exec_encode(const std::string& cmd) {
    std::array<char, 128>                 buffer;
    std::string                           result;
    int                                   exit_status = 0;
    std::unique_ptr<FILE, int (*)(FILE*)> pipe(popen(cmd.c_str(), "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    exit_status = pclose(pipe.release());
    result.erase(std::remove(result.begin(), result.end(), '\n'), result.end());

    return {result, exit_status};
}

/**
 * @class EncodeTestParam
 * @brief Structure to hold the test parameters for encoding.
 *
 */
struct EncodeTestParam {
    std::string  test_name;
    std::string  input;
    std::string  expected_output;
    std::int32_t exit_code;
};

/**
 * @brief Print the test parameters to the output stream.
 *
 * @param param The test parameters.
 * @param os The output stream.
 */
void PrintTo(const EncodeTestParam& param, std::ostream* os) {
    // Truncate the input if it is too long
    if (param.input.length() > 20) {
        *os << param.input.substr(0, 20) << "...";
    } else {
        *os << param.input;
    }
}

/**
 * @class EncodeFFT
 * @brief Test fixture class for the FFT version of the program.
 *
 */
class EncodeFFT : public ::testing::TestWithParam<EncodeTestParam> {};

/**
 * @brief Test the expected output of the FFT version of the program.
 */
TEST_P(EncodeFFT, TestExpectedOutput) {
    EncodeTestParam param            = GetParam();
    std::string     temp_input_file  = std::filesystem::temp_directory_path() / "temp_input.txt";
    std::string     temp_output_file = std::filesystem::temp_directory_path() / "temp_output.wav";
    std::ofstream   temp_file(temp_input_file);
    temp_file << param.input;
    temp_file.close();

    std::string                 command = std::format("{} encode {} {} 2>/dev/null", BINARY_FFT, temp_input_file, temp_output_file);
    std::pair<std::string, int> result;

    try {
        result = exec_encode(command);
    } catch (const std::runtime_error& e) {
        std::cerr << "Error executing command: " << e.what() << std::endl;
    }

    std::string                 hash_command = std::format("ffprobe {} 2>&1 | tail -n2 | md5sum", temp_output_file);
    std::pair<std::string, int> hash_result;

    try {
        hash_result = exec_encode(hash_command);
    } catch (const std::runtime_error& e) {
        std::cerr << "Error executing hash command: " << e.what() << std::endl;
    }

    if (param.exit_code) {
        EXPECT_EQ(result.second >> 8, param.exit_code);
    } else {
        EXPECT_EQ(hash_result.first, param.expected_output);
    }
}

/**
 * @class AudioTestGoertzel
 * @brief Test fixture class for the Goertzel version of the program.
 *
 */
class EncodeGoertzel : public ::testing::TestWithParam<EncodeTestParam> {};

/**
 * @brief Test the expected output of the Goertzel version of the program.
 */
TEST_P(EncodeGoertzel, TestExpectedOutput) {
    EncodeTestParam param            = GetParam();
    std::string     temp_input_file  = std::filesystem::temp_directory_path() / "temp_input.txt";
    std::string     temp_output_file = std::filesystem::temp_directory_path() / "temp_output.wav";
    std::ofstream   temp_file(temp_input_file);
    temp_file << param.input;
    temp_file.close();

    std::string                 command = std::format("{} encode {} {}", BINARY_GOERTZEL, temp_input_file, temp_output_file);
    std::pair<std::string, int> result;

    try {
        result = exec_encode(command);
    } catch (const std::runtime_error& e) {
        std::cerr << "Error executing command: " << e.what() << std::endl;
    }


    std::string                 hash_command = std::format("ffprobe {} 2>&1 | tail -n2 | md5sum", temp_output_file);
    std::pair<std::string, int> hash_result;

    try {
        hash_result = exec_encode(hash_command);
    } catch (const std::runtime_error& e) {
        std::cerr << "Error executing hash command: " << e.what() << std::endl;
    }

    if (param.exit_code) {
        EXPECT_EQ(result.second >> 8, param.exit_code);
    } else {
        EXPECT_EQ(hash_result.first, param.expected_output);
    }
}

/**
 * @brief Load the test parameters from the TSV file.
 *
 * @param tsv_file The path to the TSV file.
 * @return A vector of EncodeTestParam objects.
 */
std::vector<EncodeTestParam> loadTestParams_encode(const std::string& tsv_file) {
    std::ifstream                file(tsv_file);
    std::vector<EncodeTestParam> params;
    std::string                  line, test_name, input, expected_output, exit_code;

    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << tsv_file << std::endl;
        return params;
    }

    std::getline(file, line);

    while (std::getline(file, line)) {
        std::istringstream ss(line);
        if (std::getline(ss, test_name, '\t') && std::getline(ss, input, '\t') && std::getline(ss, expected_output, '\t') && std::getline(ss, exit_code, '\t')) {
            params.push_back({test_name, input, expected_output, std::stoi(exit_code)});
        } else {
            std::cerr << "Failed to parse line: " << line << std::endl;
        }
    }
    return params;
}

/**
 * @brief The test parameters for the audio files.
 */
std::vector<EncodeTestParam> audio_test_params_encode = loadTestParams_encode(TSV_FILE);

/**
 * @brief Generate a custom test name for the test.
 *
 * @param info The test parameter info.
 * @return The custom test name.
 */
std::string CustomTestNameGenerator_encode(const ::testing::TestParamInfo<EncodeTestParam>& info) {
    return info.param.test_name;
}

// Instantiate the test suites
INSTANTIATE_TEST_SUITE_P(AudioEncodeTests, EncodeFFT, ::testing::ValuesIn(audio_test_params_encode), CustomTestNameGenerator_encode);
INSTANTIATE_TEST_SUITE_P(AudioEncodeTests, EncodeGoertzel, ::testing::ValuesIn(audio_test_params_encode), CustomTestNameGenerator_encode);

// Allow uninstantiated parameterized tests
GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(EncodeFFT);
GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(EncodeGoertzel);
