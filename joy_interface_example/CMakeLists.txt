cmake_minimum_required(VERSION 3.5)

## write your package name
project(joy_interface_example)

set(CMAKE_PREFIX_PATH $ENV{RAISIN_WS}/install)

## added your dependencies here
find_package(raisin_network REQUIRED)

add_executable(joy_interface_example src/joy_interface_example.cpp)
target_link_libraries(joy_interface_example raisin_network)
target_include_directories(joy_interface_example PRIVATE  $ENV{RAISIN_WS}/install)