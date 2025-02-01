cmake_minimum_required(VERSION 3.20)
project(btree CXX)

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

add_executable(btree src/benchmark.cpp)
target_include_directories(btree PRIVATE src)
target_link_libraries(btree PRIVATE benchmark::benchmark)
