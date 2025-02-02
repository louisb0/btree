cmake_minimum_required(VERSION 3.20)
project(btree)

set(CMAKE_BUILD_TYPE Release)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(COMMON_WARNINGS
    -Werror -Wall -Wextra -Wpedantic
    -Wshadow
    -Wnon-virtual-dtor
    -Wold-style-cast
    -Wcast-align
    -Woverloaded-virtual
    -Wmisleading-indentation
    -Wduplicated-cond
    -Wduplicated-branches
    -Wlogical-op
    -Wnull-dereference
    -Wdouble-promotion
    -Wformat=2
)

set(COMMON_FLAGS
    -O3
    -mavx512f
    -mavx512vl
    -mbmi
)

include(FetchContent)

# Google Benchmark
FetchContent_Declare(
    benchmark
    GIT_REPOSITORY https://github.com/google/benchmark.git
    GIT_TAG v1.8.3
)
set(BENCHMARK_ENABLE_TESTING OFF)
set(BENCHMARK_ENABLE_GTEST_TESTS OFF)
set(BENCHMARK_BUILD_TYPE Release)

# Google Test
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
)
set(INSTALL_GTEST OFF)
set(BUILD_GMOCK OFF)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(benchmark googletest)

# Benchmark
add_executable(bench src/benchmark.cpp)
target_include_directories(bench PRIVATE src)
target_compile_options(bench PRIVATE ${COMMON_WARNINGS} ${COMMON_FLAGS})
target_link_libraries(bench PRIVATE benchmark::benchmark)
set_target_properties(bench PROPERTIES 
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

# Tests
add_executable(test src/tests.cpp)
target_include_directories(test PRIVATE src)
target_compile_options(test PRIVATE ${COMMON_WARNINGS} ${COMMON_FLAGS})
target_link_libraries(test PRIVATE GTest::gtest_main)
set_target_properties(test PROPERTIES 
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

# Profile
add_executable(profile src/profile.cpp)
target_include_directories(profile PRIVATE src)
target_compile_options(profile PRIVATE 
    ${COMMON_WARNINGS}
    ${COMMON_FLAGS}
    -g
)
set_target_properties(profile PROPERTIES 
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)
