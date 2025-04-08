// main/esp/audio/audio_provider.h

#ifndef APP_ESP_AUDIO_AUDIO_PROVIDER_H_
#define APP_ESP_AUDIO_AUDIO_PROVIDER_H_

#include "tensorflow/lite/c/common.h"
#include "tensorflow/lite/micro/tflite_bridge/micro_error_reporter.h"

// Provides an interface to access audio data from a microphone.

// Initializes the audio frontend (e.g., I2S microphone).
// Returns kTfLiteOk on success, kTfLiteError on failure.
TfLiteStatus InitAudio(tflite::ErrorReporter* error_reporter);

// Attempts to retrieve a slice of audio samples for inference.
// The implementation should handle buffering and slicing the continuous audio stream.
// Arguments:
//   error_reporter:   For logging errors.
//   expected_samples: The number of samples required for one inference (e.g., kAudioInputSize).
//   samples_read:     Output parameter populated with the number of samples actually retrieved.
//                     This might be less than expected_samples if data isn't ready.
//   audio_samples:    Destination buffer to copy the audio samples into. The buffer must be
//                     large enough to hold 'expected_samples' of type int8_t.
//                     The implementation needs to convert raw microphone data (usually int16_t)
//                     to the int8_t format expected by the quantized model.
// Returns kTfLiteOk if samples were successfully retrieved (even if fewer than expected),
// kTfLiteError on critical failure.
TfLiteStatus GetAudioSamples(tflite::ErrorReporter* error_reporter,
                             int expected_samples,
                             int* samples_read, // Output: number of samples actually read
                             int8_t* audio_samples); // Output: buffer to fill

// Returns the number of audio samples captured since the last time this function was called.
// Useful for synchronization or debugging. Returns -1 on error.
int32_t GetLatestAudioTimestamp();

#endif // APP_ESP_AUDIO_AUDIO_PROVIDER_H_