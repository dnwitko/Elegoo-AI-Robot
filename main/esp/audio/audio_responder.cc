// main/esp/audio/audio_responder.cc

#include "esp/audio/audio_responder.h" // Our header

// Include project settings for label names
#include "models/model_settings.h"

// Include TFLM headers for logging
#include "tensorflow/lite/micro/tflite_bridge/micro_error_reporter.h"

// Include ESP-IDF log for potential standard logging
#include "esp_log.h"

// Tag for logging (optional)
// static const char* TAG = "AUDIO_RESPONDER";

// --- State Variables for Debouncing/New Command Detection (Optional - FUTURE USE) ---
namespace { // Anonymous namespace
// int current_command_index = -1;
// int64_t last_command_ms = 0;
// constexpr int kSilenceTimeoutMs = 1000; // e.g., 1 second of silence resets command state
// constexpr int kCommandDebounceMs = 300; // e.g., ignore same command within 300ms
} // namespace


// --- RespondToAudioCommand Implementation ---
void RespondToAudioCommand(tflite::ErrorReporter* error_reporter,
                           int8_t command_index,
                           float score)
                           // bool is_new_command) // Parameter currently commented out in header
{
    // Basic check for valid index
    if (command_index < 0 || command_index >= kAudioCategoryCount) {
        // This might indicate an issue in the main loop logic finding the max score
        // Or simply means background noise or low confidence overall resulted in no valid command index.
        // error_reporter->Report("Audio Response: No command detected (index: %d, score: %.2f)", command_index, score);
        // We'll rely on the main_functions loop logic to filter low confidence results.
        // This function is called *after* a potential command is identified there.
        return;
    }

    // --- Simple Logging Implementation ---
    // Get the label string corresponding to the detected command index
    const char* command_label = kAudioCategoryLabels[command_index];

    // Log the detected command and score using the TFLM error reporter
    error_reporter->Report("Audio Result: [%s] (Score: %.2f)",
                           command_label, score);


    // --- FUTURE WORK: Implement Debouncing / is_new_command logic ---
    /*
    int64_t current_time_ms = esp_timer_get_time() / 1000; // Get current time
    bool is_new_command_flag = false;

    // Check for silence timeout
    if ((current_time_ms - last_command_ms) > kSilenceTimeoutMs) {
        current_command_index = -1; // Reset state after silence
    }

    // Check if different command or first command after silence/debounce period
    if (command_index != current_command_index && (current_time_ms - last_command_ms) > kCommandDebounceMs) {
       if (command_index != 0) { // Ignore "Background Noise" as a "new" command trigger usually
            is_new_command_flag = true;
            current_command_index = command_index; // Update current command
            last_command_ms = current_time_ms;     // Update time
            error_reporter->Report(">>> New Command Detected: %s", command_label);
            // Trigger actions based on *new* command here if needed
        } else {
             // It's background noise, just update time if needed but don't flag as new command
             last_command_ms = current_time_ms;
        }
    } else if (command_index == current_command_index) {
        // Same command detected, just update the timestamp
        last_command_ms = current_time_ms;
    }

    // Pass is_new_command_flag to further logic if the parameter is re-enabled in the header
    */

    // --- FUTURE WORK: Update Display (Optional) ---
    // Could potentially update an icon or text on the LCD here based on the command
    // Requires coordination with GUI elements created in detection_responder.cc
    // Example:
    // #if APP_DISPLAY_SUPPORT
    //   bsp_display_lock(0);
    //   // Find and update an LVGL label object for audio status...
    //   lv_label_set_text(audio_status_label, command_label);
    //   bsp_display_unlock();
    // #endif
}