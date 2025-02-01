cmake_minimum_required(VERSION 3.20)
project(btree)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

string(APPEND CMAKE_CXX_FLAGS
    " -Werror -Wall -Wextra -Wpedantic"
    " -Wshadow"
    " -Wnon-virtual-dtor"
    " -Wold-style-cast"
    " -Wcast-align"
    " -Woverloaded-virtual"
    " -Wmisleading-indentation"
    " -Wduplicated-cond"
    " -Wduplicated-branches"
    " -Wlogical-op"
    " -Wnull-dereference"
    " -Wdouble-promotion"
    " -Wformat=2"
)
string(APPEND CMAKE_CXX_FLAGS " -O3 -mavx512f -mavx512vl -mbmi")

include(FetchContent)
FetchContent_Declare(
    benchmark
    GIT_REPOSITORY https://github.com/google/benchmark.git
    GIT_TAG v1.8.3
)
set(BENCHMARK_ENABLE_TESTING OFF)
FetchContent_MakeAvailable(benchmark)

# Benchmark
add_executable(bench src/benchmark.cpp)
target_include_directories(bench PRIVATE src)
target_link_libraries(bench PRIVATE benchmark::benchmark)
set_target_properties(bench PROPERTIES 
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

# Correctness
add_executable(correctness src/correctness.cpp)
target_include_directories(correctness PRIVATE src)
target_compile_options(correctness PRIVATE -g)
set_target_properties(correctness PROPERTIES 
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

# Profiling
add_executable(profiling src/profiling.cpp)
target_include_directories(profiling PRIVATE src)
target_compile_options(profiling PRIVATE -g)
set_target_properties(profiling PROPERTIES 
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)
