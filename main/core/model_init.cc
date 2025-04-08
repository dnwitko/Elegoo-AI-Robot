// main/core/model_init.cc

#include "core/model_init.h"

#include "models/model_settings.h"
// Include the headers that declare the model data arrays
#include "models/face_model/person_detect_model_data.h"

// TensorFlow Lite Micro specific headers
#include "tensorflow/lite/micro/tflite_bridge/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/schema/schema_generated.h"
// Include common micro ops
#include "tensorflow/lite/micro/kernels/micro_ops.h" // Provides AddBuiltin calls

// Logging wrapper
#include "esp_log.h"
static const char* TAG = "MODEL_INIT";

// --- Globals / Statics ---
// (These are scoped to this file, access is provided via getter functions)

namespace { // Use an anonymous namespace for C++ style static scoping

    // Error Reporter: Standard TFLM object for logging errors.
    tflite::MicroErrorReporter micro_error_reporter;
    tflite::ErrorReporter* error_reporter = nullptr;
    
    // Models: Pointers to the model data loaded from the flatbuffer arrays.
    const tflite::Model* person_model = nullptr;
    const tflite::Model* audio_model = nullptr;
    
    // --- EMBEDDED AUDIO MODEL OVERRIDE ---
    // These symbols are auto-created by the linker when using `EMBED_FILES` in CMake
    extern const uint8_t _binary_audio_model_tflite_start[] asm("_binary_audio_model_tflite_start");
    extern const uint8_t _binary_audio_model_tflite_end[]   asm("_binary_audio_model_tflite_end");
    const unsigned char* g_audio_model_data = _binary_audio_model_tflite_start;
    const int g_audio_model_data_len = _binary_audio_model_tflite_end - _binary_audio_model_tflite_start;
    
    // Op Resolver: Registers the TFLM operations needed by the models.
    tflite::MicroMutableOpResolver<15> micro_op_resolver;
    
    // Interpreters: One for each model.
    tflite::MicroInterpreter* person_interpreter = nullptr;
    tflite::MicroInterpreter* audio_interpreter = nullptr;
    
    // Tensor Arenas
    constexpr size_t kPersonArenaSize = 150 * 1024;
    constexpr size_t kAudioArenaSize  = 200 * 1024;
    
    uint8_t* person_tensor_arena = nullptr;
    uint8_t* audio_tensor_arena = nullptr;
    
    // Input tensor pointers
    TfLiteTensor* person_input_tensor = nullptr;
    TfLiteTensor* audio_input_tensor = nullptr;
    
    // Flag to track initialization state
    bool initialized = false;
    
    } // namespace

// --- Function Implementations ---

