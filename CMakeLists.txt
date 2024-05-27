cmake_minimum_required(VERSION 3.25.0)

project(lib_sensor_compress VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_subdirectory(bind)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
file(GLOB SOURCES CONFIGURE_DEPENDS src/*.cpp src/*.cc)

add_library(lib_sensor_compress ${SOURCES})

add_executable(format_selector format_search.cc)
target_link_libraries(format_selector lib_sensor_compress)
target_link_libraries(sensor_compress PRIVATE lib_sensor_compress)
