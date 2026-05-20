cmake_minimum_required(VERSION 3.25)

project(Cleo LANGUAGES C CXX ASM)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_library(cleo_warnings INTERFACE)
target_compile_options(cleo_warnings INTERFACE
    -Wall
    -Wextra
    -Wpedantic
    -Wshadow
    -Wconversion
    -Wnon-virtual-dtor
    -Wold-style-cast
    -Wcast-align
)

add_library(cleo_freestanding INTERFACE)
target_compile_options(cleo_freestanding INTERFACE
    -ffreestanding
    -nostdlib
    -nostdlib++
    -nostdlibinc
    -fno-rtti
    -fno-unwind-tables
)

include(CTest)

add_subdirectory(kernel)

if(BUILD_TESTING)
    find_package(GTest REQUIRED CONFIG)
    add_subdirectory(tests)
endif()

if (CMAKE_SYSTEM_NAME STREQUAL Generic)
    add_subdirectory(arch)
endif()
