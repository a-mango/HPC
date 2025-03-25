# Specify common source files
set(COMMON_SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/dtmf_common.c
    ${CMAKE_CURRENT_SOURCE_DIR}/dtmf_encode.c
)

# Specify source files for Goertzel implementation
set(GOERTZEL_SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/dtmf_decode_goertzel.c
    ${COMMON_SOURCE_FILES}
)

# Specify source files for FFT implementation
set(FFT_SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/dtmf_decode_fft.c
    ${COMMON_SOURCE_FILES}
)

# Specify header files
set(LIB_HEADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/dtmf_common.h
)

# Add Goertzel library
add_library(${LIB_NAME}-goertzel STATIC ${GOERTZEL_SOURCE_FILES} ${LIB_HEADER_FILES})
target_include_directories(${LIB_NAME}-goertzel PRIVATE ${CMAKE_SOURCE_DIR}/include/lib)

# Add FFT library
add_library(${LIB_NAME}-fft STATIC ${FFT_SOURCE_FILES} ${LIB_HEADER_FILES})
target_include_directories(${LIB_NAME}-fft PRIVATE ${CMAKE_SOURCE_DIR}/include/lib)
target_link_libraries(${LIB_NAME}-fft fftw3)
