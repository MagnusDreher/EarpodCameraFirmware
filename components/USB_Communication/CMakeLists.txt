idf_component_register(SRCS "audiodata_usb.cpp" "videodata_usb.cpp"
                    INCLUDE_DIRS "."
                    REQUIRES esp_tinyusb audioa camera)