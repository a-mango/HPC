cmake_minimum_required(VERSION 3.10)

###################
## Project setup ##
###################

project(dtmf_encdec VERSION 0.1 LANGUAGES C CXX)

# CMake options
set(CMAKE_COLOR_DIAGNOSTICS ON)
set(CMAKE_COLOR_MAKEFILE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# GCC version < 14 compatibility
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "14")
    message(STATUS "Detected GCC < 14: forcing compiler to /usr/bin/gcc-13 and /usr/bin/g++-13")
    set(CMAKE_C_COMPILER "/usr/bin/gcc-13" CACHE FILEPATH "C compiler" FORCE)
    set(CMAKE_CXX_COMPILER "/usr/bin/g++-13" CACHE FILEPATH "C++ compiler" FORCE)
  endif()
endif()

# C options
set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# C++ options
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compilation flags
add_compile_options(-Wall -Wextra -Wconversion -Wsign-conversion -pedantic -Wformat -Wshadow -Wpointer-arith -Wcast-align -Wstrict-overflow -Wparentheses -Wunreachable-code -Winline -Wswitch-enum -Wlogical-op -Wnull-dereference -Wformat-nonliteral -Wvla -Wdouble-promotion)

# Conditional compilation flags
if(CMAKE_BUILD_TYPE MATCHES Debug)
  add_definitions(-DDEBUG)
  add_compile_options(-g3 -fPIE -pie)
elseif(CMAKE_BUILD_TYPE MATCHES Release)
  add_definitions(-DNDEBUG)
  add_compile_options(-O3 -Werror)
endif()

# Enable sanitizers
# Requires: libasan, libubsan, liblsan, libtsan
# Windows users : try installing the LLVM toolchain or use WSL
option(ENABLE_ASAN "Enable asan" OFF)
if(ENABLE_ASAN AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang" AND CMAKE_BUILD_TYPE MATCHES Debug)
  # Check if the required sanitizing libraries are present
  set(SAN_OPT "-fsanitize=address,leak,undefined,pointer-compare,pointer-subtract,alignment,signed-integer-overflow,shift,integer-divide-by-zero,null")
  message(STATUS "Enabling sanitizers")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${SAN_OPT} -fstack-protector")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${SAN_OPT}")
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${SAN_OPT}")
  set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} ${SAN_OPT}")
endif()

###################
## Targets setup ##
###################

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

# Setup the binary output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BIN_PATH})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${LIB_PATH})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${LIB_PATH})

# Include directories
include_directories(${INC_PATH})
include_directories(${INC_PATH}/lib)

# Add subdirectories
add_subdirectory(${SRC_PATH})
add_subdirectory(${TEST_PATH})
