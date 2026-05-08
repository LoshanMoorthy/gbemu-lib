cmake_minimum_required(VERSION 3.20)
project(gbemu-lib VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE GBEMU_SOURCES
    "src/*.cpp"
    "src/*.cc"
)

if(NOT GBEMU_SOURCES)
    set(GBEMU_SOURCES "src/gbemu.cpp")
endif()

add_library(gbemu STATIC ${GBEMU_SOURCES})

target_include_directories(gbemu
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

option(GBEMU_BUILD_EXAMPLES "Build examples" ON)
if(GBEMU_BUILD_EXAMPLES)
    add_subdirectory(examples/sdl)
endif()