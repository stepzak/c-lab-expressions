cmake_minimum_required(VERSION 3.30)
project(c_lab_expressions C)

set(CMAKE_C_STANDARD 23)

if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

add_subdirectory(external/cvec)

add_executable(c_lab_expressions src/main.c
        include/unlikely.h
        include/expression_tree.h
        include/stack.h
        external/cvec/include/cvec/cvec.h
        src/lexer.c
        src/tree.c)

target_link_libraries(c_lab_expressions PRIVATE cvec)
