cmake_minimum_required(VERSION 3.10)
project(HPC_lab04_simd)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(GS_SRC
    src/main.c
    src/image.c
)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall -mavx2 -O3")

include_directories(
    include/stb
    include/
)

add_executable(grayscale_c ${GS_SRC} src/grayscale_c.c)
target_link_libraries(grayscale_c m)

add_executable(grayscale_simd ${GS_SRC} src/grayscale_simd.c)
target_link_libraries(grayscale_simd m)
