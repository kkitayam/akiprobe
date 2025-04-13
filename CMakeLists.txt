cmake_minimum_required(VERSION 3.12)

if (NOT PICO_SDK_PATH)
  set(PICO_SDK_FETCH_FROM_GIT on)
endif ()

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/pico_sdk_import.cmake)

find_package(Git QUIET)
if (GIT_FOUND)
  execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init lib/tinyusb
    WORKING_DIRECTORY ${PICO_SDK_PATH}
    RESULT_VARIABLE GIT_SUBMOD_RESULT)
  if(NOT GIT_SUBMOD_RESULT EQUAL "0")
    message(FATAL_ERROR "git submodule update --init failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
  endif()
endif()

project(akiprobe)

if (NOT PICO_BOARD)
  set(PICO_BOARD pico)
endif()
if (NOT PICO_COMPILER)
  set(PICO_COMPILER pico_arm_gcc)
endif()
set(PICO_COPY_TO_RAM 1)

pico_sdk_init()

add_executable(akiprobe
  ${CMAKE_CURRENT_SOURCE_DIR}/src/rp2040/board.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/main.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/usb_descriptors.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/cmsis_dap_device.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/cmsis-dap/SWO.c

  ${CMAKE_CURRENT_SOURCE_DIR}/lib/CMSIS-DAP/Firmware/Source/DAP.c
  ${CMAKE_CURRENT_SOURCE_DIR}/lib/CMSIS-DAP/Firmware/Source/JTAG_DP.c
  ${CMAKE_CURRENT_SOURCE_DIR}/lib/CMSIS-DAP/Firmware/Source/DAP_vendor.c
  ${CMAKE_CURRENT_SOURCE_DIR}/lib/CMSIS-DAP/Firmware/Source/SW_DP.c
)

target_include_directories(akiprobe PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${CMAKE_CURRENT_SOURCE_DIR}/src/rp2040
  ${CMAKE_CURRENT_SOURCE_DIR}/lib/CMSIS-DAP/Firmware/Include/
)

target_compile_definitions(akiprobe PRIVATE
  PICO_RP2040_USB_DEVICE_ENUMERATION_FIX=1
)

target_link_libraries(akiprobe PRIVATE
  cmsis_core
  pico_fix_rp2040_usb_device_enumeration
  pico_stdlib
  pico_unique_id
  tinyusb_device
  pico_bootsel_via_double_reset)

pico_add_extra_outputs(akiprobe)
