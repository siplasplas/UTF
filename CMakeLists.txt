cmake_minimum_required(VERSION 3.13)
project(GTests_utf)

add_definitions("-Werror=return-type")

add_executable(${PROJECT_NAME}
        test.cpp)

target_link_libraries(${PROJECT_NAME} gtest gtest_main)
