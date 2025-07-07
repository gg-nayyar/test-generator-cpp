
cmake_minimum_required(VERSION 3.14)
project(cpp_test_runner)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "C:/Users/KIIT/Projects/cpp-tests/tests/build")

enable_testing()

include_directories("C:/Users/KIIT/Projects/cpp-tests/src")

file(GLOB_RECURSE SRC_FILES "C:/Users/KIIT/Projects/cpp-tests/src/*.cc")
file(GLOB_RECURSE TEST_SOURCES "C:/Users/KIIT/Projects/cpp-tests/tests/test_src/src/*.cc")

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/heads/main.zip
)
FetchContent_MakeAvailable(googletest)

add_executable(tests ${SRC_FILES} ${TEST_SOURCES})
target_link_libraries(tests gtest_main)
add_test(NAME AllTests COMMAND tests)
