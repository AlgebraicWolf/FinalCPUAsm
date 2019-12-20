cmake_minimum_required(VERSION 3.15)
project(CPU)

set(CMAKE_CXX_STANDARD 14)

add_executable(CPU main.cpp)
add_library(StackLibrary stack.cpp stack.h)
add_library(MurMurHash3 MurMurHash3.cpp MurMurHash3.h)

target_link_libraries(CPU StackLibrary MurMurHash3)