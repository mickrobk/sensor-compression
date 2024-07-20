cmake_minimum_required(VERSION 3.25.0)

project(sensor_compress VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_subdirectory(pybind11)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../lib/include)
pybind11_add_module(sensor_compress bind.cc)
