cmake_minimum_required(VERSION 3.30)

project(chessy)

set(EXECUTABLE_OUTPUT_PATH "bin")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

add_executable(chessy 
    src/main.cpp
    src/core/game.cpp
    src/core/generators/generators.cpp
    src/core/generators/magic.cpp
    src/core/generators/helpers.cpp
)

add_executable(maggen src/core/generators/magic.cpp)

target_compile_definitions(maggen PRIVATE MAGIC_STANDALONE)
