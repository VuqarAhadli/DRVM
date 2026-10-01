cmake_minimum_required(VERSION 3.20)

project(
    DiamondRushVM
    VERSION 1.0.0
    DESCRIPTION "J2ME JVM and class-file disassembler"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    "${PROJECT_SOURCE_DIR}/src/*.cpp"
)

add_executable(drvm
    ${SOURCES}
)

target_include_directories(drvm PRIVATE
    "${PROJECT_SOURCE_DIR}/include"
)

target_compile_options(drvm PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -O3
)

add_executable(drvmopt EXCLUDE_FROM_ALL
    ${SOURCES}
)

target_include_directories(drvmopt PRIVATE
    "${PROJECT_SOURCE_DIR}/include"
)

target_compile_options(drvmopt PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -O3
)

add_executable(drvmnoopt EXCLUDE_FROM_ALL
    ${SOURCES}
)

target_include_directories(drvmnoopt PRIVATE
    "${PROJECT_SOURCE_DIR}/include"
)

target_compile_options(drvmnoopt PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -O0
    -g
)

add_custom_target(debugbuild
    DEPENDS
        drvmopt
        drvmnoopt
)