cmake_minimum_required(VERSION 3.10)
project(RAMSpeedTest)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -O3 -Wall")

add_executable(ram_speed_test ram_speed_test.cpp)