add_executable(OpenMP nbody.c)
set_target_properties(OpenMP PROPERTIES COMPILE_FLAGS -fopenmp)
set_target_properties(OpenMP PROPERTIES LINK_FLAGS -fopenmp)
target_link_libraries(OpenMP m)