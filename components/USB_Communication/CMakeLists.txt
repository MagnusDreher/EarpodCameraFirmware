idf_component_register(SRCS "usb_descriptors.cpp" "audiodata_usb.cpp" "videodata_usb.cpp"
                    INCLUDE_DIRS "."
                    REQUIRES  audioa camera espressif__tinyusb usb_device_uac
                    PRIV_REQUIRES usb_device_uac usb_device_uvc)
idf_component_get_property(tusb_lib espressif__tinyusb COMPONENT_LIB)
target_include_directories(${tusb_lib} PUBLIC "${COMPONENT_DIR}")
