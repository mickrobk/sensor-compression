cmake_minimum_required(VERSION 3.25.0)
include(FetchContent)

set(JSON_ImplicitConversions OFF)
FetchContent_Declare(
  nlohmann_json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG        9cca280a4d0ccf0c08f47a99aa71d1b0e52f8d03 # v3.11.3
)

set(EXPECTED_BUILD_PACKAGE OFF)
set(EXPECTED_BUILD_TESTS OFF)
FetchContent_Declare(
  expected
  GIT_REPOSITORY https://github.com/TartanLlama/expected.git
  GIT_TAG        292eff8bd8ee230a7df1d6a1c00c4ea0eb2f0362 # v1.1.0
)

set(FMT_MODULE OFF)
set(FMT_INSTALL OFF)
set(FMT_OS OFF)
FetchContent_Declare(
  fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt.git
  GIT_TAG        0c9fce2ffefecfdce794e1859584e25877b7b592 # 11.0.2
)

set(UUID_USING_CXX20_SPAN ON)
FetchContent_Declare(
  stduuid
  GIT_REPOSITORY https://github.com/mariusbancila/stduuid.git
  GIT_TAG        3afe7193facd5d674de709fccc44d5055e144d7a # 1.2.3
)

FetchContent_Declare(
  base64
  GIT_REPOSITORY https://github.com/mickrobk/base64.git
)

FetchContent_MakeAvailable(nlohmann_json expected fmt stduuid base64)

if (SENSOR_TEST)
  FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
  )
  FetchContent_MakeAvailable(googletest)
endif()