cmake_minimum_required(VERSION 3.24)

###################
## Project setup ##
###################

project(dtmf_encdec VERSION 0.1 LANGUAGES C CXX)

# CMake options
set(CMAKE_COLOR_DIAGNOSTICS ON)
set(CMAKE_COLOR_MAKEFILE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# GCC version < 14 compatibility
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS "14")
  message(STATUS "Detected GCC < 14: forcing compiler to /usr/bin/gcc-13 and /usr/bin/g++-13")
  set(CMAKE_C_COMPILER "/usr/bin/gcc-13" CACHE FILEPATH "C compiler" FORCE)
  set(CMAKE_CXX_COMPILER "/usr/bin/g++-13" CACHE FILEPATH "C++ compiler" FORCE)
endif()

# C options
set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# C++ options
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Create names and paths
set(EXE_NAME ${CMAKE_PROJECT_NAME})
set(LIB_NAME ${CMAKE_PROJECT_NAME}_lib)
set(TEST_NAME ${CMAKE_PROJECT_NAME}_test)

# Create paths
set(SRC_PATH ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(INC_PATH ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(BIN_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(LIB_PATH ${CMAKE_CURRENT_SOURCE_DIR}/lib)
set(TEST_PATH ${CMAKE_CURRENT_SOURCE_DIR}/test)
set(VENDOR_PATH ${CMAKE_CURRENT_SOURCE_DIR}/vendor)

# Setup the binary output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BIN_PATH})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${LIB_PATH})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${LIB_PATH})

# Include directories
include_directories(${INC_PATH})
include_directories(${INC_PATH}/lib)

# Compilation flags
add_compile_options(
  -Wall -Wextra -Wconversion -Wsign-conversion -pedantic
  -Wformat -Wshadow -Wpointer-arith -Wcast-align -Wstrict-overflow
  -Wparentheses -Wunreachable-code -Winline -Wswitch-enum -Wlogical-op
  -Wnull-dereference -Wformat-nonliteral -Wvla -Wdouble-promotion
)

# Conditional compilation flags
if(CMAKE_BUILD_TYPE MATCHES Debug)
  add_compile_definitions(DEBUG)
  add_compile_options(-g3 -fPIE -pie)
elseif(CMAKE_BUILD_TYPE MATCHES Release)
  add_compile_definitions(NDEBUG)
  add_compile_options(-O3 -Werror)
endif()

# Enable sanitizers in Debug mode
option(ENABLE_ASAN "Enable asan" OFF)
if(ENABLE_ASAN AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang" AND CMAKE_BUILD_TYPE MATCHES Debug)
  execute_process(COMMAND ${CMAKE_C_COMPILER} -print-file-name=libasan.so OUTPUT_VARIABLE ASAN_PATH OUTPUT_STRIP_TRAILING_WHITESPACE)
  get_filename_component(ASAN_DIR ${ASAN_PATH} DIRECTORY)

  find_library(ASAN_LIB NAMES asan libasan PATHS ${ASAN_DIR})
  find_library(UBSAN_LIB NAMES ubsan libubsan PATHS ${ASAN_DIR})
  find_library(LSAN_LIB NAMES lsan liblsan PATHS ${ASAN_DIR})
  find_library(TSAN_LIB NAMES tsan libtsan PATHS ${ASAN_DIR})

  if(ASAN_LIB AND UBSAN_LIB AND LSAN_LIB AND TSAN_LIB)
    message(STATUS "Enabling sanitizers for target ${target}")
    set(SAN_OPT "-fsanitize=address,leak,undefined,pointer-compare,pointer-subtract,alignment,signed-integer-overflow,shift,integer-divide-by-zero,null")
  else()
    message(WARNING "Sanitizer libraries not found. Disabling sanitizers for target ${target}.")
  endif()
endif()

function(enable_asan target)
  if(SAN_OPT)
    target_compile_options(${target} PRIVATE ${SAN_OPT} -fno-omit-frame-pointer -fstack-protector)
    target_link_libraries(${target} PRIVATE ${SAN_OPT})
  endif()
endfunction()

# Add subdirectories
add_subdirectory(${SRC_PATH})
add_subdirectory(${TEST_PATH})
