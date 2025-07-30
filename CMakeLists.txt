include(FetchContent)
cmake_minimum_required(VERSION 3.31)
project(FluentSTL)

set(CMAKE_CXX_STANDARD 20)

add_library(FluentSTL INTERFACE)
add_library(fluent::stl ALIAS FluentSTL)

target_include_directories(
        FluentSTL
        INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)