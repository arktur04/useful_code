cmake_minimum_required(VERSION 3.10)
project(MmapSpeedTest)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -Wall")

add_executable(mmap_speed_test
    mmap_speed_test.cpp
)

if (UNIX)
    target_compile_options(mmap_speed_test PRIVATE -Wall -Wextra -pedantic)
endif()
