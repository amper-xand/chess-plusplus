cmake_minimum_required(VERSION 3.30)

project(chessy)

set(EXECUTABLE_OUTPUT_PATH "bin")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

add_executable(chessy 
    src/main.cpp

    src/core/representation.cpp

    src/core/generators.cpp
    src/core/magic/magic.cpp

    src/core/generators/pawns.cpp
    src/core/generators/sliders.cpp
    src/core/generators/knights.cpp
    src/core/generators/kings.cpp
)

add_executable(maggen src/core/magic/magic.cpp)

include_directories(src/)

target_compile_definitions(maggen PRIVATE MAGIC_STANDALONE)
