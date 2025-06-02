cmake_minimum_required(VERSION 3.10)
project(HPC_lab04_segmented)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

file(GLOB_RECURSE SEG_SRC src/*.c)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall -mavx2 -O3")

include_directories(
    include/stb
    include/
)

add_executable(segmentation ${SEG_SRC})
target_link_libraries(segmentation m)
