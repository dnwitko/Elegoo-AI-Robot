// main/esp/audio/audio_provider.cc

#include "esp/audio/audio_provider.h"

// Include ESP-IDF driver headers
#include "driver/i2s.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include <esp_heap_caps.h> // For heap_caps_malloc

// Include FreeRTOS headers for ring buffer
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/ringbuf.h"

// Include TFLM headers and project settings
#include "tensorflow/lite/micro/tflite_bridge/micro_error_reporter.h"
#include "models/model_settings.h" // For kAudioSampleRate

// Tag for logging
static const char* TAG = "AUDIO_PROVIDER";

// --- Configuration Constants ---

// I2S Configuration for ESP32-S3-EYE v2.x (Check board schematic if different)
#define CONFIG_I2S_BCK_PIN      GPIO_NUM_1   // I2S Bit Clock
#define CONFIG_I2S_LRCK_PIN     GPIO_NUM_2   // I2S Word Select (Left/Right Clock)
#define CONFIG_I2S_DATA_PIN     GPIO_NUM_3   // I2S Data In
#define CONFIG_I2S_PORT         I2S_NUM_0    // Use I2S Port 0

// Audio Buffer Configuration
// Size of DMA buffers (should be multiple of frame size)
#define CONFIG_AUDIO_DMA_BUFFER_COUNT 4
#define CONFIG_AUDIO_DMA_BUFFER_SIZE  512  // Bytes per buffer
// Ring buffer size (needs to be large enough to hold audio between inferences)
// Suggest >= 2x model input size in bytes (kAudioInputSize * sizeof(int16_t))
#define CONFIG_AUDIO_RING_BUFFER_SIZE (kAudioInputSize * sizeof(int16_t) * 3)

// --- Static Variables ---
namespace { // Anonymous namespace for C++ style static scoping

// Ring buffer to store captured audio samples (int16_t format)
RingbufHandle_t g_audio_ring_buffer_handle = nullptr;
// Buffer used by I2S driver to read data via DMA
int16_t* g_i2s_read_buffer = nullptr;

// Track the total number of samples captured
volatile int32_t g_total_samples_captured = 0;
// Timestamp of the last sample retrieved by GetAudioSamples
int32_t g_latest_audio_timestamp = 0;

// Flag to indicate if audio capture has started
volatile bool g_audio_capture_started = false;

} // namespace


// --- I2S Initialization ---
static esp_err_t i2s_init(tflite::ErrorReporter* error_reporter) {
    // Configure I2S
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX), // Master, RX mode
        .sample_rate = kAudioSampleRate,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT, // Mono microphone
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1, // Interrupt level 1
        .dma_buf_count = CONFIG_AUDIO_DMA_BUFFER_COUNT,
        .dma_buf_len = CONFIG_AUDIO_DMA_BUFFER_SIZE / sizeof(int16_t), // Length in samples
        .use_apll = false, // Use default PLL
        .tx_desc_auto_clear = false,
        .fixed_mclk = 0
    };

    // Install I2S driver
    esp_err_t err = i2s_driver_install(CONFIG_I2S_PORT, &i2s_config, 0, NULL);
    if (err != ESP_OK) {
        error_reporter->Report("ERROR: I2S driver install failed: %s", esp_err_to_name(err));
        return err;
    }

    // Configure I2S pins
    i2s_pin_config_t pin_config = {
        .bck_io_num = CONFIG_I2S_BCK_PIN,
        .ws_io_num = CONFIG_I2S_LRCK_PIN,
        .data_out_num = I2S_PIN_NO_CHANGE, // Not used for RX
        .data_in_num = CONFIG_I2S_DATA_PIN
    };

    err = i2s_set_pin(CONFIG_I2S_PORT, &pin_config);
    if (err != ESP_OK) {
        error_reporter->Report("ERROR: I2S set pin failed: %s", esp_err_to_name(err));
        i2s_driver_uninstall(CONFIG_I2S_PORT); // Clean up driver
        return err;
    }

    // Enable MCLK output (required by some MEMS microphones)
    // The ESP32-S3 internal microphone likely doesn't need this, but good practice
    // REG_SET_BIT(I2S_TX_CONF_REG(CONFIG_I2S_PORT), I2S_TX_UPDATE);
    // REG_SET_BIT(I2S_FIFO_CONF_REG(CONFIG_I2S_PORT), I2S_DSCR_EN);

    error_reporter->Report("I2S driver initialized successfully for %d Hz, 16-bit mono.", kAudioSampleRate);
    return ESP_OK;
}


