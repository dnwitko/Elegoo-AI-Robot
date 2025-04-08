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

// main/core/main_functions.cc

#include "core/main_functions.h" // Our function declarations

// Include project headers using relative paths from 'main' directory
#include "core/model_init.h"           // For model initialization and accessors
#include "models/model_settings.h"     // For constants like kPersonIndex, kAudioCategoryCount
#include "esp/esp_main.h"              // For InitCamera, InitAudio (if needed there)
#include "esp/camera/image_provider.h" // For GetImage
#include "esp/audio/audio_provider.h"  // For GetAudioSamples
#include "esp/detection_responder.h" // For RespondToDetection
#include "esp/audio/audio_responder.h"     // For RespondToAudioCommand

// Include TFLM, RTOS, ESP-IDF headers
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <esp_timer.h> // For timing inference
#include <esp_log.h>

// Include and setup USB Host Serial library
#include "USBserial/USBHostSerial.h"
USBHostSerial usbSerial;

// Tag for logging
static const char* TAG = "MAIN_FUNC";

// --- Globals (minimal) ---
// We get interpreters and tensors via model_init accessors now

// --- Setup Function ---
void setup() {
    ESP_LOGI(TAG, "Starting Setup...");;

    // 1. Initialize Models (Error Reporter is now ready for model_init to use)
    if (!model_init()) {
        MicroPrintf("ERROR: Model initialization failed!");
        // Consider halting or indicating error state permanently
        while(1) { vTaskDelay(1000); }
    }
    ESP_LOGI(TAG, "Models Initialized.");

    // 2. Initialize USB Serial for Robot Control
    // stopbits: 0: 1 stopbit, 1: 1.5 stopbits, 2: 2 stopbits
    // parity: 0: None, 1: Odd, 2: Even, 3: Mark, 4: Space
    // databits: 8
    usbSerial.begin(9600, 0, 0, 8);
    ESP_LOGI(TAG, "USB Serial Initialized (9600 baud).");
    // Note: This might make the ESP32-S3's native USB CDC unusable for logging/flashing
    // depending on board connections. Use UART for logging if needed.

    // 3. Initialize Camera Input
    // Assumes InitCamera() is defined elsewhere (e.g., in esp_main.cc or image_provider.cc)
    // and handles camera setup.
    #ifndef CLI_ONLY_INFERENCE
    if (InitCamera(get_error_reporter()) != kTfLiteOk) { // Pass error reporter
         MicroPrintf("ERROR: Camera initialization failed!");
         // Handle error appropriately
    } else {
        ESP_LOGI(TAG, "Camera Initialized.");
    }
    #endif // CLI_ONLY_INFERENCE

    // 4. Initialize Audio Input
    // Assumes InitAudio() is defined elsewhere (e.g., in esp_main.cc or audio_provider.cc)
    // and handles microphone/I2S setup.
    #ifndef CLI_ONLY_INFERENCE
    if (InitAudio(get_error_reporter()) != kTfLiteOk) { // Pass error reporter
        MicroPrintf("ERROR: Audio initialization failed!");
        // Handle error appropriately
    } else {
        ESP_LOGI(TAG, "Audio Initialized.");
    }
    #endif // CLI_ONLY_INFERENCE

    ESP_LOGI(TAG, "Setup Complete.");
}

