cmake_minimum_required(VERSION 3.25.0)
project(MetasequoiaVoiceInput VERSION 1.0)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Windows specific definitions
add_definitions(
    /D_UNICODE=1
    /DUNICODE=1
    /DNOMINMAX
)

# Dependencies
find_package(fmt REQUIRED)

# whisper.cpp
set(WHISPER_BUILD_EXAMPLES OFF CACHE BOOL "whisper: build examples" FORCE)
set(WHISPER_BUILD_TESTS OFF CACHE BOOL "whisper: build tests" FORCE)
set(WHISPER_BUILD_SERVER OFF CACHE BOOL "whisper: build server example" FORCE)
add_subdirectory(third_party/whisper.cpp)

# miniaudio
include_directories(third_party/miniaudio)

# Source files
file(GLOB_RECURSE SOURCE_FILES "src/*.cpp" "src/*.h")

set(MY_EXECUTABLE_NAME "MetasequoiaVoiceInput")
add_executable(${MY_EXECUTABLE_NAME} ${SOURCE_FILES})

# Link libraries
target_link_libraries(${MY_EXECUTABLE_NAME}
    PRIVATE
    whisper
    fmt::fmt
    User32
)

# Target include directories
target_include_directories(${MY_EXECUTABLE_NAME}
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/whisper.cpp/include
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/miniaudio
)
