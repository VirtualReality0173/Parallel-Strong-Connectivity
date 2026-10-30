cmake_minimum_required(VERSION 3.16)

project(ParallelSCC VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(OpenMP)

add_executable(parallel_scc
    src/main.cpp
    src/graph.cpp
    src/sequential/tarjan.cpp
    src/parallel/reachability.cpp
    src/parallel/pivot_scc.cpp
    src/parallel/recursive_scc.cpp
)

target_include_directories(parallel_scc PRIVATE include)

if(OpenMP_CXX_FOUND)
    target_link_libraries(parallel_scc PRIVATE OpenMP::OpenMP_CXX)
endif()

enable_testing()