// --- Audio Capture Task ---
// This task continuously reads data from the I2S peripheral via DMA
// and writes it into the ring buffer.
static void audio_capture_task(void* arg) {
    tflite::ErrorReporter* error_reporter = static_cast<tflite::ErrorReporter*>(arg);
    size_t bytes_read = 0;

    while (g_audio_capture_started) {
        // Read data from I2S driver into intermediate buffer
        esp_err_t ret = i2s_read(CONFIG_I2S_PORT,
                                 (void*)g_i2s_read_buffer,
                                 CONFIG_AUDIO_DMA_BUFFER_SIZE, // Max bytes to read
                                 &bytes_read,
                                 pdMS_TO_TICKS(100)); // Wait max 100ms for data

        if (ret != ESP_OK) {
             error_reporter->Report("ERROR: I2S read failed: %s", esp_err_to_name(ret));
             // Consider adding recovery logic here if needed
             vTaskDelay(pdMS_TO_TICKS(100)); // Wait before retrying
             continue;
        }

        if (bytes_read > 0 && g_audio_ring_buffer_handle != nullptr) {
            // Write data to the ring buffer (non-blocking)
            BaseType_t result = xRingbufferSend(g_audio_ring_buffer_handle,
                                               (void*)g_i2s_read_buffer,
                                               bytes_read,
                                               pdMS_TO_TICKS(10)); // Wait max 10ms if buffer full

            if (result != pdTRUE) {
                // Ring buffer might be full, data is lost.
                // This indicates the consumer (GetAudioSamples) isn't keeping up
                // or the ring buffer is too small.
                ESP_LOGV(TAG, "Ring buffer full, discarding %d bytes", bytes_read);
            } else {
                // Update total samples captured (atomic update not strictly needed here if only read elsewhere)
                g_total_samples_captured += (bytes_read / sizeof(int16_t));
                 ESP_LOGV(TAG, "Wrote %d bytes to ring buffer", bytes_read);
            }
        } else if (bytes_read == 0) {
             ESP_LOGV(TAG, "I2S read 0 bytes"); // Might happen if task yields exactly between DMA buffers
        }
    } // end while

    // Cleanup if task exits
    free(g_i2s_read_buffer);
    g_i2s_read_buffer = nullptr;
    vRingbufferDelete(g_audio_ring_buffer_handle);
    g_audio_ring_buffer_handle = nullptr;
    i2s_driver_uninstall(CONFIG_I2S_PORT);
    ESP_LOGI(TAG, "Audio capture task finished.");
    vTaskDelete(NULL); // Delete self
}


// --- Public API Functions ---

