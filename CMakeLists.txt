cmake_minimum_required(VERSION 3.20)

project(DiamondRushVM LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(GNUInstallDirs)

include_directories(include)

file(GLOB_RECURSE SOURCES
    src/*.cpp
)

add_executable(drvm ${SOURCES})

target_compile_options(drvm PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(drvm PRIVATE
        -g3
        -O0
    )
else()
    target_compile_options(drvm PRIVATE
        -O3
    )
endif()