cmake_minimum_required(VERSION 3.25.0)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

option(SENSOR_TEST "Generate tests." OFF)
option(SENSOR_BIND "Generate pybindings." OFF)
option(SENSOR_IDF "Generate esp idf component." ${ESP_PLATFORM})

if (SENSOR_BIND)
  project(sensor_compress VERSION 0.1.0 LANGUAGES C CXX)
  add_subdirectory(bind)
endif()

if(SENSOR_IDF)
  idf_component_register()
endif()

add_subdirectory(lib)

if (SENSOR_IDF)
  target_include_directories(lib_sensor_compress INTERFACE "lib/include")
  target_link_libraries(${COMPONENT_LIB} INTERFACE lib_sensor_compress)
else()
  target_link_libraries(sensor_compress PRIVATE lib_sensor_compress)
endif()