# CMakeLists.txt (main component) - Cleaned up for ESP-IDF v5+

# Define Source Files (Ensure paths are correct relative to 'main')
set(SOURCES
    core/main.cc
    core/main_functions.cc
    core/model_init.cc
    esp/esp_cli.c
    esp/camera/app_camera_esp.c
    esp/camera/image_provider.cc
    esp/detection_responder.cc
    esp/audio/audio_provider.cc
    esp/audio/audio_responder.cc
    models/face_model/person_detect_model_data.cc
    models/model_settings.cc
    USBserial/USBHostSerial.cpp
)

# Define Include Directories FOR THIS COMPONENT ONLY
# (Do NOT add paths to managed_components here)
set(INCLUDE_DIRS
    .             # Include main component root (for headers in main/)
    core          # Core logic headers
    esp           # ESP-specific headers (like detection_responder.h)
    esp/camera    # Camera headers (like image_provider.h)
    esp/audio     # Audio headers (like audio_provider.h)
    models        # Models root headers (like model_settings.h)
    models/face_model # Only if face_model dir has .h files needed by others
    models/audio_model # Only if audio_model dir has .h files needed by others
    USBserial     # USB Serial headers
)

# Define Text Files to Embed
set(EMBED_TXTFILES
    models/audio_model/labels.txt
    models/audio_model/audio_model.tflite
)

# Register the component
idf_component_register(
    SRCS "${SOURCES}"
    INCLUDE_DIRS "${INCLUDE_DIRS}"
    EMBED_FILES "${EMBED_TXTFILES}"
    # List ALL components whose headers/functions are DIRECTLY used by 'main' sources
    REQUIRES
        # --- Core ESP-IDF ---
        driver          # For GPIO, I2C, I2S etc.
        esp_timer       # For timing
        log             # For ESP_LOGx macros
        nvs_flash       # If used
        spi_flash       # If used directly or by NVS
        esp_hw_support  # Often needed indirectly
        esp_system      # For system functions
        freertos        # If using FreeRTOS APIs directly
        hal             # Hardware Abstraction Layer

        # --- Managed Components (from idf_component.yml) ---
        esp32-camera      # Camera component
        esp-tflite-micro  # TensorFlow Lite Micro component
        esp_tinyusb       # TinyUSB component
        tinyusb           # TinyUSB library base (often needed with esp_tinyusb)

        # --- Other Project Components ---
        console           # For esp_cli.c
        static_images     # If used

        # --- USB Specific (Check if USBSerial.cpp needs these directly) ---
        usb               # Core USB driver
        # usb_host_cdc_acm # Add if directly used
        # usb_host_vcp     # Add if directly used
        # Add other usb_host_* if needed
)