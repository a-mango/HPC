file(GLOB_RECURSE TEST_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

# Fetch Google in case it is not present locally
include(FetchContent)
fetchcontent_declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
  DOWNLOAD_EXTRACT_TIMESTAMP true
)
fetchcontent_makeavailable(googletest)

# Configure Google Test
enable_testing()

# Define the test executable name
set(TEST_NAME dtmf_test)

add_executable(${TEST_NAME} ${TEST_SOURCES})
target_include_directories(${TEST_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(${TEST_NAME} ${CMAKE_SOURCE_DIR}/lib/libdtmf_encdec_lib-goertzel.a ${CMAKE_SOURCE_DIR}/lib/libdtmf_encdec_lib-fft.a GTest::gtest_main)
include(GoogleTest)
gtest_discover_tests(${TEST_NAME})
