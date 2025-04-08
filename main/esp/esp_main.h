// Copyright 2020-2021 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// main/esp/esp_main.h

#ifndef APP_ESP_MAIN_H_ // <<< Added include guard
#define APP_ESP_MAIN_H_

#include "sdkconfig.h" // Include project specific sdkconfig defines

// --- Build Time Configuration Defines ---

// Uncomment this to build *only* for CLI inference using static images
// (Disables camera/audio init and the main loop() execution)
// #define APP_CLI_ONLY_INFERENCE 1

// Uncomment this to enable CPU stats collection in run_inference()
// #define APP_COLLECT_CPU_STATS 1 // Enabled via Kconfig is often better

// --- Conditional Compilation Logic ---

// Set APP_DISPLAY_SUPPORT based on ESP-BSP Kconfig option
// Use this consistently across files (detection_responder.cc, image_provider.cc)
#if defined(CONFIG_ESP_BSP_ENABLED) && CONFIG_ESP_BSP_ENABLED
  #define APP_DISPLAY_SUPPORT 1
#else
  #define APP_DISPLAY_SUPPORT 0
#endif


// --- Extern "C" Declarations ---
// For functions implemented in C++ but called from C (like esp_cli.c)

#ifdef __cplusplus
extern "C" {
#endif

// Declares the function that runs inference for the command-line interface.
// Defined in main/core/main_functions.cc
void run_inference(void *ptr_image_data); // Changed ptr -> ptr_image_data for clarity

#ifdef __cplusplus
}
#endif

#endif // APP_ESP_MAIN_H_
