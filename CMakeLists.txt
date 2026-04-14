cmake_minimum_required(VERSION 3.10)
project(ASPC VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add the library
add_library(ASPC STATIC ASPC.cpp)

# Include directories
target_include_directories(ASPC PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Optionally, add compile options
target_compile_options(ASPC PRIVATE -Wall -Wextra)