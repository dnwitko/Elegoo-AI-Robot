/* Copyright 2019 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

// main/esp/camera/app_camera_esp.c

#include "esp/camera/app_camera_esp.h" // Include own header
#include "sdkconfig.h"              // To check Kconfig options
#include "esp_log.h"
#include "driver/gpio.h"            // For manual pin config if needed
#include "esp_camera.h"             // Main camera driver header
#include "elegoo_camera_pins.h"

// Include project config header for APP_DISPLAY_SUPPORT etc. (Optional, could use Kconfig directly)
// #include "esp/esp_main.h"

// --- Conditional Includes/Configuration ---
// Use the standard Kconfig option for enabling BSP
#if defined(CONFIG_ESP_BSP_ENABLED) && CONFIG_ESP_BSP_ENABLED
#include "bsp/esp-bsp.h" // Include BSP header if enabled
#define APP_USES_BSP 1
#else
#define APP_USES_BSP 0
#endif

static const char* TAG = "APP_CAMERA";

int app_camera_init() {
#if ESP_CAMERA_SUPPORTED
    ESP_LOGI(TAG, "Initializing camera...");

#if CONFIG_CAMERA_MODULE_ESP_EYE || CONFIG_CAMERA_MODULE_ESP32_CAM_BOARD
    // Workaround for boards reusing JTAG pins for camera
    ESP_LOGD(TAG, "Applying JTAG pin workaround for ESP_EYE/ESP32_CAM...");
    gpio_config_t conf;
    conf.mode = GPIO_MODE_INPUT;
    conf.pull_up_en = GPIO_PULLUP_ENABLE;
    conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    conf.intr_type = GPIO_INTR_DISABLE;
    conf.pin_bit_mask = 1LL << 13;
    gpio_config(&conf);
    conf.pin_bit_mask = 1LL << 14;
    gpio_config(&conf);
    ESP_LOGD(TAG, "JTAG pins configured as inputs.");
#endif // CONFIG_CAMERA_MODULE_ESP_EYE || CONFIG_CAMERA_MODULE_ESP32_CAM_BOARD

    camera_config_t config;

// >>> MODIFICATION: Use standardized BSP check <<<
#if APP_USES_BSP
    ESP_LOGI(TAG, "Using ESP-BSP default camera configuration.");
    // Initialize I2C for BSP communication with camera module
    bsp_i2c_init();
    // Get the default config from BSP (should include correct pins, freq, etc. for S3-EYE)
    config = BSP_CAMERA_DEFAULT_CONFIG;
#else // APP_USES_BSP == 0
    ESP_LOGI(TAG, "Using manual camera pin configuration.");
    // --- Manual Configuration (Verify these defines match your board!) ---
    // These defines (CAMERA_PIN_*, XCLK_FREQ_HZ) must be set correctly,
    // typically via Kconfig (menuconfig) or a board-specific header.
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = CAMERA_PIN_D0;
    config.pin_d1 = CAMERA_PIN_D1;
    config.pin_d2 = CAMERA_PIN_D2;
    config.pin_d3 = CAMERA_PIN_D3;
    config.pin_d4 = CAMERA_PIN_D4;
    config.pin_d5 = CAMERA_PIN_D5;
    config.pin_d6 = CAMERA_PIN_D6;
    config.pin_d7 = CAMERA_PIN_D7;
    config.pin_xclk = CAMERA_PIN_XCLK;
    config.pin_pclk = CAMERA_PIN_PCLK;
    config.pin_vsync = CAMERA_PIN_VSYNC;
    config.pin_href = CAMERA_PIN_HREF;
    config.pin_sscb_sda = CAMERA_PIN_SIOD; // I2C SDA
    config.pin_sscb_scl = CAMERA_PIN_SIOC; // I2C SCL
    config.pin_pwdn = CAMERA_PIN_PWDN;
    config.pin_reset = CAMERA_PIN_RESET;
    // config.xclk_freq_hz = XCLK_FREQ_HZ; // e.g., 16500000 (16.5MHz) or 20000000 (20MHz)
    config.jpeg_quality = 10; // Not used for raw formats
    config.fb_count = 2;      // Use 2 frame buffers for smoother capture
    config.fb_location = CAMERA_FB_IN_PSRAM; // Use PSRAM for frame buffers
#endif // APP_USES_BSP

    // --- Application Specific Configuration ---
    // These MUST be set correctly based on model requirements and display usage.
    // Check Kconfig (menuconfig -> Component config -> ESP32 Camera) or project headers.
    config.pixel_format = CAMERA_PIXEL_FORMAT; // Should be PIXFORMAT_GRAYSCALE or PIXFORMAT_RGB565
    config.frame_size = CAMERA_FRAME_SIZE;     // Should be FRAMESIZE_96X96

    ESP_LOGI(TAG, "Camera pixel format: %d (%s)", config.pixel_format,
             (config.pixel_format == PIXFORMAT_GRAYSCALE) ? "GRAYSCALE" :
             (config.pixel_format == PIXFORMAT_RGB565) ? "RGB565" : "Other");
    ESP_LOGI(TAG, "Camera frame size: %d (Expected 96x96)", config.frame_size);

    if (config.frame_size != FRAMESIZE_96X96) {
        ESP_LOGW(TAG, "*** WARNING: Camera frame size is NOT 96x96! Model expects 96x96. ***");
    }

    // Initialize camera driver
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Camera init failed with error 0x%x (%s)", err, esp_err_to_name(err));
        return -1;
    }
    ESP_LOGI(TAG, "esp_camera_init successful.");

    // Apply sensor-specific settings
    sensor_t* s = esp_camera_sensor_get();
    if (s == NULL) {
        ESP_LOGE(TAG, "Failed to get camera sensor instance.");
        esp_camera_deinit(); // Clean up
        return -1;
    }

    // Common settings adjustments
    s->set_vflip(s, 1); // Flip camera image vertically (adjust as needed)
    ESP_LOGD(TAG, "Vertical flip set.");

    // Sensor specific adjustments (Example for OV3660)
    if (s->id.PID == OV3660_PID) {
        ESP_LOGD(TAG, "Applying OV3660 specific settings.");
        s->set_brightness(s, 1);  // Example: slightly brighter
        s->set_saturation(s, -2); // Example: slightly less saturated
    } else {
        ESP_LOGD(TAG, "Sensor PID: 0x%x", s->id.PID);
    }

    ESP_LOGI(TAG, "Camera initialization complete.");
    return 0; // Success

#else // ESP_CAMERA_SUPPORTED
    ESP_LOGE(TAG, "Camera is not supported for this device!");
    return -1;
#endif // ESP_CAMERA_SUPPORTED
}