cmake_minimum_required(VERSION 3.16)
project(tfliteexecuter)

find_package(Boost COMPONENTS filesystem REQUIRED QUIET)

set(LIBS
    stdc++
    rt
    m
    pthread
)

add_library(${PROJECT_NAME} SHARED
    tfliteexecuter.cpp
    tfliteexecuter.h
)

# =========================================================
# TF Lite (Bazel-built shared library, single .so)
# =========================================================
# 3rdparty layout (recommended):
#   ../3rdparty/tflite/lib/libtensorflowlite.so
#   ../3rdparty/tflite/include/...
set(TFLITE_ROOT "${CMAKE_CURRENT_LIST_DIR}/../3rdparty/tflite")

add_library(tflite SHARED IMPORTED GLOBAL)
set_target_properties(tflite PROPERTIES
    IMPORTED_LOCATION "${TFLITE_ROOT}/lib/libtensorflowlite.so"
)

# One include root that contains:
#   tensorflow/...
#   absl/...
#   flatbuffers/...
target_include_directories(tflite INTERFACE
    "${TFLITE_ROOT}/include"
)

# Link your library against TF Lite .so
target_link_libraries(${PROJECT_NAME} PRIVATE
    ${LIBS}
    tflite
    Boost::filesystem
    ${CMAKE_DL_LIBS}
)

# Your local headers
target_include_directories(${PROJECT_NAME} PUBLIC .)

# Make runtime loader find libtensorflowlite.so without LD_LIBRARY_PATH
# (works both in build tree and after install if you keep the same relative layout)
set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 17
    BUILD_RPATH "\$ORIGIN;\$ORIGIN/../lib;${TFLITE_ROOT}/lib"
    INSTALL_RPATH "\$ORIGIN;\$ORIGIN/../lib;${TFLITE_ROOT}/lib"
)
