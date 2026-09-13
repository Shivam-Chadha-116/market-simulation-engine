cmake_minimum_required(VERSION 3.20)
project(QuantForgeLOB)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)


enable_testing()

add_executable(orderbook_tests tests/test_orderbook.cpp)
target_include_directories(orderbook_tests PRIVATE include)
target_link_libraries(orderbook_tests GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(orderbook_tests)