add_subdirectory(lib)

file(GLOB_RECURSE SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.c)
file(GLOB_RECURSE HEADER_FILES ${CMAKE_SOURCE_DIR}/include/*.h)

find_path(SNDFILE_INCLUDE_DIR sndfile.h REQUIRED)
find_library(SNDFILE_LIBRARY sndfile REQUIRED)

if(NOT SNDFILE_INCLUDE_DIR OR NOT SNDFILE_LIBRARY)
  message(FATAL_ERROR "libsndfile non trouvé. Assurez-vous que libsndfile1 et libsndfile1-dev sont installés.")
endif()

# Filter source files for Goertzel and FFT implementations
list(FILTER SOURCE_FILES EXCLUDE REGEX ".*dtmf_decode_fft.c$")
add_executable(${CMAKE_PROJECT_NAME}-goertzel ${SOURCE_FILES})
target_include_directories(${CMAKE_PROJECT_NAME}-goertzel PRIVATE ${SNDFILE_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(${CMAKE_PROJECT_NAME}-goertzel PRIVATE ${SNDFILE_LIBRARY} ${LIB_NAME}-goertzel m)

list(FILTER SOURCE_FILES EXCLUDE REGEX ".*dtmf_decode_goertzel.c$")
add_executable(${CMAKE_PROJECT_NAME}-fft ${SOURCE_FILES})
target_include_directories(${CMAKE_PROJECT_NAME}-fft PRIVATE ${SNDFILE_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(${CMAKE_PROJECT_NAME}-fft PRIVATE ${SNDFILE_LIBRARY} ${LIB_NAME}-fft m fftw3)
