cmake_minimum_required(VERSION 3.5)

## write your package name
project(service_client_example)

set(CMAKE_PREFIX_PATH $ENV{RAISIN_WS}/install)

## added your dependencies here
find_package(raisin_network REQUIRED)

add_executable(service src/service.cpp)
target_link_libraries(service raisin_network)
target_include_directories(service PRIVATE  $ENV{RAISIN_WS}/install)
add_executable(client src/client.cpp)
target_link_libraries(client raisin_network)
target_include_directories(client PRIVATE  $ENV{RAISIN_WS}/install)
