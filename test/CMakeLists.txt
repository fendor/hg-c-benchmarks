add_executable(nbody_tests allNbodyTests.cpp ../src/util/util.c ../src/nbody/nbody-util.c ../src/nbody/nbody-run.c)
target_link_libraries(nbody_tests gtest_main)
target_link_libraries(nbody_tests gtest)
set_target_properties(nbody_tests PROPERTIES COMPILE_FLAGS -fopenmp)
set_target_properties(nbody_tests PROPERTIES LINK_FLAGS -fopenmp)

add_executable(convolution_tests allConvolutionTests.cpp ../src/util/util.c ../src/convolution/convolution-util.c ../src/convolution/convolution-run.c)
target_link_libraries(convolution_tests gtest_main)
target_link_libraries(convolution_tests gtest)
set_target_properties(convolution_tests PROPERTIES COMPILE_FLAGS -fopenmp)
set_target_properties(convolution_tests PROPERTIES LINK_FLAGS -fopenmp)


add_test(NAME AllTests COMMAND nbody_tests)
