// main/core/model_init.h

#ifndef APP_CORE_MODEL_INIT_H_
#define APP_CORE_MODEL_INIT_H_

#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/schema/schema_generated.h"

// Initializes the TFLM interpreters, tensor arenas, and error reporter.
// Returns true on success, false on failure.
// NOTE: Call this function only once during setup.
bool model_init();

// Provides access to the initialized error reporter.
// Returns nullptr if model_init() has not been called successfully.
tflite::ErrorReporter* get_error_reporter();

// Provides access to the initialized person detection interpreter.
// Returns nullptr if model_init() has not been called successfully.
tflite::MicroInterpreter* get_person_detection_interpreter();

// Provides access to the initialized audio classification interpreter.
// Returns nullptr if model_init() has not been called successfully.
tflite::MicroInterpreter* get_audio_classifier_interpreter();

// Provides access to the input tensor for the person detection model.
// Returns nullptr if model_init() has not been called successfully.
TfLiteTensor* get_person_detection_input_tensor();

// Provides access to the input tensor for the audio classification model.
// Returns nullptr if model_init() has not been called successfully.
TfLiteTensor* get_audio_classifier_input_tensor();

#endif // APP_CORE_MODEL_INIT_H_