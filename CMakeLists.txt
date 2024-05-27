cmake_minimum_required(VERSION 3.25.0)
project(sensor_compress VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_subdirectory(bind)
add_subdirectory(lib)

target_link_libraries(sensor_compress PRIVATE lib_sensor_compress)
