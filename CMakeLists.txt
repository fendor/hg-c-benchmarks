cmake_minimum_required(VERSION 2.8.2)
project(hg-c-benchmarks)

set(CMAKE_CXX_STANDARD 99)
set(CMAKE_C_COMPILER gcc)
set(CMAKE_C_FLAGS "-O3 -Wall -pedantic -mavx")

add_executable(Nbody-OpenMP src/nbody.c src/util.c)
set_target_properties(Nbody-OpenMP PROPERTIES COMPILE_FLAGS -fopenmp)
set_target_properties(Nbody-OpenMP PROPERTIES LINK_FLAGS -fopenmp)
target_link_libraries(Nbody-OpenMP m)

add_executable(2D-Convolution src/2d-convolution.c src/util.c)
set_target_properties(2D-Convolution PROPERTIES COMPILE_FLAGS -fopenmp)
set_target_properties(2D-Convolution PROPERTIES LINK_FLAGS -fopenmp)
target_link_libraries(2D-Convolution  m)
