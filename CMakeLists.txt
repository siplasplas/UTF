cmake_minimum_required(VERSION 3.13)
# 'Google_test' is the subproject name
project(Google_Tests_run)

add_definitions("-Werror=return-type")

# 'Google_Tests_run' is the target name
# 'test1.cpp tests2.cpp' are source files with tests
add_executable(Google_Tests_run
        test.cpp)

target_link_libraries(Google_Tests_run gtest gtest_main)
