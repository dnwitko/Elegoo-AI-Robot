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

// main/esp/camera/image_provider.h

#ifndef APP_ESP_CAMERA_IMAGE_PROVIDER_H_ // <<< Updated include guard
#define APP_ESP_CAMERA_IMAGE_PROVIDER_H_

#include "tensorflow/lite/c/common.h"          // For TfLiteStatus
#include "tensorflow/lite/micro/tflite_bridge/micro_error_reporter.h" // <<< Added for ErrorReporter type

// Provides an interface to get image data from a camera sensor.

// Returns a buffer containing the latest camera image, potentially suitable for display.
// The format (e.g., RGB565, JPEG) depends on the implementation in image_provider.cc
// Returns nullptr if display buffer is not available or not supported.
void* image_provider_get_display_buf();

// Initializes the camera sensor.
// Should be called once during setup.
// Returns kTfLiteOk on success, kTfLiteError on failure.
// Passes the error reporter for logging.
TfLiteStatus InitCamera(tflite::ErrorReporter* error_reporter); // <<< Added error_reporter parameter

// Get image data from the camera sensor.
// Populates the 'image_data' buffer with the captured image, formatted
// according to the model's requirements (e.g., grayscale, specific dimensions).
// Arguments:
//   error_reporter: For logging any issues during capture/processing.
//   image_width: Target width required by the model (e.g., kPersonNumCols).
//   image_height: Target height required by the model (e.g., kPersonNumRows).
//   channels: Target number of channels (e.g., kPersonNumChannels, likely 1).
//   image_data: Destination buffer (type int8_t assumed based on original code).
// Returns kTfLiteOk on success, kTfLiteError on failure.
TfLiteStatus GetImage(tflite::ErrorReporter* error_reporter, // <<< Added error_reporter parameter
                      int image_width, int image_height, int channels,
                      int8_t* image_data); // <<< Assumes model input buffer is int8_t

#endif // APP_ESP_CAMERA_IMAGE_PROVIDER_H_