// --- Loop Function ---
void loop() {
    #ifndef CLI_ONLY_INFERENCE // Only run loop logic if not in CLI mode

    // --- 1. Person Detection ---
    tflite::MicroInterpreter* person_interpreter = get_person_detection_interpreter();
    TfLiteTensor* person_input = get_person_detection_input_tensor();
    tflite::ErrorReporter* error_reporter = get_error_reporter();
    bool person_detected = false;

    if (!person_interpreter || !person_input || !error_reporter) {
        MicroPrintf("ERROR: Person detection TFLM objects not initialized!");
        vTaskDelay(1000);
        return;
    }

    // Get image from provider (fills person_input->data)
    if (kTfLiteOk != GetImage(error_reporter, kPersonNumCols, kPersonNumRows, kPersonNumChannels, person_input->data.int8)) {
        MicroPrintf("WARN: Image capture failed.");
        // Decide if we should continue or wait/retry
    } else {
        // Run person detection inference
        int64_t start_time = esp_timer_get_time();
        if (kTfLiteOk != person_interpreter->Invoke()) {
            MicroPrintf("WARN: Person Invoke failed.");
        }
        int64_t end_time = esp_timer_get_time();
        long long person_inference_time = (end_time - start_time) / 1000;
        // ESP_LOGI(TAG, "Person Inference Time: %lld ms", person_inference_time); // Optional timing log

        // Process person detection results
        TfLiteTensor* person_output = person_interpreter->output(0);
        int8_t person_score_quant = person_output->data.uint8[kPersonIndex];
        int8_t no_person_score_quant = person_output->data.uint8[kNotAPersonIndex];

        // Dequantize scores
        float person_scale = person_output->params.scale;
        int person_zero_point = person_output->params.zero_point;
        float person_score_f = (person_score_quant - person_zero_point) * person_scale;
        float no_person_score_f = (no_person_score_quant - person_zero_point) * person_scale;

        // Log detection result (using detection_responder)
        RespondToDetection(error_reporter, person_score_f, no_person_score_f);

        // --- Control based on Person Detection ---
        // Original behavior: Move forward if person detected with high confidence
        bool person_detected = (person_score_f > 0.7); // Use a threshold
        if (person_detected) {
            // Maybe don't send command immediately, let audio commands override?
            // ESP_LOGI(TAG, "Person detected, potentially moving forward...");
        }
    } // End of successful image capture block

    // --- 2. Audio Classification ---
    tflite::MicroInterpreter* audio_interpreter = get_audio_classifier_interpreter();
    TfLiteTensor* audio_input = get_audio_classifier_input_tensor();

    if (!audio_interpreter || !audio_input || !error_reporter) {
        MicroPrintf("ERROR: Audio classification TFLM objects not initialized!");
        vTaskDelay(1000);
        return;
    }

    // Get audio samples (fills audio_input->data)
    // The audio provider needs to handle buffering and slicing correctly.
    int samples_read = 0;
    if (kTfLiteOk != GetAudioSamples(error_reporter,
                                     kAudioInputSize, // How many samples the model expects
                                     &samples_read,   // How many samples were actually read/available
                                     audio_input->data.int8)) // Destination buffer (model input)
    {
        MicroPrintf("WARN: Audio capture failed.");
        // Decide if we should continue or wait/retry
    } else {
        // Only run inference if we got the expected number of samples
        // (or handle partial input if your model/provider supports it)
        if (samples_read == kAudioInputSize) {
            // Run audio classification inference
            int64_t start_time = esp_timer_get_time();
            if (kTfLiteOk != audio_interpreter->Invoke()) {
                MicroPrintf("WARN: Audio Invoke failed.");
            }
            int64_t end_time = esp_timer_get_time();
            long long audio_inference_time = (end_time - start_time) / 1000;
            // ESP_LOGI(TAG, "Audio Inference Time: %lld ms", audio_inference_time); // Optional timing log

            // Process audio classification results
            TfLiteTensor* audio_output = audio_interpreter->output(0);
            // Assuming audio model output is also int8. Verify audio_output->type!
            int8_t top_category_index = -1;
            int8_t top_category_score_quant = -128; // Min int8 value

            // Find the category with the highest score
            for (int i = 0; i < kAudioCategoryCount; ++i) {
                int8_t current_score_quant = audio_output->data.int8[i];
                if (current_score_quant > top_category_score_quant) {
                    top_category_score_quant = current_score_quant;
                    top_category_index = i;
                }
            }

            // Dequantize the top score
            float audio_scale = audio_output->params.scale;
            int audio_zero_point = audio_output->params.zero_point;
            float top_category_score_f = (top_category_score_quant - audio_zero_point) * audio_scale;

            // Log and respond to the detected audio command
            RespondToAudioCommand(error_reporter, top_category_index, top_category_score_f);

            // --- Control based on Audio Classification ---
            // Send serial command based on the detected audio command (if confidence is high enough)
            const float audio_threshold = 0.8; // Confidence threshold
            if (top_category_score_f > audio_threshold && top_category_index != 0 /* Ignore Background Noise */) {
                ESP_LOGI(TAG, "Audio command detected: %s (Score: %.2f)", kAudioCategoryLabels[top_category_index], top_category_score_f);
                // Define Elegoo serial commands (matching your original format)
                // {'H':'Elegoo','N':1,'D1':<Direction>,'D2':<Speed>,'D3':<Enable>}
                // Direction: 0=Stop, 1=Fwd, 2=Bwd, 3=Left, 4=Right
                // Speed: 0-255
                // Enable: 1=Enable, 0=Disable (usually 1)
                uint8_t serial_cmd[50] = {0}; // Buffer for command string
                int speed = 150; // Default speed, adjust as needed

                switch (top_category_index) {
                    case 1: // DriveBackward
                        snprintf((char*)serial_cmd, sizeof(serial_cmd), "{'H':'Elegoo','N':1,'D1':2,'D2':%d,'D3':1}", speed);
                        break;
                    case 2: // DriveForward
                        snprintf((char*)serial_cmd, sizeof(serial_cmd), "{'H':'Elegoo','N':1,'D1':1,'D2':%d,'D3':1}", speed);
                        break;
                    case 3: // DriveToMe (Treat as Forward for now)
                         snprintf((char*)serial_cmd, sizeof(serial_cmd), "{'H':'Elegoo','N':1,'D1':1,'D2':%d,'D3':1}", speed);
                        break;
                    case 4: // Stop
                        snprintf((char*)serial_cmd, sizeof(serial_cmd), "{'H':'Elegoo','N':1,'D1':0,'D2':0,'D3':1}");
                        break;
                    case 5: // TurnLeft
                        snprintf((char*)serial_cmd, sizeof(serial_cmd), "{'H':'Elegoo','N':1,'D1':3,'D2':%d,'D3':1}", speed);
                        break;
                    case 6: // TurnRight
                        snprintf((char*)serial_cmd, sizeof(serial_cmd), "{'H':'Elegoo','N':1,'D1':4,'D2':%d,'D3':1}", speed);
                        break;
                    default:
                        // Should not happen if index != 0 checked
                        break;
                }

                if (serial_cmd[0] != 0) { // If a command was formatted
                    ESP_LOGI(TAG, "Sending USB Serial: %s", serial_cmd);
                    usbSerial.write(serial_cmd, strlen((char*)serial_cmd));
                }

            } else if (person_detected && top_category_index == 0) {
                // OPTIONAL: If person detected AND only background noise, maybe move forward?
                uint8_t person_fwd_cmd[] = "{'H':'Elegoo','N':1,'D1':1,'D2':50,'D3':1}";
                ESP_LOGI(TAG, "Person detected, background noise - Moving Forward");
                usbSerial.write(person_fwd_cmd, sizeof(person_fwd_cmd)-1);
            } else if (top_category_index == 0) {
                // No command detected (background noise), ensure robot is stopped
                // (unless person is detected, handled above)
                uint8_t stop_cmd[] = "{'H':'Elegoo','N':1,'D1':0,'D2':0,'D3':1}";
                ESP_LOGI(TAG, "Background noise detected - Sending Stop");
                usbSerial.write(stop_cmd, sizeof(stop_cmd)-1);
                // Send Stop command periodically might be safer
            }

        } else {
             ESP_LOGD(TAG, "Insufficient audio samples received (%d/%d). Skipping audio inference.", samples_read, kAudioInputSize);
        }
    } // End of successful audio capture block

    vTaskDelay(pdMS_TO_TICKS(10)); // Small delay to yield task, adjust as needed (e.g., 10ms)

    #endif // CLI_ONLY_INFERENCE
}

