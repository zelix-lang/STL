include(FetchContent)
cmake_minimum_required(VERSION 3.14)
project(ZelixSTL)

set(CMAKE_CXX_STANDARD 20)

FetchContent_Declare(
        xxhash
        GIT_REPOSITORY https://github.com/Cyan4973/xxHash.git
        GIT_TAG        v0.8.3
)

FetchContent_MakeAvailable(xxhash)
file(GLOB_RECURSE ZELIX_STL_HEADERS CONFIGURE_DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/extras/*.h
        ${CMAKE_CURRENT_SOURCE_DIR}/extras/*.hpp)
add_library(ZelixSTL STATIC ${ZELIX_STL_HEADERS}
        include/zelix/unused.cpp)
add_library(zelix::stl ALIAS ZelixSTL)

target_include_directories(
        ZelixSTL
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/extras>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(ZelixSTL PUBLIC xxhash)