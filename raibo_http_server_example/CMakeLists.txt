# use a real version here; see note below
cmake_minimum_required(VERSION 3.23)
project(raibo_http_server_example)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(crow CONFIG REQUIRED)           # package name is case-insensitive
find_package(ffmpeg REQUIRED)
find_package(raisin_network CONFIG REQUIRED)
find_package(raisin_ffmpeg_image_transport CONFIG REQUIRED)
find_package(OpenCV CONFIG REQUIRED)

add_executable(${PROJECT_NAME} main.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE
        Crow::Crow
        raisin_network
        raisin_ffmpeg_image_transport
        ${OpenCV_LIBS}
)

target_include_directories(${PROJECT_NAME} PRIVATE ${FFMPEG_INCLUDE_DIRS})
target_link_directories(${PROJECT_NAME} PRIVATE ${FFMPEG_LIBRARY_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${FFMPEG_LIBRARIES})

if (WIN32)
    install(DIRECTORY "${CMAKE_BINARY_DIR}/vcpkg_installed/${VCPKG_TARGET_TRIPLET}/"
            DESTINATION ".")
    add_compile_options("/utf-8")
endif()
