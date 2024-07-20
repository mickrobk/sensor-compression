cmake_minimum_required(VERSION 3.25.0)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if (NOT ESP_PLATFORM)
  project(sensor_compress VERSION 0.1.0 LANGUAGES C CXX)
  add_subdirectory(bind)
else()
  set(SENSOR_DISABLE_TESTS True)
  idf_component_register()
endif()

add_subdirectory(lib)

if (ESP_PLATFORM)
  target_include_directories(lib_sensor_compress INTERFACE "lib/include")
  target_link_libraries(${COMPONENT_LIB} INTERFACE lib_sensor_compress)
else()
  target_link_libraries(sensor_compress PRIVATE lib_sensor_compress)
endif()