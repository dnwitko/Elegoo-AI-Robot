// main/models/model_settings.h

#ifndef APP_MODEL_SETTINGS_H_ // <<< Renamed include guard
#define APP_MODEL_SETTINGS_H_

//----------------------------------------------------------------------------
// Person Detection Model Settings
//----------------------------------------------------------------------------
// Input dimensions (derived from model training)
constexpr int kPersonNumCols = 96;
constexpr int kPersonNumRows = 96;
constexpr int kPersonNumChannels = 1; // Assuming grayscale input
constexpr int kPersonMaxImageSize = kPersonNumCols * kPersonNumRows * kPersonNumChannels;

// Output categories
constexpr int kPersonCategoryCount = 2;
constexpr int kPersonIndex = 1;         // Index of the 'person' class
constexpr int kNotAPersonIndex = 0;     // Index of the 'no person' class
extern const char* kPersonCategoryLabels[kPersonCategoryCount]; // Defined in .cc

//----------------------------------------------------------------------------
// Audio Classification Model Settings
//----------------------------------------------------------------------------
// --- Audio Input Settings ---
constexpr int kAudioSampleRate = 48000;     // <<< CONFIRMED sample rate (samples/second) >>>

// Input size confirmed from model input tensor shape.
constexpr int kAudioInputSize = 44032;

// Calculate duration based on confirmed sample rate and input size.
// (44032 * 1000) / 48000 = 917.33...  We'll use 917ms.
constexpr int kAudioSliceDurationMs = (kAudioInputSize * 1000) / kAudioSampleRate; // Should be 917 ms

// --- Audio Output Settings ---
// This count MUST match the number of lines in main/models/audio_model/labels.txt
constexpr int kAudioCategoryCount = 7; // 0: Background Noise, 1: DriveBackward, ..., 6: TurnRight

// Labels for audio commands (to be defined in .cc using embedded labels.txt)
extern const char* kAudioCategoryLabels[kAudioCategoryCount];

#endif // APP_MODEL_SETTINGS_H_