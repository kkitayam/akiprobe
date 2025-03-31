# This settings for pakcage version google test of Ubuntu 22.04
# The following packages are needed
# - libgtest-dev
# - libgmock-dev
cmake_minimum_required(VERSION 3.22)
project(unit_test)

set(CMAKE_CXX_STANDARD 11)  # for Google Test version 1.11
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_testing()
find_package(GTest REQUIRED)
include(GoogleTest)

add_executable(class_driver_api_tests
  ${CMAKE_CURRENT_SOURCE_DIR}/../src/cmsis_dap_device.c
  class_test.cpp
  mock_tinyusb.cpp
)

target_compile_options(class_driver_api_tests PRIVATE
  -fprofile-arcs
  -ftest-coverage
)
target_compile_definitions(class_driver_api_tests PRIVATE
  CFG_TUSB_MCU=OPT_MCU_NONE
  CFG_DCD_ENDPOINT_MAX=8
  TUP_DCD_ENDPOINT_MAX=8
)
target_include_directories(class_driver_api_tests PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/
  ${CMAKE_CURRENT_SOURCE_DIR}/../src
  ${CMAKE_CURRENT_SOURCE_DIR}/../lib/tinyusb/src
  ${CMAKE_CURRENT_SOURCE_DIR}/../lib/CMSIS-DAP/Firmware/Include
)

target_link_libraries(class_driver_api_tests
  GTest::gmock_main
  gcov
)
gtest_discover_tests(class_driver_api_tests)
