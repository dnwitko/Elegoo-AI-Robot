# Elegoo-Smart-Car

This project is a fork of Henry Tran's [Elegoo-AI-Robot](https://github.com/henrytran720/Elegoo-AI-Robot) repository. It deploys low-latency computer vision and voice recognition models on an ESP32-S3-EYE module to control an Elegoo AI-powered robot car. Using Google’s Teachable Machine, the system trains and tests models for recognizing visual and auditory inputs. The ESP32-S3-EYE microcontroller supports machine learning inference while operating under strict memory and processing constraints. This project utilizes Lite Runtime (LiteRT), a framework which allows machine learning to be deployed on edge devices with small memory and processing power to optimize performance. It is hypothesized that LiteRT will reduce inference latency and improve responsiveness, demonstrating the feasibility of deploying AI on resource-constrained devices for robotics and smart home applications. This is my University Research Symposium Project for 2025.

### Original Sections (from Henry Tran's Repository)
- **Hardware Requirements** (with minor modification)
- **Software Dependencies** (inherited from the original)
- **Getting Started** (partially adapted from the original)

### My Contributions & Modifications
- **Facial/Auditory Recognition** – Added Google's Teachable Machine support. (WIP)
- **LiteRT Optimization** – Integrated Lite Runtime for improved AI inference speed. (WIP)
- **Expanded AI Capabilities** – Enabled both visual and auditory model training/testing. (WIP)
- **Research Symposium Focus** – My analysis on inference latency and responsiveness, created as part of my 2025 University Research Symposium. (WIP)

## Hardware Requirements
- [Elegoo Smart Car Robot](https://us.elegoo.com/products/elegoo-smart-robot-car-kit-v-4-0) - This is a core part of the project: the smart car.
- [ESP32-S3-EYE Dev Board](https://www.aliexpress.us/item/3256803794751194.html) - This is also a core part of the project: the microcontroller.
- [CP2102 Micro USB to UART Converter](https://www.amazon.com/HiLetgo-CP2102-Module-Converter-Replace/dp/B01N47LXRA) - Any serial device will work here, so long as it's compatible with the drivers below.
- [JST XH to Dupont Connector Kit](https://www.amazon.com/Kidisoii-Dupont2-54-Connector-Pre-Crimped-Compatible/dp/B0CMCN9CXD/135-4941321-1839956) - This is used to fabricate a cable for our serial adapter to UART connection.
- [Micro USB to Micro USB Male-to-Male OTG Cable](https://www.amazon.com/Micro-USB-Male-Data-Cable/dp/B0872GMD7V/) - This is used to communicate between the serial adapter and the ESP32-S3-EYE.
- [USB to Micro USB Data Transfer Cable](https://www.amazon.com/FEMORO-Transfer-Charging-Smartphone-Bluetooth/dp/B0D2KZQR8T) - This is used to flash the ESP32-S3-EYE.

## Software Dependencies

All dependencies have been already added to [idf_component.yml](https://github.com/henrytran720/Elegoo-AI-Robot/blob/main/main/idf_component.yml) and should be automatically downloaded upon compile, but for reference, here are all the dependencies this project requires:

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
 
# Getting Started

Getting started involves following instructions detailed in [Henry Tran's repository](https://github.com/henrytran720/Elegoo-AI-Robot). After completing the steps listed in [The basics](https://github.com/henrytran720/Elegoo-AI-Robot?tab=readme-ov-file#the-basics) and [Assembly](https://github.com/henrytran720/Elegoo-AI-Robot?tab=readme-ov-file#assembly), you are ready to proceed to the Facial/Auditory Recognition section.

