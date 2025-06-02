cmake_minimum_required(VERSION 3.0)
project(HPC_lab04_smid)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

file(GLOB_RECURSE SEG_SRC src/*.c)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -g -Wall -fno-inline -fno-tree-vectorize")

include_directories(
    ../lib/stb
    include/
)

add_executable(segmentation ${SEG_SRC})
target_link_libraries(segmentation m)