TfLiteStatus InitAudio(tflite::ErrorReporter* error_reporter) {
    if (g_audio_capture_started) {
        error_reporter->Report("WARN: Audio already initialized.");
        return kTfLiteOk;
    }

    // 1. Initialize I2S peripheral and driver
    if (i2s_init(error_reporter) != ESP_OK) {
        return kTfLiteError;
    }

    // 2. Allocate I2S read buffer (use DMA capable memory if possible)
    g_i2s_read_buffer = (int16_t*) heap_caps_malloc(CONFIG_AUDIO_DMA_BUFFER_SIZE, MALLOC_CAP_DMA);
    if (g_i2s_read_buffer == nullptr) {
         error_reporter->Report("ERROR: Failed to allocate I2S read buffer.");
         i2s_driver_uninstall(CONFIG_I2S_PORT);
         return kTfLiteError;
    }

    // 3. Create the Ring Buffer
    // Use NoSplit type to ensure samples are contiguous when read
    g_audio_ring_buffer_handle = xRingbufferCreate(CONFIG_AUDIO_RING_BUFFER_SIZE, RINGBUF_TYPE_NOSPLIT);
    if (g_audio_ring_buffer_handle == nullptr) {
        error_reporter->Report("ERROR: Failed to create audio ring buffer.");
        free(g_i2s_read_buffer);
        g_i2s_read_buffer = nullptr;
        i2s_driver_uninstall(CONFIG_I2S_PORT);
        return kTfLiteError;
    }

    // 4. Start the Audio Capture Task
    g_audio_capture_started = true;
    g_total_samples_captured = 0;
    g_latest_audio_timestamp = 0;

    BaseType_t ret = xTaskCreate(audio_capture_task,
                                 "AudioCapture",   // Task name
                                 2 * 1024,         // Stack size (adjust as needed)
                                 error_reporter,   // Pass error reporter as argument
                                 10,               // Priority (adjust as needed)
                                 NULL);            // Task handle (optional)

    if (ret != pdPASS) {
        error_reporter->Report("ERROR: Failed to create audio capture task.");
        g_audio_capture_started = false; // Prevent task from running if creation failed
        free(g_i2s_read_buffer);
        g_i2s_read_buffer = nullptr;
        vRingbufferDelete(g_audio_ring_buffer_handle);
        g_audio_ring_buffer_handle = nullptr;
        i2s_driver_uninstall(CONFIG_I2S_PORT);
        return kTfLiteError;
    }

    error_reporter->Report("Audio Provider Initialized.");
    return kTfLiteOk;
}

// Retrieve audio samples for inference
TfLiteStatus GetAudioSamples(tflite::ErrorReporter* error_reporter,
                             int expected_samples,
                             int* samples_read_out, // Output param
                             int8_t* audio_samples_out // Output buffer (int8_t)
                             )
{
    if (!g_audio_capture_started || g_audio_ring_buffer_handle == nullptr) {
        error_reporter->Report("ERROR: Audio capture not initialized.");
        *samples_read_out = 0;
        return kTfLiteError;
    }

    // Calculate how many bytes are needed (model expects int8, but ring buffer holds int16)
    size_t expected_bytes = expected_samples * sizeof(int16_t);
    int16_t* temp_buffer = nullptr; // Intermediate buffer for int16 samples

    // Try to retrieve the required number of bytes (as int16_t) from the ring buffer
    size_t item_size = 0;
    void* item_ptr = xRingbufferReceiveUpTo(g_audio_ring_buffer_handle,
                                           &item_size,
                                           pdMS_TO_TICKS(0), // Don't wait (0 tick timeout)
                                           expected_bytes);

    if (item_ptr != nullptr && item_size > 0) {
        // Successfully retrieved data directly from the ring buffer
        temp_buffer = (int16_t*) item_ptr;
        int actual_samples_read = item_size / sizeof(int16_t);
        *samples_read_out = actual_samples_read;
        g_latest_audio_timestamp += actual_samples_read; // Update timestamp

         ESP_LOGV(TAG, "Read %d samples (%d bytes) from ring buffer", actual_samples_read, item_size);

        // --- Convert/Quantize int16_t samples to int8_t ---
        // Simple scaling: shift right by 8 bits (effectively dividing by 256)
        // This maps [-32768, 32767] to approx [-128, 127]
        for (int i = 0; i < actual_samples_read; ++i) {
            // You might need more sophisticated scaling/clipping depending on model training
            audio_samples_out[i] = static_cast<int8_t>(temp_buffer[i] >> 8);
        }

        // Return the buffer space back to the ring buffer
        vRingbufferReturnItem(g_audio_ring_buffer_handle, item_ptr);

        // Check if we got fewer samples than expected
        if (actual_samples_read < expected_samples) {
             ESP_LOGD(TAG, "Got %d samples, expected %d", actual_samples_read, expected_samples);
             // Return Ok, but the caller (main_functions) should check samples_read_out
        }
         return kTfLiteOk;

    } else {
        // No data available in the ring buffer currently
        *samples_read_out = 0;
         ESP_LOGV(TAG, "No audio data available in ring buffer yet.");
        // Return Ok, caller should check samples_read_out and potentially wait/retry
        return kTfLiteOk;
    }
}


// Get the timestamp of the last sample retrieved
int32_t GetLatestAudioTimestamp() {
    return g_latest_audio_timestamp;
}