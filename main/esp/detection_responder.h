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

// main/esp/detection_responder.h

// Provides an interface to take an action based on the output from the person
// detection model.

#ifndef APP_ESP_DETECTION_RESPONDER_H_ // <<< Updated include guard
#define APP_ESP_DETECTION_RESPONDER_H_

#include "tensorflow/lite/c/common.h"
#include "tensorflow/lite/micro/tflite_bridge/micro_error_reporter.h" // <<< Added for ErrorReporter (optional but good practice)

// Called every time the results of a person detection run are available.
// Arguments:
//   error_reporter: For logging purposes within the function.
//   person_score: Numerical confidence that the image contains a person.
//   no_person_score: Numerical confidence that the image does not contain a person.
// Typically if person_score > no_person_score, the image is considered to contain a person.
void RespondToDetection(tflite::ErrorReporter* error_reporter, // <<< Added error_reporter
                      float person_score, float no_person_score);

#endif // APP_ESP_DETECTION_RESPONDER_H_