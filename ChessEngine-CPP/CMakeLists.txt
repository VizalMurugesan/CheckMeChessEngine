cmake_minimum_required(VERSION 3.16)
project(ChessEngine)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Collect all .cpp files in src/
file(GLOB_RECURSE SOURCES "src/*.cpp")

add_executable(ChessEngine ${SOURCES})

# Enable optimizations for release builds
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(ChessEngine PRIVATE -O3)
endif()
