// main/esp/camera/image_provider.cc

#include "esp/camera/image_provider.h" // Our header

// Standard/ESP-IDF Headers
#include <cstdlib>
#include <cstring>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_system.h"
#include <esp_heap_caps.h> // For heap_caps_malloc

// Camera specific headers
#include "esp/camera/app_camera_esp.h" // Defines app_camera_init()
#include "esp_camera.h"             // Defines camera_fb_t, esp_camera_fb_get/return

// Project headers
#include "models/model_settings.h" // For kPersonNumRows, kPersonNumCols constants
#include "esp/esp_main.h"          // For APP_CLI_ONLY_INFERENCE, APP_DISPLAY_SUPPORT defines

// TFLM headers
#include "tensorflow/lite/micro/tflite_bridge/micro_error_reporter.h" // For ErrorReporter type

// Tag for logging
static const char* TAG = "IMAGE_PROVIDER";

// --- Globals ---
// Display buffer (allocated in PSRAM if available)
#if APP_DISPLAY_SUPPORT // Use the standardized define from esp_main.h
static uint16_t* display_buf = nullptr;
#endif

// --- Function Implementations ---

// Initialize the camera by calling the platform-specific init function
TfLiteStatus InitCamera(tflite::ErrorReporter* error_reporter) {
// Use the standardized define from esp_main.h
#if defined(APP_CLI_ONLY_INFERENCE) && APP_CLI_ONLY_INFERENCE == 1
    error_reporter->Report("CLI_ONLY_INFERENCE enabled, skipping camera init.");
    return kTfLiteOk;
#endif // APP_CLI_ONLY_INFERENCE

// Initialize display buffer if display support is enabled
#if APP_DISPLAY_SUPPORT // Use the standardized define from esp_main.h
    if (display_buf == NULL) {
        // Allocate display buffer, attempting PSRAM first
        ESP_LOGI(TAG, "Allocating display buffer (192x192xRGB565)...");
        display_buf = (uint16_t*) heap_caps_malloc(
            kPersonNumRows * 2 * kPersonNumCols * 2 * sizeof(uint16_t),
            MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);

        if (display_buf == NULL) {
             ESP_LOGW(TAG, "Could not allocate display buffer in SPIRAM, trying internal RAM...");
             display_buf = (uint16_t*) malloc(
                 kPersonNumRows * 2 * kPersonNumCols * 2 * sizeof(uint16_t));
        }
        if (display_buf == NULL) {
             error_reporter->Report("ERROR: Couldn't allocate display buffer");
             return kTfLiteError;
        }
        ESP_LOGI(TAG, "Display buffer allocated at 0x%p.", display_buf);
    }
#endif // APP_DISPLAY_SUPPORT

#if ESP_CAMERA_SUPPORTED
    // >>> MODIFICATION START <<<
    // Call the initialization function defined in app_camera_esp.c
    ESP_LOGI(TAG, "Calling app_camera_init()...");
    int ret = app_camera_init();
    if (ret != 0) {
        // app_camera_init should log specific errors via ESP_LOGE
        error_reporter->Report("ERROR: Camera init failed (app_camera_init returned %d). Check logs.", ret);
        return kTfLiteError;
    }
    error_reporter->Report("Camera Initialized via app_camera_init().");
    // >>> MODIFICATION END <<<
    return kTfLiteOk;
#else // !ESP_CAMERA_SUPPORTED
    error_reporter->Report("ERROR: Camera not supported on this platform!");
    return kTfLiteError;
#endif // ESP_CAMERA_SUPPORTED
}


// Get the display buffer pointer (if enabled)
void* image_provider_get_display_buf() {
#if APP_DISPLAY_SUPPORT // Use the standardized define from esp_main.h
    return (void*) display_buf;
#else
    return nullptr; // Display buffer not available
#endif
}


