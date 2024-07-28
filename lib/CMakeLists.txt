cmake_minimum_required(VERSION 3.25.0)
project(lib_sensor_compress VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_subdirectory(extern)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
file(GLOB SOURCES CONFIGURE_DEPENDS src/*.cpp src/*.cc)

add_library(lib_sensor_compress ${SOURCES})
target_link_libraries(lib_sensor_compress nlohmann_json expected fmt-header-only)

# Unit tests
if (SENSOR_TEST)
  include_directories(${googletest_SOURCE_DIR}/googletest/include)
  include_directories(${googletest_SOURCE_DIR}/googlemock/include)
  enable_testing()

  file(GLOB TEST_SOURCES CONFIGURE_DEPENDS test/*.cpp test/*.cc)
  add_executable(
    sensor_test
    ${TEST_SOURCES}
  )
  target_link_libraries(
    sensor_test
    GTest::gtest_main
    lib_sensor_compress
  )
  target_compile_options(sensor_test PRIVATE -fsanitize=address,undefined)
  target_link_options(sensor_test PRIVATE -fsanitize=address,undefined)
  include(GoogleTest)
  gtest_discover_tests(sensor_test)
endif()