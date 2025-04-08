# Elegoo-AI-Robot: Vision and Voice Control with LiteRT

This project supplements the original [Elegoo-AI-Robot](https://github.com/henrytran720/Elegoo-AI-Robot) repository by **@henrytran720**. It integrates both computer vision (person/face detection) and voice command recognition using quantized TensorFlow Lite Micro models, deployed on an ESP32-S3-EYE module, to control an Elegoo robot car via serial commands.

This repository represents work undertaken for the 2025 University Research Symposium, focusing on analyzing the performance impact of hardware acceleration (ESP-NN) on embedded machine learning models.

Sections marked (WIP - Work In Progress) are subject to change.

## How it Works

1.  **Vision:** The ESP32-S3-EYE uses its camera and a TensorFlow Lite Micro model (originally person detection, potentially adaptable to face detection) to detect a person/face.
2.  **Voice Commands:** Upon visual confirmation (or potentially always listening, depending on the config), the ESP32-S3-EYE uses its microphone and a second TensorFlow Lite Micro audio classification model to recognize spoken commands (e.g., "Drive Forward",   
    "Stop", "Turn Left", "Turn Right").
3.  **Control:** Recognized voice commands are translated into specific JSON-formatted serial commands.
4.  **Actuation:** These serial commands are sent via a Micro USB-to-UART adapter to the Elegoo robot car's microcontroller, causing the robot to move accordingly.
5.  **Research:** The system allows for enabling/disabling ESP-NN hardware acceleration to measure and compare inference latency and responsiveness for the quantized models running on the ESP32 edge device.

## Hardware Requirements

- [Elegoo Smart Car Robot](https://us.elegoo.com/products/elegoo-smart-robot-car-kit-v-4-0) - The robot platform.
- [ESP32-S3-EYE Dev Board](https://www.aliexpress.us/item/3256803794751194.html) - The microcontroller performing sensing and ML inference.
- [CP2102 Micro USB to UART Converter](https://www.amazon.com/HiLetgo-CP2102-Module-Converter-Replace/dp/B01N47LXRA) - Bridges USB Host on ESP32 to Robot's UART.
- [JST XH to Dupont Connector Kit](https://www.amazon.com/Kidisoii-Dupont2-54-Connector-Pre-Crimped-Compatible/dp/B0CMCN9CXD/135-4941321-1839956) - For fabricating the ESP32 microUSB -> Robot UART cable.
- [Micro USB to Micro USB Male-to-Male OTG Cable](https://www.amazon.com/Micro-USB-Male-Data-Cable/dp/B0872GMD7V/) - Connects the ESP32-S3-EYE to the CP2102 adapter.
- [USB to Micro USB Data Transfer Cable](https://www.amazon.com/FEMORO-Transfer-Charging-Smartphone-Bluetooth/dp/B0D2KZQR8T) - For flashing the ESP32-S3-EYE from your computer.

## Software Dependencies

All dependencies have been already added to [idf_component.yml](https://github.com/dnwitko/Elegoo-AI-Robot/blob/main/main/idf_component.yml) thanks to Henry, and should be automatically downloaded upon compilation. For reference, here are all the dependencies this project requires:

- [ESP-IDF: Version v5.4](https://docs.espressif.com/projects/esp-idf/en/stable/esp32s3/get-started/index.html#manual-installation) (or your specific version) is required.
- [espressif/esp-tflite-micro](https://components.espressif.com/components/espressif/esp-tflite-micro): TensorFlow Lite Micro library optimized for Espressif chips (includes ESP-NN integration).
- [Espressif TinyUSB fork](https://components.espressif.com/components/espressif/tinyusb)
- [Espressif's additions to TinyUSB](https://components.espressif.com/components/espressif/esp_tinyusb)
- [USB Host CDC-ACM Class Drive](https://components.espressif.com/components/espressif/usb_host_cdc_acm/versions/2.0.3)
- [Virtual COM Port Service](https://components.espressif.com/components/espressif/usb_host_vcp)
- [espressif/esp32-camera](https://components.espressif.com/components/espressif/esp32-camera): Driver for the camera module.
- [espressif/esp-tflite-micro](https://components.espressif.com/components/espressif/esp-tflite-micro): TensorFlow Lite Micro library optimized for Espressif chips (includes ESP-NN integration).
- [bertmelis' USBHostSerial](https://github.com/bertmelis/USBHostSerial)
- Serial device drivers (Technically, you only need one, but to save time, all three have been included):
  - [CH34x USB-UART converter driver](https://components.espressif.com/components/espressif/usb_host_ch34x_vcp/versions/2.0.0)
  - [Silicon Labs CP210x USB-UART converter driver](https://components.espressif.com/components/espressif/usb_host_cp210x_vcp/versions/2.0.0)
  - [FTDI UART-USB converters driver](https://components.espressif.com/components/espressif/usb_host_ftdi_vcp/versions/2.0.0)

---

# Getting Started

Getting started involves following some instructions detailed in [Henry Tran's repository](https://github.com/henrytran720/Elegoo-AI-Robot) to assemble and prepare the Elegoo Robot car. After completing the steps listed in [The basics](https://github.com/henrytran720/Elegoo-AI-Robot?tab=readme-ov-file#the-basics) and [Assembly](https://github.com/henrytran720/Elegoo-AI-Robot?tab=readme-ov-file#assembly), you are ready to train your models proceed to setup.

## Vision/Voice Recognition (WIP)

This project utilizes Google's Teachable Machine to train vision and voice recognition models, exported as TensorFlow Lite (`.tflite`) files. To train your own models, visit Google's [Teachable Machine website](https://teachablemachine.withgoogle.com/). Once you have your models exported, you are ready to move on to **Setup and Flashing**.

## Setup and Flashing (WIP)

The setup process is very similar to Henry's. Start by cloning my repository:

```bash
git clone https://github.com/dnwitko/Elegoo-AI-Robot.git
cd Elegoo-AI-Robot
```

Due to the memory constraints of the ESP32, it is crucial to quantize your TF Lite models. We need to quantize them both to `int8` from `float32`. Use the `quantize_audio.py` script for this step. Then, convert the model to a C Array with the instructions below.

Use the `xxd` tool (or a similar utility) to convert each quantized `.tflite` model into a C byte array (`.cc`) and create a corresponding header (`.h`) declaring the array and its length variable.
        
```bash
# For audio model:
xxd -i quantized_audio_model.tflite > main/models/audio_model/audio_model_data.cc
# Add 'const int g_audio_model_data_len = <size>;' to the .cc file
# Create main/models/audio_model/audio_model_data.h declaring the array/len

# For vision model (replace 'person'  with 'face' if using person detection):
xxd -i quantized_vision_model.tflite > main/models/person_model/person_detect_model_data.cc
# Add 'const int g_person_detect_model_data_len = <size>;' to the .cc file
# Create main/models/person_model/person_detect_model_data.h declaring the array/len
```

From there, add your quantized models to their respective `models\` folder. Then, open your ESP-IDF environment and set your target:

```bash
# This will set up the project to be compiled for the ESP32-S3.
# If you're compiling this project for another platform, please set the target appropriately for your environment.
idf.py set-target esp32s3
```

You are now ready to flash the project to the ESP32. Connect the ESP32-S3-EYE to your computer using the **data transfer** Micro USB cable. Run the flash command (this will build, then flash):

```bash
idf.py flash
# (Optional) You may choose a port to flash to by running: idf.py flash -p [YOUR-ESP32-PORT]
# (e.g., idf.py flash -p COM4 on Windows, or /dev/ttyS4 on Linux/WSL)
```

Once the flashing process completes, you can unplug the ESP32 and plug it in to the serial adapter using the Micro USB to Micro USB OTG cable.

## Fine-tuning & Testing AI Capabilities (WIP)

The fine-tuning process is quite limited. The goal of fine-tuning is to verify the system's responsiveness and accuracy in various environments, in order to improve the car's interactivity and functionality. Since Teachable Machine doesn't support direct fine-tuning, you can simply re-training the model with new or more diverse data, and replacing your models in the same directory with your fine-tuned model (remember to quantize!). Do this if you would like to add new classes, or if you feel like your models aren't making accurate inferences.

## Research Symposium Focus (WIP)

For the research portion of this project, I will analyze the performance of the quantized TF Lite models running on the ESP32-S3-EYE under two conditions:

1.  **Without ESP-NN:** Hardware acceleration disabled via `menuconfig`.
2.  **With ESP-NN:** Hardware acceleration enabled via `menuconfig`.

The key metrics for comparison will be:

*   **Inference Latency:** The time taken (in milliseconds or microseconds) for each model (vision and audio) to perform a single inference, measured using `esp_timer`.
*   **System Responsiveness:** Measure of the delay between a command being spoken and the robot reacting.
*   **(Optional) CPU Utilization / Power Consumption:** If time and tools permit.

This analysis will demonstrate the real-world impact of Espressif's Neural Network acceleration on the feasibility and performance of running multiple ML models concurrently on an edge device.

(Data Collection WIP)

---

## License

TensorFlow and Espressif's sample code is covered by the Apache 2.0 license.

bertmelis' USBHostSerial code is covered by the MIT license.

Modifications made to both codebases for this project are also covered by the included MIT license.

## Credits

This project is based on Henry Tran's [Elegoo-AI-Robot](https://github.com/henrytran720/Elegoo-AI-Robot). Special thanks to **Henry Tran** for his work on the original code.

Thanks to **bertmelis** for the [USBHostSerial code](https://github.com/bertmelis/USBHostSerial).

Thanks to **Espressif** for the [ESP32 SDK](https://github.com/espressif/esp-idf).
