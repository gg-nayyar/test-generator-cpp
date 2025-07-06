cmake_minimum_required(VERSION 3.16)
project(UnitTests)

enable_testing()

add_subdirectory(${CMAKE_SOURCE_DIR}/src ${CMAKE_BINARY_DIR}/src_build)

find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

file(GLOB TEST_SOURCES ${CMAKE_SOURCE_DIR}/tests/test_src/**/*.cc)

add_executable(runUnitTests ${TEST_SOURCES})
target_link_libraries(runUnitTests PRIVATE drogon GTest::gtest_main)

add_test(NAME runTests COMMAND runUnitTests)