// Initializes the TFLM components
bool model_init() {
    if (initialized) {
        ESP_LOGW(TAG, "model_init() already called.");
        return true; // Already initialized successfully
    }
    ESP_LOGI(TAG, "Starting Model Initialization...");

    // 1. Set up the Error Reporter
    error_reporter = &micro_error_reporter;
    ESP_LOGI(TAG, "Error Reporter Initialized.");

    // 2. Load the Models
    person_model = tflite::GetModel(g_person_detect_model_data);
    if (person_model->version() != TFLITE_SCHEMA_VERSION) {
        MicroPrintf("Person model provided is schema version %d not equal to supported version %d.", // Use MicroPrintf
            person_model->version(), TFLITE_SCHEMA_VERSION);
        return false;
    }
    ESP_LOGI(TAG, "Person Detection Model Loaded (Size: %d bytes).", g_person_detect_model_data_len);

    audio_model = tflite::GetModel(g_audio_model_data);
    if (audio_model->version() != TFLITE_SCHEMA_VERSION) {
        MicroPrintf("Audio model provided is schema version %d not equal to supported version %d.", // Use MicroPrintf
            audio_model->version(), TFLITE_SCHEMA_VERSION);
        return false;
    }
    ESP_LOGI(TAG, "Audio Classification Model Loaded (Size: %d bytes).", g_audio_model_data_len);


    // 3. Register Operations for the Op Resolver
    // TODO: Add only the Ops needed by your specific models for efficiency.
    //       You can find needed Ops using `tflite_ops_check.py` script or Netron.
    //       For now, register common ops found in many image/audio models.
    micro_op_resolver.AddAveragePool2D();
    micro_op_resolver.AddConv2D();
    micro_op_resolver.AddDepthwiseConv2D();
    micro_op_resolver.AddReshape();
    micro_op_resolver.AddSoftmax(); // Or AddLogistic if your models use that
    micro_op_resolver.AddMaxPool2D();
    micro_op_resolver.AddFullyConnected();
    // Add other ops like AddRelu, AddAdd, AddMul, etc., if needed.
    // micro_op_resolver.AddQuantize(); // Typically not needed if model is already quantized
    // micro_op_resolver.AddDequantize(); // Add if float output needed from int model

    ESP_LOGI(TAG, "Operators Registered.");


    // 4. Allocate Tensor Arenas from Heap
    // Consider MALLOC_CAP_SPIRAM if PSRAM is enabled and internal RAM is insufficient
    person_tensor_arena = (uint8_t*) malloc(kPersonArenaSize);
    if (person_tensor_arena == nullptr) {
         MicroPrintf("E: Failed to allocate person_tensor_arena (size: %d)", kPersonArenaSize);
    }
    audio_tensor_arena = (uint8_t*) malloc(kAudioArenaSize);
     if (audio_tensor_arena == nullptr) {
        MicroPrintf("E: Failed to allocate audio_tensor_arena (size: %d)", kAudioArenaSize);
         // Free already allocated arena before returning
         free(person_tensor_arena);
         person_tensor_arena = nullptr;
         return false;
    }
    ESP_LOGI(TAG, "Tensor Arenas Allocated (Person: %d KB, Audio: %d KB).", kPersonArenaSize / 1024, kAudioArenaSize / 1024);


    // 5. Instantiate Interpreters and Allocate Tensors
    // --- Person Detection Interpreter ---
    static tflite::MicroInterpreter static_person_interpreter(
        person_model, micro_op_resolver, person_tensor_arena, kPersonArenaSize);
    person_interpreter = &static_person_interpreter;

    TfLiteStatus allocate_status = person_interpreter->AllocateTensors();
    if (allocate_status != kTfLiteOk) {
        MicroPrintf("E: Person interpreter AllocateTensors() failed.");
        // Free arenas before returning
        free(person_tensor_arena);
        free(audio_tensor_arena);
        person_tensor_arena = nullptr;
        audio_tensor_arena = nullptr;
        return false;
    }
    person_input_tensor = person_interpreter->input(0); // Get input tensor pointer
    ESP_LOGI(TAG, "Person Detection Interpreter Initialized.");
    ESP_LOGI(TAG, "Input tensor dims: %d, type: %d", person_input_tensor->dims->size, person_input_tensor->type);


    // --- Audio Classification Interpreter ---
    static tflite::MicroInterpreter static_audio_interpreter(
        audio_model, micro_op_resolver, audio_tensor_arena, kAudioArenaSize);
    audio_interpreter = &static_audio_interpreter;

    allocate_status = audio_interpreter->AllocateTensors();
    if (allocate_status != kTfLiteOk) {
        MicroPrintf("E: Person interpreter AllocateTensors() failed."); // Use MicroPrintf
         // Free arenas before returning
        free(person_tensor_arena);
        free(audio_tensor_arena);
        person_tensor_arena = nullptr;
        audio_tensor_arena = nullptr;
        // Note: person_interpreter is static, no need to delete
        return false;
    }
    audio_input_tensor = audio_interpreter->input(0); // Get input tensor pointer
    ESP_LOGI(TAG, "Audio Classification Interpreter Initialized.");
    ESP_LOGI(TAG, "Input tensor dims: %d, type: %d", audio_input_tensor->dims->size, audio_input_tensor->type);


    // 6. Initialization Complete
    initialized = true;
    ESP_LOGI(TAG, "Model Initialization Finished Successfully.");
    return true;
}

// --- Getter Functions ---

tflite::ErrorReporter* get_error_reporter() {
    return error_reporter;
}

tflite::MicroInterpreter* get_person_detection_interpreter() {
    return initialized ? person_interpreter : nullptr;
}

tflite::MicroInterpreter* get_audio_classifier_interpreter() {
    return initialized ? audio_interpreter : nullptr;
}

TfLiteTensor* get_person_detection_input_tensor() {
     return initialized ? person_input_tensor : nullptr;
}

TfLiteTensor* get_audio_classifier_input_tensor() {
    return initialized ? audio_input_tensor : nullptr;
}