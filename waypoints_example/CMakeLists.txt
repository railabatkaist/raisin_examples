cmake_minimum_required(VERSION 3.5)

## write your package name
project(waypoints_example)

set(CMAKE_PREFIX_PATH $ENV{RAISIN_WS}/install)

## added your dependencies here
find_package(raisin_network REQUIRED)

add_executable(waypoints_example src/waypoints_example.cpp)
target_link_libraries(waypoints_example raisin_network)
target_include_directories(waypoints_example PRIVATE  $ENV{RAISIN_WS}/install)