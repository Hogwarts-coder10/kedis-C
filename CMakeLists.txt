cmake_minimum_required(VERSION 3.15)
project(kedis-c C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Default to Debug build with useful warnings if none specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

add_compile_options(-Wall -Wextra -Wpedantic)

# --- Public headers ---
include_directories(${PROJECT_SOURCE_DIR}/include)

# --- Main executable ---
add_executable(kedis-c
    src/main.c
)

# --- Testing ---
enable_testing()

add_executable(test_main
    tests/test_main.c
)

add_test(NAME test_main COMMAND test_main)
