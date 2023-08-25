cmake_minimum_required(VERSION 3.13)
project(GTests_utf)

set(CMAKE_CXX_STANDARD 17)

add_definitions("-Werror=return-type")

add_executable(${PROJECT_NAME}
        test.cpp)

target_link_libraries(${PROJECT_NAME} gtest gtest_main)
