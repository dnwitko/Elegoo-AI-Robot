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

/*
 * SPDX-FileCopyrightText: 2019-2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

// main/esp/detection_responder.cc

#include "esp/detection_responder.h" // Our header

// TFLM header for logging
#include "tensorflow/lite/micro/tflite_bridge/micro_error_reporter.h"

// Project headers & conditional includes for display
#include "esp/esp_main.h" // May contain definitions needed by BSP/display

// --- Conditional Display Support ---
// Requires ESP-BSP library and LVGL configured in project.
// Ensure necessary components are added to main/CMakeLists.txt REQUIRES if enabled.
// Example: esp_bsp, lvgl, esp_lvgl_port
#if defined(CONFIG_ESP_BSP_ENABLED) && CONFIG_ESP_BSP_ENABLED // Use Kconfig flag for display
#define APP_DISPLAY_SUPPORT 1 // Internal flag
#include "esp/camera/image_provider.h" // For image_provider_get_display_buf()
#include "bsp/esp-bsp.h"          // ESP Board Support Package
#include "lvgl.h"                 // LVGL library

// GUI elements (scoped within display support)
namespace {
// Camera image dimensions for display (extrapolated)
const int kDisplayImageWidth = 96 * 2;
const int kDisplayImageHeight = 96 * 2;

// LVGL objects
lv_obj_t* camera_canvas = nullptr;
lv_obj_t* person_indicator = nullptr;
lv_obj_t* label = nullptr;
} // namespace

// Function to create the LVGL GUI elements
static void create_gui(tflite::ErrorReporter* error_reporter) {
    if (camera_canvas) return; // Already created

    error_reporter->Report("Creating GUI...");
    bsp_display_lock(0); // Lock display mutex

    // Canvas for camera image
    camera_canvas = lv_canvas_create(lv_scr_act());
    if (!camera_canvas) { error_reporter->Report("ERROR: Failed to create canvas"); }
    lv_obj_align(camera_canvas, LV_ALIGN_TOP_MID, 0, 0);

    // LED indicator for person detection status
    person_indicator = lv_led_create(lv_scr_act());
     if (!person_indicator) { error_reporter->Report("ERROR: Failed to create LED indicator"); }
    lv_obj_align(person_indicator, LV_ALIGN_BOTTOM_MID, -70, 0);
    lv_led_set_color(person_indicator, lv_palette_main(LV_PALETTE_GREEN));

    // Label next to the LED
    label = lv_label_create(lv_scr_act());
    if (!label) { error_reporter->Report("ERROR: Failed to create label"); }
    lv_label_set_text_static(label, "Person detected");
    lv_obj_align_to(label, person_indicator, LV_ALIGN_OUT_RIGHT_MID, 20, 0);

    bsp_display_unlock(); // Unlock display mutex
    error_reporter->Report("GUI created.");
}

#else // APP_DISPLAY_SUPPORT is 0 or undefined
#define APP_DISPLAY_SUPPORT 0
#endif // CONFIG_ESP_BSP_ENABLED


// Implementation of the response function
void RespondToDetection(tflite::ErrorReporter* error_reporter,
                      float person_score, float no_person_score)
{
    // Convert scores to integer percentage for logging/display
    int person_score_int = static_cast<int>(person_score * 100 + 0.5f);
    int no_person_score_int = static_cast<int>(no_person_score * 100 + 0.5f);
    // Ensure percentages roughly add up (adjust no_person if needed)
    if (person_score_int + no_person_score_int > 105 || person_score_int + no_person_score_int < 95) {
        no_person_score_int = 100 - person_score_int;
    }


    // Log detection scores using the error reporter
    error_reporter->Report("Detection results: Person[%d%%] NoPerson[%d%%]",
                           person_score_int, no_person_score_int);

#if APP_DISPLAY_SUPPORT
    // Ensure GUI is created if display is supported
    if (!camera_canvas) {
        // Initialize BSP display first if not already done (depends on project structure)
        // This might be better placed in setup()
        if (!bsp_display_is_initialized()) {
             bsp_display_start(); // Should handle initialization
             bsp_display_backlight_on();
        }
        create_gui(error_reporter);
    }

    // Get the display buffer (populated by image_provider)
    uint16_t* display_buf_ptr = (uint16_t*) image_provider_get_display_buf();

    // Update GUI elements
    if (display_buf_ptr && camera_canvas && person_indicator) {
        bsp_display_lock(0); // Lock display mutex

        // Update LED based on threshold
        const int person_threshold_percent = 60;
        if (person_score_int < person_threshold_percent) {
            lv_led_off(person_indicator);
        } else {
            lv_led_on(person_indicator);
        }

        // Update canvas with the latest camera image
        lv_canvas_set_buffer(camera_canvas, display_buf_ptr,
                             kDisplayImageWidth, kDisplayImageHeight,
                             LV_IMG_CF_TRUE_COLOR); // Assumes RGB565 from provider

        bsp_display_unlock(); // Unlock display mutex
    } else {
         if (!display_buf_ptr) error_reporter->Report("WARN: Display buffer is null");
         if (!camera_canvas) error_reporter->Report("WARN: Camera canvas not initialized");
         if (!person_indicator) error_reporter->Report("WARN: Person indicator not initialized");
    }
#endif // APP_DISPLAY_SUPPORT
}