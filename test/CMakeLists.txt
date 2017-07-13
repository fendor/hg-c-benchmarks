add_executable(all_tests allTests.cpp ../src/util/util.c ../src/convolution/convolution-util.c ../src/convolution/convolution-run.c ../src/nbody/nbody-run.h ../src/nbody/nbody-run.c ../src/nbody/nbody-util.c)
target_link_libraries(all_tests gtest_main)
target_link_libraries(all_tests gtest)
set_target_properties(all_tests PROPERTIES COMPILE_FLAGS -fopenmp)
set_target_properties(all_tests PROPERTIES LINK_FLAGS -fopenmp)

add_test(NAME AllTests COMMAND all_tests)
