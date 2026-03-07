idf_component_register(
    SRCS
        "audiodata_usb.cpp"
        "videodata_usb.cpp"
    INCLUDE_DIRS
        "."
    REQUIRES audioa camera espressif__tinyusb espressif__usb_device_uac espressif__usb_device_uvc 
)
