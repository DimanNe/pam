cmake_minimum_required(VERSION 3.13 FATAL_ERROR)
project(pamtest)

add_compile_options(-Wall -Wextra -pedantic -Werror)
set(CMAKE_CXX_STANDARD 20)

add_executable(pamtest
   main.cpp
)
target_link_libraries(pamtest
   pam
)
