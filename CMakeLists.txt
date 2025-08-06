include(FetchContent)
cmake_minimum_required(VERSION 3.31)
project(FluentSTL)

set(CMAKE_CXX_STANDARD 20)

FetchContent_Declare(
        xxhash
        GIT_REPOSITORY https://github.com/Cyan4973/xxHash.git
        GIT_TAG        v0.8.3
)

FetchContent_MakeAvailable(xxhash)
file(GLOB_RECURSE FLUENT_HEADERS CONFIGURE_DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/extras/*.h
        ${CMAKE_CURRENT_SOURCE_DIR}/extras/*.hpp)
add_library(FluentSTL STATIC ${FLUENT_HEADERS}
        include/fluent/unused.cpp)
add_library(fluent::stl ALIAS FluentSTL)

target_include_directories(
        FluentSTL
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/extras>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(FluentSTL PUBLIC xxhash)