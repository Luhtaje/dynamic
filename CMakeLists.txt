cmake_minimum_required(VERSION 3.22)

set(This DynamicRingBuffer)

project(${This} C CXX)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

enable_testing()

add_subdirectory(googletest)

include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

include_directories(include)

add_executable(RunTests test/test_ring_buffer.cpp test/test_iterators.cpp include/ring_buffer.hpp)

target_link_libraries(RunTests gtest gtest_main)