cmake_minimum_required(VERSION 3.30)

project(chessy)

set(EXECUTABLE_OUTPUT_PATH "bin")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

add_executable(chessy 
    src/main.cpp

    src/core/representation/representation.cpp

    src/core/generators/generators.cpp
    src/core/generators/magic/magic.cpp

    src/core/generators/piecewise/pawns.cpp
    src/core/generators/piecewise/sliders.cpp
    src/core/generators/piecewise/knights.cpp
    src/core/generators/piecewise/kings.cpp
)

add_executable(maggen src/core/generators/magic/magic.cpp)

target_compile_definitions(maggen PRIVATE MAGIC_STANDALONE)
