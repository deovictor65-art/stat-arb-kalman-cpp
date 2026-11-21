cmake_minimum_required(VERSION 3.20)
project(stat_arb_kalman VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (MSVC)
    add_compile_options(/W4 /O2 /permissive-)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -O3 -march=native)
endif()

add_library(statarb_core INTERFACE)
target_include_directories(statarb_core INTERFACE 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

add_executable(statarb_sim src/main.cpp)
target_link_libraries(statarb_sim PRIVATE statarb_core)

enable_testing()
add_executable(statarb_tests tests/test_kalman.cpp)
target_link_libraries(statarb_tests PRIVATE statarb_core)

add_test(NAME StatArbKalmanTests COMMAND statarb_tests)
