cmake_minimum_required(VERSION 2.8.2)
project(hg-c-benchmark)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_COMPILER gcc)
set(CMAKE_C_FLAGS "-Ofast -Wall -pedantic -fopenmp")

add_executable(Nbody-OpenMP src/nbody.c)
set_target_properties(Nbody-OpenMP PROPERTIES COMPILE_FLAGS -fopenmp)
set_target_properties(Nbody-OpenMP PROPERTIES LINK_FLAGS -fopenmp)
target_link_libraries(Nbody-OpenMP m)