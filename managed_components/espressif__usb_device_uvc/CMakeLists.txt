idf_component_register(SRCS usb_device_uvc.c
                    INCLUDE_DIRS "include"
                    REQUIRES usb esp_timer)

idf_component_get_property(tusb_lib espressif__tinyusb COMPONENT_LIB)

idf_build_get_property(project_dir PROJECT_DIR)
target_include_directories(${tusb_lib} BEFORE PUBLIC "${project_dir}/components/USB_Communication")

target_include_directories(${tusb_lib} PUBLIC "${COMPONENT_DIR}/tusb")
target_sources(${tusb_lib} PUBLIC "${COMPONENT_DIR}/tusb/usb_descriptors.c")

idf_component_get_property(uac_dir espressif__usb_device_uac COMPONENT_DIR)
target_include_directories(${tusb_lib} PUBLIC "${uac_dir}/tusb_uac")

include(package_manager)
cu_pkg_define_version(${CMAKE_CURRENT_LIST_DIR})
