cmake_minimum_required(VERSION 2.8.2)
project(hg-c-benchmark)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_COMPILER gcc)
set(CMAKE_C_FLAGS -O3)
set(CMAKE_C_FLAGS -Wall)
set(CMAKE_C_FLAGS -pedantic)

add_subdirectory(src)
add_subdirectory(test)