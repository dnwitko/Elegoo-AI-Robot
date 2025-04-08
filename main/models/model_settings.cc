// main/models/model_settings.cc

#include "models/model_settings.h" // Use relative path from main dir

// Person Detection Labels
// Ensure this matches the order and count defined in model_settings.h
const char* kPersonCategoryLabels[kPersonCategoryCount] = { // <<< Use kPersonCategoryCount
    "no person", // <<< Updated label for clarity (matches common examples)
    "person",
};

// Audio Classification Labels
// Ensure this matches the order, count, and content of models/audio_model/labels.txt
const char* kAudioCategoryLabels[kAudioCategoryCount] = { // <<< Use kAudioCategoryCount
    "Background Noise", // Index 0
    "DriveBackward",    // Index 1
    "DriveForward",     // Index 2
    "DriveToMe",        // Index 3
    "Stop",             // Index 4
    "TurnLeft",         // Index 5
    "TurnRight",        // Index 6
};

// Note: We define kAudioCategoryLabels manually here for simplicity.
// The build system also embeds models/audio_model/labels.txt.
// It's CRUCIAL that this manual definition exactly matches the embedded file content.
// An alternative would be to parse the embedded file at runtime, but that adds complexity.