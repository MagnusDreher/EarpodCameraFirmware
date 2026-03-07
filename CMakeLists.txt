cmake_minimum_required(VERSION 3.5)


set(EXTRA_COMPONENT_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/components")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(EarpodCameraFirmware)

# Workaround for GCC 14.2.0 ICE (segfault in IRA pass) on esp_lcd_panel_rgb.c
# set_source_files_properties only works within the same directory scope,
# so we must use target_compile_options on the component target instead.
target_compile_options(__idf_esp_lcd PRIVATE -O0)


target_include_directories(
    __idf_espressif__usb_device_uac
    PRIVATE
    ${CMAKE_SOURCE_DIR}/components/usb_communication
)

target_include_directories(
    __idf_espressif__usb_device_uvc
    PRIVATE
    ${CMAKE_SOURCE_DIR}/components/usb_communication
)