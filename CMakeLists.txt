include(FetchContent)
cmake_minimum_required(VERSION 3.31)
project(FluentSTL)

set(CMAKE_CXX_STANDARD 20)

FetchContent_Declare(
        xxhash
        GIT_REPOSITORY https://github.com/Cyan4973/xxHash.git
        GIT_TAG        v0.8.3
)

add_library(FluentSTL INTERFACE)
add_library(fluent::stl ALIAS FluentSTL)

target_include_directories(
        FluentSTL
        INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

target_link_libraries(FluentSTL INTERFACE xxhash)
target_include_directories(FluentSTL INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/extras)