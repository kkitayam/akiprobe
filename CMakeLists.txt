cmake_minimum_required(VERSION 3.12)

if (NOT PICO_SDK_PATH)
  set(PICO_SDK_PATH "${CMAKE_CURRENT_LIST_DIR}/lib/pico-sdk")
endif ()
set(PICO_BOARD pico)
set(PICO_COPY_TO_RAM 1)

set(TINYUSB_PATH "${CMAKE_CURRENT_LIST_DIR}/lib/tinyusb")


include(${PICO_SDK_PATH}/external/pico_sdk_import.cmake)


add_library(tinyusb_common_base INTERFACE)

target_sources(tinyusb_common_base INTERFACE
  ${TINYUSB_PATH}/src/tusb.c
  ${TINYUSB_PATH}/src/common/tusb_fifo.c
)
target_include_directories(tinyusb_common_base INTERFACE
  ${TINYUSB_PATH}/src
  ${TINYUSB_PATH}/src/common
)
target_link_libraries(tinyusb_common_base INTERFACE
  hardware_structs
  hardware_irq
  hardware_resets
  pico_sync
)

set(TINYUSB_DEBUG_LEVEL 0)
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  message("Compiling TinyUSB with CFG_TUSB_DEBUG=1")
  set(TINYUSB_DEBUG_LEVEL 1)
endif()

target_compile_definitions(tinyusb_common_base INTERFACE
  CFG_TUSB_MCU=OPT_MCU_RP2040
  CFG_TUSB_OS=OPT_OS_PICO
  CFG_TUSB_DEBUG=${TINYUSB_DEBUG_LEVEL}
)

add_library(tinyusb_device_base INTERFACE)
target_sources(tinyusb_device_base INTERFACE
  ${TINYUSB_PATH}/src/portable/raspberrypi/rp2040/dcd_rp2040.c
  ${TINYUSB_PATH}/src/portable/raspberrypi/rp2040/rp2040_usb.c
  ${TINYUSB_PATH}/src/device/usbd.c
  ${TINYUSB_PATH}/src/device/usbd_control.c
  ${TINYUSB_PATH}/src/class/audio/audio_device.c
  ${TINYUSB_PATH}/src/class/cdc/cdc_device.c
  ${TINYUSB_PATH}/src/class/dfu/dfu_device.c
  ${TINYUSB_PATH}/src/class/dfu/dfu_rt_device.c
  ${TINYUSB_PATH}/src/class/hid/hid_device.c
  ${TINYUSB_PATH}/src/class/midi/midi_device.c
  ${TINYUSB_PATH}/src/class/msc/msc_device.c
  ${TINYUSB_PATH}/src/class/net/ecm_rndis_device.c
  ${TINYUSB_PATH}/src/class/net/ncm_device.c
  ${TINYUSB_PATH}/src/class/usbtmc/usbtmc_device.c
  ${TINYUSB_PATH}/src/class/video/video_device.c
)


project(akiprobe)

pico_sdk_init()

add_executable(akiprobe
  ${CMAKE_CURRENT_SOURCE_DIR}/src/rp2040/board.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/main.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/usb_descriptors.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/vendor_device.c

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
  tinyusb_common_base
  tinyusb_device_base
  pico_bootsel_via_double_reset)

pico_add_extra_outputs(akiprobe)
