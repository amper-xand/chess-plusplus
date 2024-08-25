cmake_minimum_required(VERSION 3.30)

project(chessy)

set(EXECUTABLE_OUTPUT_PATH "bin")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

add_executable(chessy src/main.cpp src/game.cpp src/generators.cpp  src/magic.cpp)

add_executable(maggen src/magic.cpp)

target_compile_definitions(maggen PRIVATE MAGIC_STANDALONE)
