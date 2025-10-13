cmake_minimum_required(VERSION 3.10)
project(ipc_pipeline C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_executable(parent src/parent.c)
add_executable(child1 src/child1.c)
add_executable(child2 src/child2.c)

set_target_properties(parent PROPERTIES OUTPUT_NAME "parent.out")
set_target_properties(child1 PROPERTIES OUTPUT_NAME "child1.out")
set_target_properties(child2 PROPERTIES OUTPUT_NAME "child2.out")
