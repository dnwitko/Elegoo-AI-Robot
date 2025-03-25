# Elegoo-AI-Robot

This project is a fork of Henry Tran's [Elegoo-AI-Robot](https://github.com/henrytran720/Elegoo-AI-Robot) repository. It deploys low-latency computer vision and voice recognition models on an ESP32-S3-EYE module to control an Elegoo AI-powered robot car. Using Google’s Teachable Machine, the system trains and tests models for recognizing visual and auditory inputs. The ESP32-S3-EYE microcontroller supports machine learning inference while operating under strict memory and processing constraints. This project utilizes Lite Runtime (LiteRT), a framework which allows machine learning to be deployed on edge devices with small memory and processing power to optimize performance. It is hypothesized that LiteRT will reduce inference latency and improve responsiveness, demonstrating the feasibility of deploying AI on resource-constrained devices for robotics and smart home applications. This is my University Research Symposium Project for 2025.

### Original Sections (from Henry Tran's Repository)
- **Hardware Requirements** (with minor modification)
- **Software Dependencies** (inherited from the original)
- **Setup** (adapted from the original)

### My Contributions & Modifications
- **Vision/Voice Recognition** – Added Google's Teachable Machine support. (WIP)
- **LiteRT Optimization** – Integrated Lite Runtime for improved AI inference speed. (WIP)
- **Fine-tuning & Testing AI Capabilities** – Enabled both visual and auditory model training/testing. (WIP)
- **Research Symposium Focus** – My analysis on inference latency and responsiveness, created as part of my 2025 University Research Symposium. (WIP)

## Hardware Requirements

- [Elegoo Smart Car Robot](https://us.elegoo.com/products/elegoo-smart-robot-car-kit-v-4-0) - This is a core part of the project: the smart car.
- [ESP32-S3-EYE Dev Board](https://www.aliexpress.us/item/3256803794751194.html) - This is also a core part of the project: the camera microcontroller.
- [CP2102 Micro USB to UART Converter](https://www.amazon.com/HiLetgo-CP2102-Module-Converter-Replace/dp/B01N47LXRA) - Any serial device will work here, so long as it's compatible with the drivers below.
- [JST XH to Dupont Connector Kit](https://www.amazon.com/Kidisoii-Dupont2-54-Connector-Pre-Crimped-Compatible/dp/B0CMCN9CXD/135-4941321-1839956) - This is used to fabricate a cable for our serial adapter to UART connection.
- [Micro USB to Micro USB Male-to-Male OTG Cable](https://www.amazon.com/Micro-USB-Male-Data-Cable/dp/B0872GMD7V/) - This is used to communicate between the serial adapter and the ESP32-S3-EYE.
- [USB to Micro USB Data Transfer Cable](https://www.amazon.com/FEMORO-Transfer-Charging-Smartphone-Bluetooth/dp/B0D2KZQR8T) - This is used to flash the ESP32-S3-EYE.

## Software Dependencies

All dependencies have been already added to [idf_component.yml](https://github.com/henrytran720/Elegoo-AI-Robot/blob/main/main/idf_component.yml) thanks to Henry, and should be automatically downloaded upon compilation. For reference, here are all the dependencies this project requires:

- [Espressif TinyUSB fork](https://components.espressif.com/components/espressif/tinyusb)
- [Espressif's additions to TinyUSB](https://components.espressif.com/components/espressif/esp_tinyusb)
- [USB Host CDC-ACM Class Drive](https://components.espressif.com/components/espressif/usb_host_cdc_acm/versions/2.0.3)
- [Virtual COM Port Service](https://components.espressif.com/components/espressif/usb_host_vcp)
- [ESP32 Camera Driver](https://components.espressif.com/components/espressif/esp32-camera/)
- [TensorFlow Lite Micro for Espressif Chipsets](https://components.espressif.com/components/espressif/esp-tflite-micro/)
- [bertmelis' USBHostSerial](https://github.com/bertmelis/USBHostSerial)
- Serial device drivers (Technically you only need one, but to save time, all three have been included):
  - [CH34x USB-UART converter driver](https://components.espressif.com/components/espressif/usb_host_ch34x_vcp/versions/2.0.0)
  - [Silicon Labs CP210x USB-UART converter driver](https://components.espressif.com/components/espressif/usb_host_cp210x_vcp/versions/2.0.0)
  - [FTDI UART-USB converters driver](https://components.espressif.com/components/espressif/usb_host_ftdi_vcp/versions/2.0.0)

---

# Getting Started

Getting started involves following some instructions detailed in [Henry Tran's repository](https://github.com/henrytran720/Elegoo-AI-Robot). After completing the steps listed in [The basics](https://github.com/henrytran720/Elegoo-AI-Robot?tab=readme-ov-file#the-basics) and [Assembly](https://github.com/henrytran720/Elegoo-AI-Robot?tab=readme-ov-file#assembly), you are ready to proceed to setup.

## Setup (WIP)

The setup process is very similar to Henry's. 

## Vision/Voice Recognition (WIP)

This project utilizes Google's Teachable Machine to train vision and voice recognition models, exported as TensorFlow Lite (.tflite) files. To train your own models, visit Google's [Teachable Machine website](https://teachablemachine.withgoogle.com/).

## LiteRT Optimization (WIP)

Now that the pre-trained models have been deployed onto the ESP32-S3-EYE module, we are ready to optimize them utilizing LiteRT for improved AI inference speed. The goal is to ensure the models work effectively under the constraints of the ESP32-S3-EYE, providing low-latency efficient responses to visual and auditory inputs (e.g., face detection, voice commands). 

## Fine-tuning & Testing AI Capabilities (WIP)

The ESP32-S3-EYE module is now ready to be fine-tuned and tested. The goal is to verify the system's responsiveness and accuracy in various environments, in order to improve the car's interactivity and functionality. Since Teachable Machine doesn't support direct fine-tuning, you may start by re-training the model with new or more diverse data.

## Research Symposium Focus (WIP)

For the research portion of this project, I will analyze our results from the Fine-tuning & Testing AI Capabilities section. Hypothetically, LiteRT will reduce inference latency and improve responsiveness. This may be verified by deploying models without LiteRT optimization and comparing the results.

---

## License

TensorFlow and Espressif's sample code is covered by the Apache 2.0 license.

bertmelis' USBHostSerial code is covered by the MIT license.

Modifications made to both codebases for this project are also covered by the included MIT license.

## Credits

- This project is based on Henry Tran's [Elegoo-AI-Robot](https://github.com/henrytran720/Elegoo-AI-Robot). Special thanks to **Henry Tran** for his work on the original code.
- Thanks to **bertmelis** for the [USBHostSerial code](https://github.com/bertmelis/USBHostSerial).
- Thanks to **Espressif** for the [ESP32 SDK](https://github.com/espressif/esp-idf).
