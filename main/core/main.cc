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

// main/core/main.cc

// Core application logic functions (setup, loop)
#include "core/main_functions.h"

// ESP-IDF headers
#include "esp_log.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Project configuration and C function declarations
#include "esp/esp_main.h" // Contains APP_CLI_ONLY_INFERENCE define

// Conditional include for Command Line Interface
#if defined(APP_CLI_ONLY_INFERENCE) && APP_CLI_ONLY_INFERENCE == 1
#include "esp/esp_cli.h" // Assumes header is in main/esp/
#endif

// Tag for logging (optional)
// static const char* TAG = "MAIN_CORE";

// Main task function that runs the application logic
void tf_main_task(void* pvParameters) {
    // Call the main setup function (initializes models, peripherals, etc.)
    setup();

#if defined(APP_CLI_ONLY_INFERENCE) && APP_CLI_ONLY_INFERENCE == 1
    // If configured for CLI only, start the CLI and wait
    esp_cli_start();
    vTaskDelay(portMAX_DELAY); // Keep task alive but idle
#else
    // Otherwise, enter the main processing loop
    while (true) {
        loop();
        // loop() should contain its own vTaskDelay or yield mechanism
    }
#endif
    // Should not reach here in normal operation
    vTaskDelete(NULL);
}

// Entry point called by ESP-IDF startup code
extern "C" void app_main() {
    // Create the main application task
    // Stack size (8KB typical for TFLM + drivers), Priority (adjust as needed)
    xTaskCreate(tf_main_task,      // Function to execute
                "tf_main_task",    // Task name
                8 * 1024,          // Stack size in bytes (increased to 8KB)
                NULL,              // Parameter pvParameters
                5,                 // Task priority (adjust as needed, e.g., 5)
                NULL);             // Task handle (optional)

    // The app_main task can now be deleted or suspend itself,
    // as the main logic runs in tf_main_task. Deleting is common.
    vTaskDelete(NULL);
}