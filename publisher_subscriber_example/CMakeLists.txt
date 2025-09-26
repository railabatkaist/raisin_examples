cmake_minimum_required(VERSION 3.25)

## write your package name
project(publisher_subscriber_example)

set(CMAKE_PREFIX_PATH $ENV{RAISIN_WS}/install)

## added your dependencies here
find_package(raisin_network REQUIRED)

add_executable(publisher src/publisher.cpp)
target_link_libraries(publisher raisin_network)
target_include_directories(publisher PRIVATE  $ENV{RAISIN_WS}/install)
add_executable(subscriber src/subscriber.cpp)
target_link_libraries(subscriber raisin_network)
target_include_directories(subscriber PRIVATE  $ENV{RAISIN_WS}/install)
