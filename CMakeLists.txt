cmake_minimum_required(VERSION 3.8.0)
project(sensor_compress VERSION 0.1.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
file(GLOB SOURCES CONFIGURE_DEPENDS src/*.cpp src/*.cc)

add_library(sensor_compress ${SOURCES})

add_executable(format_selector format_search.cc)
target_link_libraries(format_selector sensor_compress)