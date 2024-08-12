cmake_minimum_required(VERSION 3.25.0)
include(FetchContent)

project(sensor_compress VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

FetchContent_Declare(
  pybind11
  GIT_REPOSITORY https://github.com/pybind/pybind11.git
  GIT_TAG        941f45bcb51457884fa1afd6e24a67377d70f75c # v2.13.1
)
FetchContent_MakeAvailable(pybind11)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../lib/include)
pybind11_add_module(sensor_compress bind.cc)