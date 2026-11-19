# SPDX-License-Identifier: Apache-2.0
cmake_minimum_required(VERSION 3.16)
project(jpegdec_fuzz LANGUAGES C CXX)

option(BUILD_CLANG_FUZZER "Build JPEGDEC libFuzzer targets" OFF)

if(NOT BUILD_CLANG_FUZZER)
  message(STATUS "JPEGDEC fuzzing disabled; set BUILD_CLANG_FUZZER=ON to enable")
  return()
endif()

if(NOT CMAKE_C_COMPILER_ID MATCHES "Clang" OR
   NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  message(FATAL_ERROR "BUILD_CLANG_FUZZER requires Clang for C and C++")
endif()

# Instrument the decoder itself, not only the libFuzzer entry points.
# Flags are target-local and cannot leak into normal JPEGDEC builds.
set(JPEGDEC_SANITIZERS -fsanitize=address,undefined
    -fno-sanitize-recover=undefined -fno-omit-frame-pointer)
add_library(jpegdec_fuzz_core STATIC ../linux/jpegdec_test_core.c)
target_include_directories(jpegdec_fuzz_core PUBLIC ../src ../linux)
target_compile_definitions(jpegdec_fuzz_core PUBLIC JPEGDEC_NO_ARDUINO NO_SIMD)
target_compile_options(jpegdec_fuzz_core PRIVATE
    ${JPEGDEC_SANITIZERS} -fsanitize=fuzzer-no-link)

foreach(name IN ITEMS header decode yuv)
  add_executable(jpegdec_${name}_fuzzer jpegdec_${name}_fuzzer.cc)
  target_compile_features(jpegdec_${name}_fuzzer PRIVATE cxx_std_11)
  target_compile_options(jpegdec_${name}_fuzzer PRIVATE
      ${JPEGDEC_SANITIZERS} -fsanitize=fuzzer-no-link -Wall -Wextra -Werror)
  target_link_options(jpegdec_${name}_fuzzer PRIVATE
      ${JPEGDEC_SANITIZERS} -fsanitize=fuzzer)
  target_link_libraries(jpegdec_${name}_fuzzer PRIVATE jpegdec_fuzz_core)
endforeach()