// --- Inference function for CLI ---
// Keep this similar to original, but use initialized interpreter
void run_inference(void *ptr) {
    tflite::MicroInterpreter* person_interpreter = get_person_detection_interpreter();
    TfLiteTensor* person_input = get_person_detection_input_tensor();
    tflite::ErrorReporter* error_reporter = get_error_reporter();

    if (!person_interpreter || !person_input || !error_reporter) {
        MicroPrintf("ERROR: Person detection TFLM objects not initialized for CLI!");
        return;
    }

    // Convert uint8 image data to int8 (assuming input image `ptr` is uint8)
    // IMPORTANT: Verify person_input tensor type! If it's uint8, remove the ^ 0x80 conversion.
    if (person_input->type == kTfLiteInt8) {
        int8_t* input_data_int8 = person_input->data.int8;
        uint8_t* ptr_uint8 = (uint8_t*) ptr;
        for (int i = 0; i < kPersonMaxImageSize; i++) {
            input_data_int8[i] = ptr_uint8[i] ^ 0x80; // Or ptr_uint8[i] - 128
        }
    } else if (person_input->type == kTfLiteUInt8) {
         memcpy(person_input->data.uint8, ptr, kPersonMaxImageSize);
    } else {
         error_reporter->Report("ERROR: Person input tensor has unexpected type %d", person_input->type);
         return;
    }

    #if defined(COLLECT_CPU_STATS)
        // Reset and start timers (ensure these extern vars are defined elsewhere if using)
        // extern long long softmax_total_time; // etc...
        // softmax_total_time = dc_total_time = ... = 0;
        long long start_time = esp_timer_get_time();
    #endif

    // Run the model
    if (kTfLiteOk != person_interpreter->Invoke()) {
        MicroPrintf("Invoke failed.");
    }

    #if defined(COLLECT_CPU_STATS)
      long long total_time = (esp_timer_get_time() - start_time);
      printf("Total time = %lld\n", total_time / 1000);
      // Print other specific op times if needed
    #endif

    // Process the results (same dequantization logic as loop)
    TfLiteTensor* person_output = person_interpreter->output(0);
    int8_t person_score_quant = person_output->data.int8[kPersonIndex]; // Assuming int8 output
    int8_t no_person_score_quant = person_output->data.int8[kNotAPersonIndex]; // Assuming int8 output
    float person_scale = person_output->params.scale;
    int person_zero_point = person_output->params.zero_point;
    float person_score_f = (person_score_quant - person_zero_point) * person_scale;
    float no_person_score_f = (no_person_score_quant - person_zero_point) * person_scale;

    RespondToDetection(error_reporter, person_score_f, no_person_score_f);
}
