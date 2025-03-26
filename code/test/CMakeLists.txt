enable_testing()
include(GoogleTest)

set_target_properties(gtest PROPERTIES FOLDER vendor COMPILE_FLAGS "-w")
set_target_properties(gtest_main PROPERTIES FOLDER vendor COMPILE_FLAGS "-w")
set_target_properties(gmock PROPERTIES FOLDER vendor COMPILE_FLAGS "-w")
set_target_properties(gmock_main PROPERTIES FOLDER vendor COMPILE_FLAGS "-w")

set_property(TARGET gtest_main APPEND_STRING PROPERTY COMPILE_OPTIONS "--param" "inline-unit-growth=4096")

mark_as_advanced(
    BUILD_GMOCK BUILD_GTEST BUILD_SHARED_LIBS
    gmock_build_tests gtest_build_samples gtest_build_tests
    gtest_disable_pthreads gtest_force_shared_crt gtest_hide_internal_symbols
)

set(TEST_NAME ${CMAKE_PROJECT_NAME}_test)
file(GLOB_RECURSE TEST_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
add_executable(${TEST_NAME} ${TEST_SOURCES})
target_compile_options(${TEST_NAME} PRIVATE)
target_include_directories(${TEST_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(${TEST_NAME} ${CMAKE_SOURCE_DIR}/lib/libdtmf_encdec_lib-goertzel.a ${CMAKE_SOURCE_DIR}/lib/libdtmf_encdec_lib-fft.a GTest::gtest_main)

gtest_discover_tests(${TEST_NAME} WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
