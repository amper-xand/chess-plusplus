cmake_minimum_required(VERSION 3.30)

project(chessy)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BUILD_TYPE})

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

add_executable(cheesy_engine
    src/main.cpp

    src/core/representation.cpp
    src/core/generation.cpp
)

include_directories(src/)

add_executable(test_core
    src/core/tests/tests.cpp

    src/core/representation.cpp
    src/core/generation.cpp
)
