cmake_minimum_required(VERSION 3.25)

project(Cleo LANGUAGES C CXX)

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
    -nostdinc
    -nostdinc++
    -fno-rtti
    -fno-unwind-tables
)

include(CTest)
find_package(GTest REQUIRED CONFIG)

add_subdirectory(kernel)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
