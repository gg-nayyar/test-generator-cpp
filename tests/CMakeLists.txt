
cmake_minimum_required(VERSION 3.10)
project(test_executable)

enable_testing()

# Add src folder (dev code, readonly)
add_subdirectory(C:/Users/KIIT/Projects/cpp-tests/src C:/Users/KIIT/Projects/cpp-tests/tests/build/src_build)


# GoogleTest
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/heads/main.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

file(GLOB_RECURSE TEST_SOURCES "C:/Users/KIIT/Projects/cpp-tests/tests/test_src/*.cc")
add_executable(test_executable ${TEST_SOURCES})
target_link_libraries(test_executable GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(test_executable)
