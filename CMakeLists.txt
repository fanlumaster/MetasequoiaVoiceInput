cmake_minimum_required(VERSION 3.25.0)
project(MetasequoiaVoiceInput VERSION 1.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Windows-specific settings
if(MSVC)
    add_definitions(
        /D_UNICODE=1
        /DUNICODE=1
        /DNOMINMAX
    )
endif()

# Dependencies
find_package(onnxruntime CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)

# whisper.cpp
set(WHISPER_BUILD_EXAMPLES OFF CACHE BOOL "whisper: build examples" FORCE)
set(WHISPER_BUILD_TESTS OFF CACHE BOOL "whisper: build tests" FORCE)
set(WHISPER_BUILD_SERVER OFF CACHE BOOL "whisper: build server example" FORCE)
set(WHISPER_USE_ONNX OFF CACHE BOOL "" FORCE)
set(WHISPER_USE_ONNXRT OFF CACHE BOOL "" FORCE)
add_subdirectory(third_party/whisper.cpp)

# miniaudio
include_directories(third_party/miniaudio)

# Source files (Explicit list to avoid GLOB issues)
set(SOURCE_FILES
    "src/audio_capture.cpp"
    "src/audio_capture.h"
    "src/silero_vad.cpp"
    "src/silero_vad.h"
    "src/whisper_worker.cpp"
    "src/whisper_worker.h"
    "src/send_input.cpp"
    "src/send_input.h"
    "src/main.cpp"
    "src/vad.cpp"
    "src/vad.h"
)

set(MY_EXECUTABLE_NAME "MetasequoiaVoiceInput")
add_executable(${MY_EXECUTABLE_NAME} ${SOURCE_FILES})

# Link libraries
target_link_libraries(${MY_EXECUTABLE_NAME}
    PRIVATE
    whisper
    fmt::fmt
    onnxruntime::onnxruntime
    User32
)

# Target include directories
target_include_directories(${MY_EXECUTABLE_NAME}
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/whisper.cpp/include
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/miniaudio
)
