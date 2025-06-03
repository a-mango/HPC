cmake_minimum_required(VERSION 3.10)
project(FindKmers)

set(CMAKE_C_STANDARD 17)

set(CC_OPTS -Wall -g -O3 -march=native -mtune=native)

# v1
add_executable(k-mer_v1 src/main_v1.c)

# v2
add_executable(k-mer_v2 src/main_v2.c)
target_compile_options(k-mer_v2 PRIVATE ${CC_OPTS})

# v3
# add_executable(k-mer_v3 src/main_v3.c)
