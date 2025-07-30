cmake_minimum_required(VERSION 3.31)
project(FluentSTL)

set(CMAKE_CXX_STANDARD 20)

add_library(
        FluentSTL STATIC
        library.cpp
        include/fluent/container/external_string.h
        include/fluent/container/owned_string.h
        include/fluent/container/vector.h
        include/fluent/except/exception.h
)
target_include_directories(FluentSTL PRIVATE include)
