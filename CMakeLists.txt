cmake_minimum_required(VERSION 2.8.2)
project(hg-c-benchmarks)

# Download and unpack googletest at configure time
#
configure_file(CMakeLists.txt.in googletest-download/CMakeLists.txt)
execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
        RESULT_VARIABLE result
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/googletest-download)
if (result)
    message(FATAL_ERROR "CMake step for googletest failed: ${result}")
endif ()
execute_process(COMMAND ${CMAKE_COMMAND} --build .
        RESULT_VARIABLE result
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/googletest-download)
if (result)
    message(FATAL_ERROR "Build step for googletest failed: ${result}")
endif ()

# Prevent overriding the parent project's compiler/linker
# settings on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Add googletest directly to our build. This defines
# the gtest and gtest_main targets.
add_subdirectory(${CMAKE_BINARY_DIR}/googletest-src
        ${CMAKE_BINARY_DIR}/googletest-build)

# The gtest/gtest_main targets carry header search path
# dependencies automatically when using CMake 2.8.11 or
# later. Otherwise we have to add them here ourselves.
if (CMAKE_VERSION VERSION_LESS 2.8.11)
    include_directories("${gtest_SOURCE_DIR}/include")
endif ()

set(CMAKE_VERBOSE_MAKEFILE on)
enable_testing()

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_COMPILER gcc)

add_executable(Nbody-OpenMP src/nbody.c src/util/util.c src/nbody/nbody-util.c src/nbody/nbody-run.c)
set_target_properties(Nbody-OpenMP PROPERTIES COMPILE_FLAGS "-O2 -march=broadwell -Wall -pedantic -fopenmp")
set_target_properties(Nbody-OpenMP PROPERTIES LINK_FLAGS -fopenmp)
target_link_libraries(Nbody-OpenMP m)

add_executable(2D-Convolution src/2d-convolution.c src/util/util.c src/convolution/convolution-util.c src/convolution/convolution-run.c)
set_target_properties(2D-Convolution PROPERTIES COMPILE_FLAGS "-O3 -march=broadwell -msse2 -mavx -Wall -pedantic -fopenmp")
set_target_properties(2D-Convolution PROPERTIES LINK_FLAGS -fopenmp)
target_link_libraries(2D-Convolution m)

add_subdirectory(test)