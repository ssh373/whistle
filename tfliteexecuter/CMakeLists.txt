cmake_minimum_required(VERSION 3.16)
project(tfliteexecuter)

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
# TF Lite (C API shared library)
# =========================================================
# 3rdparty layout:
#   ../3rdparty/tflite/lib/libtensorflowlite_c.so
#   ../3rdparty/tflite/tflite_c_api*.h
set(TFLITE_ROOT "${CMAKE_CURRENT_LIST_DIR}/../3rdparty/tflite")

# Link your library against TF Lite .so
target_link_libraries(${PROJECT_NAME} PUBLIC
    ${LIBS}
    tflite_c
    ${CMAKE_DL_LIBS}
)

target_link_options(${PROJECT_NAME} PRIVATE
    "-Wl,--no-as-needed"
    "-Wl,--no-undefined"
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
