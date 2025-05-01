/* Copyright 2020-2023 The TensorFlow Authors. All Rights Reserved.
   Copyright (c) 2024 Bert Melis. All rights reserved. // Added from USBHostSerial example used in Elegoo-AI-Robot

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

// Standard C++ includes (may be needed by added code)
#include <algorithm>
#include <cstdint>
#include <iterator>
#include <cstring> // <<< Added: Needed for strcmp (for USB command logic)

// Original micro_speech includes
#include "main_functions.h"
#include "audio_provider.h"
// #include "command_responder.h" // <<< Removed: As requested, logic moved inline
#include "feature_provider.h"
#include "micro_model_settings.h"
#include "model.h"
#include "recognize_commands.h"

// Original TF Lite Micro includes
#include "tensorflow/lite/micro/system_setup.h" // <<< Added back: Standard TFLM setup call
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/core/c/common.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_log.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"

// <<< --- Start: Added System Includes (for USB and Task Delay) --- >>>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h" // Already used by MicroPrintf, but good to be explicit if adding more logs
// <<< --- End: Added System Includes --- >>>

// <<< --- Start: USB Host Include (Added from Elegoo-AI-Robot) --- >>>
#include "USBHostSerial.h" // Required for USB serial
// <<< --- End: USB Host Include --- >>>

// <<< --- Start: Global USB Serial Variable (Added, matching Elegoo-AI-Robot) --- >>>
USBHostSerial usbSerial; // Global instance for USB serial
// <<< --- End: Global USB Serial Variable --- >>>

// Globals, used for compatibility with Arduino-style sketches.
namespace {
const tflite::Model* model = nullptr;
tflite::MicroInterpreter* interpreter = nullptr;
TfLiteTensor* model_input = nullptr;
FeatureProvider* feature_provider = nullptr;
RecognizeCommands* recognizer = nullptr;
int32_t previous_time = 0;

// Create an area of memory to use for input, output, and intermediate arrays.
// The size of this will depend on the model you're using, and may need to be
// determined by experimentation.
constexpr int kTensorArenaSize = 30 * 1024;
// <<< Modified: Added alignas(16) for potential performance benefits/requirements >>>
alignas(16) uint8_t tensor_arena[kTensorArenaSize];
int8_t feature_buffer[kFeatureElementCount];
int8_t* model_input_buffer = nullptr;
}  // namespace

// The name of this function is important for Arduino compatibility.
void setup() {
  // <<< Added: Standard TFLM system setup call >>>
  tflite::InitializeTarget(); // Should be called before using TFLM interpreter
  MicroPrintf("--- Starting Micro Speech setup() ---"); // Added log

  // <<< --- Start: USB Host Serial Initialization (Added from Elegoo-AI-Robot) --- >>>
  MicroPrintf("--- Initializing USB Host Serial at 9600 baud... ---");
  // Use same parameters as Elegoo-AI-Robot
  if (!usbSerial.begin(9600, 0, 0, 8)) {
      MicroPrintf("--- Failed to initialize USB Host Serial! Halting. ---");
      // Halt execution if USB is essential
      while(1) { vTaskDelay(pdMS_TO_TICKS(1000)); }
  } else {
      MicroPrintf("--- USB Host Serial initialized (9600 baud). Waiting 1 sec... ---");
      // Optional delay, might help ensure the connected device is ready
      vTaskDelay(pdMS_TO_TICKS(1000));
  }
  // <<< --- End: USB Host Serial Initialization --- >>>

  // Map the model into a usable data structure. This doesn't involve any
  // copying or parsing, it's a very lightweight operation.
  model = tflite::GetModel(g_model);
  if (model->version() != TFLITE_SCHEMA_VERSION) {
    MicroPrintf("Model provided is schema version %d not equal to supported "
                "version %d.", model->version(), TFLITE_SCHEMA_VERSION);
    return;
  }

  // Pull in only the operation implementations we need.
  // This relies on a complete list of all the ops needed by this graph.
  // An easier approach is to just use the AllOpsResolver, but this will
  // incur some penalty in code space for op implementations that are not
  // needed by this graph.
  // NOLINTNEXTLINE(runtime-global-variables)
  static tflite::MicroMutableOpResolver<4> micro_op_resolver;
  if (micro_op_resolver.AddDepthwiseConv2D() != kTfLiteOk) {
      MicroPrintf("Failed AddDepthwiseConv2D"); // Added log
      return;
  }
  if (micro_op_resolver.AddFullyConnected() != kTfLiteOk) {
      MicroPrintf("Failed AddFullyConnected"); // Added log
      return;
  }
  if (micro_op_resolver.AddSoftmax() != kTfLiteOk) {
      MicroPrintf("Failed AddSoftmax"); // Added log
      return;
  }
  if (micro_op_resolver.AddReshape() != kTfLiteOk) {
      MicroPrintf("Failed AddReshape"); // Added log
      return;
  }

  // Build an interpreter to run the model with.
  static tflite::MicroInterpreter static_interpreter(
      model, micro_op_resolver, tensor_arena, kTensorArenaSize);
  interpreter = &static_interpreter;

  // Allocate memory from the tensor_arena for the model's tensors.
  TfLiteStatus allocate_status = interpreter->AllocateTensors();
  if (allocate_status != kTfLiteOk) {
    MicroPrintf("AllocateTensors() failed");
    return;
  }

  // Get information about the memory area to use for the model's input.
  model_input = interpreter->input(0);
  if ((model_input->dims->size != 2) || (model_input->dims->data[0] != 1) ||
      (model_input->dims->data[1] !=
       (kFeatureCount * kFeatureSize)) ||
      (model_input->type != kTfLiteInt8)) {
    MicroPrintf("Bad input tensor parameters in model");
    return;
  }
  model_input_buffer = tflite::GetTensorData<int8_t>(model_input);

  // Prepare to access the audio spectrograms from a microphone or other source
  // that will provide the inputs to the neural network.
  // NOLINTNEXTLINE(runtime-global-variables)
  static FeatureProvider static_feature_provider(kFeatureElementCount,
                                                 feature_buffer);
  feature_provider = &static_feature_provider;

  static RecognizeCommands static_recognizer;
  recognizer = &static_recognizer;

  previous_time = 0;
  MicroPrintf("--- Micro Speech setup() finished ---"); // Added log
}

// The name of this function is important for Arduino compatibility.
void loop() {
  // <<< --- Start: Check USB Connection (Added, good practice) --- >>>
  if (!usbSerial) {
      // Optional: Add logging or status indication for disconnection
      // MicroPrintf("USB Serial disconnected. Waiting..."); // Example log
      vTaskDelay(pdMS_TO_TICKS(100)); // Wait briefly before checking again
      return; // Skip the rest of the loop if not connected
  }
  // <<< --- End: Check USB Connection --- >>>

  // Fetch the spectrogram for the current time.
  const int32_t current_time = LatestAudioTimestamp();
  int how_many_new_slices = 0;
  TfLiteStatus feature_status = feature_provider->PopulateFeatureData(
      previous_time, current_time, &how_many_new_slices);
  if (feature_status != kTfLiteOk) {
    MicroPrintf( "Feature generation failed");
    return;
  }
  previous_time = current_time;
  // If no new audio samples have been received since last time, don't bother
  // running the network model.
  if (how_many_new_slices == 0) {
      // <<< Added: Small delay when idle to yield task scheduler >>>
      vTaskDelay(pdMS_TO_TICKS(10));
      return;
  }

  // Copy feature buffer to input tensor
  for (int i = 0; i < kFeatureElementCount; i++) {
    model_input_buffer[i] = feature_buffer[i];
  }

  // Run the model on the spectrogram input and make sure it succeeds.
  TfLiteStatus invoke_status = interpreter->Invoke();
  if (invoke_status != kTfLiteOk) {
    MicroPrintf( "Invoke failed");
    return;
  }

  // Obtain a pointer to the output tensor
  TfLiteTensor* output = interpreter->output(0);

  // <<< --- Start: Modified Result Processing --- >>>
  // Using the RecognizeCommands class to get smooth results and command strings.
  // The #if 0 block below is the original simple argmax logic, replaced by this.
  const char* found_command = nullptr;
  float score = 0;
  bool is_new_command = false;
  TfLiteStatus process_status = recognizer->ProcessLatestResults(
      output, current_time, &found_command, &score, &is_new_command);
  if (process_status != kTfLiteOk) {
    MicroPrintf("RecognizeCommands::ProcessLatestResults() failed");
    return;
  }

  // <<< --- Start: USB Command Sending Logic (Added/Adapted from Elegoo-AI-Robot) --- >>>
  // Only send a command if a new command was recognized this cycle.
  if (is_new_command) {
      MicroPrintf("New command: %s, Score: %.2f", found_command, static_cast<double>(score)); // Log recognized command

      // Define command JSON strings (using format from Elegoo-AI-Robot)
      uint8_t yes_cmd[] = "{'H':'Elegoo','N':1,'D1':0,'D2':50,'D3':1}"; // Forward command
      uint8_t no_cmd[]  = "{'H':'Elegoo','N':1,'D1':0,'D2':0,'D3':1}";  // Stop command
      uint8_t* cmd_to_send = nullptr;
      size_t cmd_len = 0;

      // Determine which command to send based on the recognized word
      if (strcmp(found_command, "yes") == 0) {
          cmd_to_send = yes_cmd;
          cmd_len = sizeof(yes_cmd) - 1; // Exclude null terminator
          MicroPrintf("Sending FORWARD command via USB Serial.");
      } else if (strcmp(found_command, "no") == 0) {
          cmd_to_send = no_cmd;
          cmd_len = sizeof(no_cmd) - 1; // Exclude null terminator
          MicroPrintf("Sending STOP command via USB Serial.");
      }
      // Add else if blocks here for other commands like "up", "down" if needed

      // Check if a valid command was identified and send it via USB Serial
      if (cmd_to_send) {
          if (usbSerial) { // Double-check connection just before writing
              // Use the write method from Elegoo-AI-Robot
              size_t written = usbSerial.write(cmd_to_send, cmd_len);
              if (written != cmd_len) {
                  MicroPrintf("Warning: USB write failed or incomplete (%d/%d bytes).", written, cmd_len);
              }
          } else {
               MicroPrintf("Warning: USB disconnected before write attempt.");
          }
      }
  }
  // <<< --- End: USB Command Sending Logic --- >>>

  // <<< --- Removed original RespondToCommand call --- >>>
  // RespondToCommand(current_time, found_command, score, is_new_command);

#if 0 // Original simple argmax logic - kept for reference but disabled
  float output_scale = output->params.scale;
  int output_zero_point = output->params.zero_point;
  int max_idx = 0;
  float max_result = 0.0;
  // Dequantize output values and find the max
  for (int i = 0; i < kCategoryCount; i++) {
    float current_result =
        (tflite::GetTensorData<int8_t>(output)[i] - output_zero_point) *
        output_scale;
    if (current_result > max_result) {
      max_result = current_result; // update max result
      max_idx = i; // update category
    }
  }
  if (max_result > 0.8f) {
    MicroPrintf("Detected %7s, score: %.2f", kCategoryLabels[max_idx],
        static_cast<double>(max_result));
  }
#endif
  // <<< --- End: Modified Result Processing --- >>>

  // <<< Added: Small delay at the end of the loop to yield task scheduler, similar to Elegoo-AI-Robot >>>
  vTaskDelay(pdMS_TO_TICKS(1));
}