cmake_minimum_required(VERSION 3.12)

if (NOT PICO_SDK_PATH)
  set(PICO_SDK_PATH "${CMAKE_CURRENT_LIST_DIR}/lib/pico-sdk")
endif ()
set(PICO_BOARD pico)
set(PICO_COPY_TO_RAM 1)
#set(PICO_TINYUSB_PATH "${CMAKE_CURRENT_LIST_DIR}/lib/tinyusb")

include(${PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(akiprobe)

pico_sdk_init()

add_executable(akiprobe
  ${CMAKE_CURRENT_SOURCE_DIR}/src/rp2040/board.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/main.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/usb_descriptors.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/cmsis_dap_device.c

  ${CMAKE_CURRENT_SOURCE_DIR}/lib/CMSIS_5/CMSIS/DAP/Firmware/Source/DAP.c
  ${CMAKE_CURRENT_SOURCE_DIR}/lib/CMSIS_5/CMSIS/DAP/Firmware/Source/JTAG_DP.c
  ${CMAKE_CURRENT_SOURCE_DIR}/lib/CMSIS_5/CMSIS/DAP/Firmware/Source/DAP_vendor.c
  ${CMAKE_CURRENT_SOURCE_DIR}/lib/CMSIS_5/CMSIS/DAP/Firmware/Source/SWO.c
  ${CMAKE_CURRENT_SOURCE_DIR}/lib/CMSIS_5/CMSIS/DAP/Firmware/Source/SW_DP.c
)

target_include_directories(akiprobe PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${CMAKE_CURRENT_SOURCE_DIR}/src/rp2040
  ${CMAKE_CURRENT_SOURCE_DIR}/lib/CMSIS_5/CMSIS/DAP/Firmware/Include/
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
  pico_bootsel_via_double_reset
  pico_multicore
)

pico_add_extra_outputs(akiprobe)
