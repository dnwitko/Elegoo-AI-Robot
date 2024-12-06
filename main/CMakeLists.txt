<<<<<<< HEAD

#
# Main component of TF Micro project 'person_detection'.
#

idf_component_register(
    SRCS
        "detection_responder.cc"
        "image_provider.cc"
        "main.cc"
        "main_functions.cc"
        "model_settings.cc"
        "person_detect_model_data.cc"
        "app_camera_esp.c"
        "esp_cli.c"
        "USBHostSerial.cpp" # MODIFICATION: Added USBHostSerial so the library compiles

    PRIV_REQUIRES console static_images spi_flash
    INCLUDE_DIRS "")
=======
idf_component_register(SRCS "main.cpp" "USBHostSerial.cpp"
                    INCLUDE_DIRS "../USBHostSerial")
>>>>>>> 83705188de6e48fbca2c34160867622cf2a0a7f5
