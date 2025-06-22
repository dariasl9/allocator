cmake_minimum_required(VERSION 3.10)
project(my_allocator_and_container LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(my_allocator my_allocator.cpp)