cmake_minimum_required(VERSION 3.16)
project(tfliteexecuter)

find_package(Boost COMPONENTS filesystem REQUIRED QUIET)

add_library(${PROJECT_NAME} SHARED
    tfliteexecuter.cpp
    tfliteexecuter.h
)

#
# We add the prebuild tflite static libraries. To build them every time would adds a tone of code and time overhead
#

if(NOT CMAKE_SYSTEM_PROCESSOR MATCHES "^(aarch64|arm64)$")
  message(FATAL_ERROR "This project is ARM64(aarch64) only. Current: ${CMAKE_SYSTEM_PROCESSOR}")
endif()

# ---- TensorFlow Lite (aarch64 prebuilt) ----
set(TFLITE_ROOT "${CMAKE_CURRENT_LIST_DIR}/../3rdparty/tflite")
set(TFLITE_LIB  "${TFLITE_ROOT}/aarch64/lib/libtensorflow-lite.a")
add_library(tensorflow-lite STATIC IMPORTED)
set_target_properties(tensorflow-lite PROPERTIES
    IMPORTED_LOCATION "${TFLITE_LIB}"
)
target_include_directories(${PROJECT_NAME} PUBLIC
    .
    "${TFLITE_ROOT}"
    "${TFLITE_ROOT}/aarch64/include"
)

target_link_libraries(${PROJECT_NAME}
    tensorflow-lite
    Boost::filesystem
    ${CMAKE_DL_LIBS}
    pthread
    m
    rt
)

set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 17)
