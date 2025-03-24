add_subdirectory(lib)

set(COMMON_INCLUDE_DIRS ${SNDFILE_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/include)
set(COMMON_LINK_LIBS ${SNDFILE_LIBRARY} m)

file(GLOB_RECURSE SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.c)
file(GLOB_RECURSE HEADER_FILES ${CMAKE_SOURCE_DIR}/include/*.h)

find_path(SNDFILE_INCLUDE_DIR sndfile.h REQUIRED)
find_library(SNDFILE_LIBRARY sndfile REQUIRED)

if(NOT SNDFILE_INCLUDE_DIR OR NOT SNDFILE_LIBRARY)
  message(FATAL_ERROR "libsndfile non trouvé. Assurez-vous que libsndfile1 et libsndfile1-dev sont installés.")
endif()

# Filter source files for Goertzel implementation
list(FILTER SOURCE_FILES EXCLUDE REGEX ".*dtmf_decode_fft.c$")
set(GOERTZEL_TARGET ${CMAKE_PROJECT_NAME}-goertzel)
add_executable(${GOERTZEL_TARGET} ${SOURCE_FILES})
target_include_directories(${GOERTZEL_TARGET} PRIVATE ${COMMON_INCLUDE_DIRS})
target_link_libraries(${GOERTZEL_TARGET} PRIVATE ${COMMON_LINK_LIBS} ${LIB_NAME}-goertzel)
enable_asan(${GOERTZEL_TARGET})

# Filter source files for FFT implementation
list(FILTER SOURCE_FILES EXCLUDE REGEX ".*dtmf_decode_goertzel.c$")
set(FFT_TARGET ${CMAKE_PROJECT_NAME}-fft)
add_executable(${FFT_TARGET} ${SOURCE_FILES})
target_include_directories(${FFT_TARGET} PRIVATE ${COMMON_INCLUDE_DIRS})
target_link_libraries(${FFT_TARGET} PRIVATE ${COMMON_LINK_LIBS} ${LIB_NAME}-fft fftw3)
enable_asan(${FFT_TARGET})
