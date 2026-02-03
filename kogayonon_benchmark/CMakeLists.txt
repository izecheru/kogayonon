cmake_minimum_required(VERSION 3.26)

project(kogayonon_benchmark)

if (MSVC)
    add_compile_options(/utf-8)
endif()

add_executable(kogayonon_benchmark
    "include/benchmark.hpp"
    "src/main.cpp"
)

target_include_directories(kogayonon_benchmark
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_link_libraries(
    kogayonon_benchmark
    PRIVATE
        kogayonon_core
        kogayonon_utilities
        kogayonon_resources
        benchmark::benchmark
        benchmark::benchmark_main
)
