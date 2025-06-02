cmake_minimum_required(VERSION 3.10)
project(ramdisk)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(
    ${PROJECT_SOURCE_DIR}
)

add_executable(ramdisk
    ramdisk.cpp
)

if (UNIX)
    target_compile_options(ramdisk PRIVATE -Wall -Wextra -pedantic)
endif()
