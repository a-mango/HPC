message("Enable perfmon is ${ENABLE_PERFMON}")
add_subdirectory(lib)

set(COMMON_INCLUDE_DIRS ${SNDFILE_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/vendor)
set(COMMON_LINK_LIBS ${SNDFILE_LIBRARY} m)

file(GLOB_RECURSE SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.c)
file(GLOB_RECURSE HEADER_FILES ${CMAKE_SOURCE_DIR}/include/*.h)

find_path(SNDFILE_INCLUDE_DIR sndfile.h REQUIRED)
find_library(SNDFILE_LIBRARY sndfile REQUIRED)

if(NOT SNDFILE_INCLUDE_DIR OR NOT SNDFILE_LIBRARY)
  message(FATAL_ERROR "libsndfile missing. Make sure libsndfile and libsndfile-dev are installed.")
endif()

find_path(FFTW3_INCLUDE_DIR fftw3.h REQUIRED)
find_library(FFTW3_LIBRARY fftw3 REQUIRED)

if(NOT FFTW3_INCLUDE_DIR OR NOT FFTW3_LIBRARY)
  message(FATAL_ERROR "libfftw3 missing. Make sure libfftw3 and libfftw3-dev are installed.")
endif()

# Filter source files for Goertzel implementation
set(SOURCE_FILES_GOERTZEL ${SOURCE_FILES})
list(FILTER SOURCE_FILES_GOERTZEL EXCLUDE REGEX ".*dtmf_decode_fft.c$")
set(GOERTZEL_TARGET ${CMAKE_PROJECT_NAME}-goertzel)
add_executable(${GOERTZEL_TARGET} ${SOURCE_FILES_GOERTZEL})
target_include_directories(${GOERTZEL_TARGET} PRIVATE ${COMMON_INCLUDE_DIRS})
target_link_libraries(${GOERTZEL_TARGET} PRIVATE ${COMMON_LINK_LIBS} ${LIB_NAME}-goertzel ${SNDFILE_LIBRARY})
enable_asan(${GOERTZEL_TARGET})

# Filter source files for FFT implementation
set(SOURCE_FILES_FFT ${SOURCE_FILES})
list(FILTER SOURCE_FILES_FFT EXCLUDE REGEX ".*dtmf_decode_goertzel.c$")
set(FFT_TARGET ${CMAKE_PROJECT_NAME}-fft)
add_executable(${FFT_TARGET} ${SOURCE_FILES_FFT})
target_include_directories(${FFT_TARGET} PRIVATE ${COMMON_INCLUDE_DIRS})
target_link_libraries(${FFT_TARGET} PRIVATE ${COMMON_LINK_LIBS} ${LIB_NAME}-fft ${FFTW3_LIBRARY} ${SNDFILE_LIBRARY})
enable_asan(${FFT_TARGET})

# Performance monitoring
if(ENABLE_PERFMON)
  message(STATUS "Enabling likwid")
  find_library(LIKWID_LIB likwid PATHS /usr/local/lib)

  if(NOT LIKWID_LIB)
    message(FATAL_ERROR "LIKWID library not found")
  endif()

  include_directories(/usr/local/include)

  target_compile_definitions(${FFT_TARGET} PRIVATE LIKWID_PERFMON)
  target_compile_options(${FFT_TARGET} PRIVATE -O3 -g3 -w)
  target_link_options(${FFT_TARGET} PRIVATE -O3 -g3 -w)
  target_link_libraries(${FFT_TARGET} PRIVATE ${LIKWID_LIB})

  target_compile_definitions(${GOERTZEL_TARGET} PRIVATE LIKWID_PERFMON)
  target_compile_options(${GOERTZEL_TARGET} PRIVATE -O3 -g3 -w)
  target_link_options(${GOERTZEL_TARGET} PRIVATE -O3 -g3 -w)
  target_link_libraries(${GOERTZEL_TARGET} PRIVATE ${LIKWID_LIB})
endif()