// Get an image from the camera and prepare it for the model
TfLiteStatus GetImage(tflite::ErrorReporter* error_reporter,
                      int image_width, int image_height, int channels,
                      int8_t* image_data // Destination buffer for model input
                     )
{
#if ESP_CAMERA_SUPPORTED
    // Capture frame from the camera
    camera_fb_t* fb = esp_camera_fb_get();
    if (!fb) {
        error_reporter->Report("ERROR: Camera frame capture failed!");
        return kTfLiteError;
    }

    // --- Process frame based on whether display is supported ---
    // This implicitly determines if the frame format is RGB565 or Grayscale,
    // assuming app_camera_init configures the camera accordingly via CAMERA_PIXEL_FORMAT define.

#if APP_DISPLAY_SUPPORT // Use the standardized define from esp_main.h
    // Check if frame buffer format is RGB565 (expected if display enabled)
    if (fb->format != PIXFORMAT_RGB565) {
         error_reporter->Report("ERROR: Expected RGB565 format for display, got %d", fb->format);
         esp_camera_fb_return(fb);
         return kTfLiteError;
    }
    // Check if frame dimensions match expectations (use constants as original code did)
     if (fb->width != kPersonNumCols || fb->height != kPersonNumRows) {
         error_reporter->Report("ERROR: Frame dimensions %dx%d mismatch expected %dx%d",
                                fb->width, fb->height, kPersonNumCols, kPersonNumRows);
         esp_camera_fb_return(fb);
         return kTfLiteError;
    }

    // Process RGB565 data: Convert to grayscale, quantize to int8, and populate display buffer
    uint16_t* frame_buf_rgb565 = (uint16_t*)fb->buf;
    for (int r = 0; r < kPersonNumRows; ++r) {
        for (int c = 0; c < kPersonNumCols; ++c) {
            int pixel_index = r * kPersonNumCols + c;
            uint16_t pixel = frame_buf_rgb565[pixel_index];

            // --- Convert RGB565 to Grayscale and Quantize to Int8 ---
            uint8_t hb = pixel & 0xFF;
            uint8_t lb = pixel >> 8;
            uint8_t rd = (lb & 0x1F) << 3; // Red
            uint8_t gr = ((hb & 0x07) << 5) | ((lb & 0xE0) >> 3); // Green
            uint8_t bl = (hb & 0xF8); // Blue
            int8_t grey_pixel = static_cast<int8_t>(
                ((305 * rd + 600 * gr + 119 * bl) >> 10) - 128);
            image_data[pixel_index] = grey_pixel;

            // --- Populate Display Buffer (Extrapolation) ---
            if (display_buf != nullptr) {
                int display_base_col = 2 * c;
                int display_base_row = 2 * r;
                int display_width = kPersonNumCols * 2;
                display_buf[display_base_row * display_width + display_base_col] = pixel;
                display_buf[display_base_row * display_width + display_base_col + 1] = pixel;
                display_buf[(display_base_row + 1) * display_width + display_base_col] = pixel;
                display_buf[(display_base_row + 1) * display_width + display_base_col + 1] = pixel;
            }
        }
    }

#else // APP_DISPLAY_SUPPORT is *not* enabled
    // Check if frame buffer format is Grayscale (expected if display disabled)
    if (fb->format != PIXFORMAT_GRAYSCALE) {
         error_reporter->Report("ERROR: Expected GRAYSCALE format, got %d", fb->format);
         esp_camera_fb_return(fb);
         return kTfLiteError;
    }
     // Check if frame dimensions match expectations (use constants as original code did)
     if (fb->width != kPersonNumCols || fb->height != kPersonNumRows) {
         error_reporter->Report("ERROR: Frame dimensions %dx%d mismatch expected %dx%d",
                                fb->width, fb->height, kPersonNumCols, kPersonNumRows);
         esp_camera_fb_return(fb);
         return kTfLiteError;
    }

    // Process Grayscale data: Quantize uint8 grayscale to int8
    uint8_t* frame_buf_gray = (uint8_t*)fb->buf;
    int pixel_count = kPersonNumCols * kPersonNumRows;
    for (int i = 0; i < pixel_count; ++i) {
        // Map [0, 255] uint8 grayscale to [-128, 127] int8
        image_data[i] = static_cast<int8_t>(frame_buf_gray[i] ^ 0x80);
    }
    // ESP_LOGD(TAG, "Image Captured (Grayscale)");

#endif // APP_DISPLAY_SUPPORT

    // Return the frame buffer to the camera driver
    esp_camera_fb_return(fb);
    return kTfLiteOk;

#else // !ESP_CAMERA_SUPPORTED
    error_reporter->Report("ERROR: Camera not supported on this platform!");
    return kTfLiteError;
#endif // ESP_CAMERA_SUPPORTED
}