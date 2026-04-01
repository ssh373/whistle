project(WhistleDetectionRelease)

cmake_minimum_required(VERSION 3.16)

if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

add_subdirectory(whistledetection)
add_subdirectory(soundcapturing)
add_subdirectory(tfliteexecuter)

add_executable(${PROJECT_NAME}
    example.cpp
)

set(TFLITE_ROOT "${CMAKE_SOURCE_DIR}/3rdparty/tflite")

add_library(tflite_c SHARED IMPORTED GLOBAL)
set_target_properties(tflite_c PROPERTIES
  IMPORTED_LOCATION "${TFLITE_ROOT}/lib/libtensorflowlite_c.so"
)
target_include_directories(tflite_c INTERFACE
  "${TFLITE_ROOT}"
)


target_link_libraries(${PROJECT_NAME}
    WhistleDetection
    SoundCapturing
    tfliteexecuter
    tflite_c
)

set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 17)
