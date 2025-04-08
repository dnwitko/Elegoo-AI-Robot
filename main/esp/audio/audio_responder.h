// main/esp/audio/audio_responder.h

// Provides an interface to take an action based on the output of the audio
// classification model.

#ifndef APP_ESP_AUDIO_AUDIO_RESPONDER_H_
#define APP_ESP_AUDIO_AUDIO_RESPONDER_H_

#include "tensorflow/lite/c/common.h"
#include "tensorflow/lite/micro/tflite_bridge/micro_error_reporter.h"

// Called every time the results of an audio classification run are available.
// Arguments:
//   error_reporter: For logging purposes.
//   command_index: The index of the most likely detected command (or background noise).
//                  Corresponds to the line number in labels.txt and the index
//                  in the kAudioCategoryLabels array. -1 if no command found.
//   score:         The confidence score (0.0 to 1.0) for the detected command.
//   is_new_command: True if this is the first time this command is detected
//                   since the last different command (or silence). Useful for
//                   debouncing or triggering actions only once per command word.
//                   (NOTE: Implementation of is_new_command logic is TBD in .cc file)
void RespondToAudioCommand(tflite::ErrorReporter* error_reporter,
                           int8_t command_index,
                           float score);
                           // bool is_new_command); // Keep this commented out for now


#endif // APP_ESP_AUDIO_AUDIO_RESPONDER_H_