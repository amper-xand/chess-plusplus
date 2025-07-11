cmake_minimum_required(VERSION 3.30)

project(chessy)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BUILD_TYPE})

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

include_directories(src/)

include(FetchContent)
FetchContent_Declare(
  googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.17.0
)

FetchContent_MakeAvailable(googletest)

set(CHESSY_CORE_FILES
    src/core/representation.cpp
    src/core/generation.cpp
    src/core/magic.cpp
)

# main binary, the engine
add_executable(cheesy_engine
    src/main.cpp

    ${CHESSY_CORE_FILES}
)

# generates magic numbers for sliders LUTs
add_executable(maggen src/core/magic.cpp)
target_compile_definitions(maggen PRIVATE MAGIC_STANDALONE)

# testing binaries

enable_testing()

add_executable(test_core
    src/core/tests/tests.cpp

    ${CHESSY_CORE_FILES}
)

target_link_libraries(
  test_core
  GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(test_core)
