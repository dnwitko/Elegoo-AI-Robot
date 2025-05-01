# Elegoo-AI-Robot: Voice Control with an ESP32-S3-EYE

This project references the original [Elegoo-AI-Robot](https://github.com/henrytran720/Elegoo-AI-Robot) repository by **@henrytran720**. It utilizes the [`micro_speech`](https://github.com/espressif/esp-tflite-micro/tree/master/examples/micro_speech) TensorFlow Lite Micro example as a base, which integrates voice command recognition using a TensorFlow Lite Micro model, deployed on an ESP32-S3-EYE microcontroller, to move a Elegoo robot car via serial commands.

This repository represents work undertaken for the 2025 University Research Symposium.

*Sections marked (WIP - Work In Progress) are subject to change.*

## How it Works

1.  **Voice Commands:** The ESP32-S3-EYE uses its microphone and a TensorFlow Lite Micro audio classification model to recognize spoken commands (e.g., "yes", "no").
2.  **Control:** Recognized voice commands are translated into specific JSON-formatted serial commands (e.g., `{"H":"Elegoo","N":1,"D1":0,"D2":50,"D3":1}`).
3.  **Actuation:** These serial commands are sent via a Micro USB-to-UART serial adapter to the Elegoo robot car's microcontroller, causing the robot to move accordingly.

### Hardware Requirements

- [Elegoo Smart Car Robot](https://us.elegoo.com/products/elegoo-smart-robot-car-kit-v-4-0) - The robot platform.
- [ESP32-S3-EYE Dev Board](https://www.aliexpress.us/item/3256803794751194.html) - The microcontroller performing sensing and ML inference.
- [CP2102 Micro USB to UART Converter](https://www.amazon.com/HiLetgo-CP2102-Module-Converter-Replace/dp/B01N47LXRA) - Bridges USB Host on ESP32 to Robot's UART.
- [JST XH to Dupont Connector Kit](https://www.amazon.com/Kidisoii-Dupont2-54-Connector-Pre-Crimped-Compatible/dp/B0CMCN9CXD/135-4941321-1839956) - For fabricating the ESP32 microUSB -> Robot UART cable.
- [Micro USB to Micro USB Male-to-Male OTG Cable](https://www.amazon.com/Micro-USB-Male-Data-Cable/dp/B0872GMD7V/) - Connects the ESP32-S3-EYE to the CP2102x serial adapter.
- [USB to Micro USB Data Transfer Cable](https://www.amazon.com/FEMORO-Transfer-Charging-Smartphone-Bluetooth/dp/B0D2KZQR8T) - For flashing the ESP32-S3-EYE from your computer.

### Software Dependencies

All dependencies have been already added to [idf_component.yml](https://github.com/dnwitko/Elegoo-AI-Robot/blob/main/main/idf_component.yml) thanks to Henry, and should be automatically downloaded upon compilation. For reference, here are all the dependencies this project requires:

- [ESP-IDF: Version v5.3.2](https://docs.espressif.com/projects/esp-idf/en/stable/esp32s3/get-started/index.html#manual-installation) (or your specific version) is required.
- [espressif/esp-tflite-micro](https://components.espressif.com/components/espressif/esp-tflite-micro): TensorFlow Lite Micro library optimized for Espressif chips (includes ESP-NN integration).
- [Espressif TinyUSB fork](https://components.espressif.com/components/espressif/tinyusb)
- [Espressif's additions to TinyUSB](https://components.espressif.com/components/espressif/esp_tinyusb)
- [USB Host CDC-ACM Class Driver](https://components.espressif.com/components/espressif/usb_host_cdc_acm/versions/2.0.3)
- [Virtual COM Port Service](https://components.espressif.com/components/espressif/usb_host_vcp)
- [bertmelis' USBHostSerial](https://github.com/bertmelis/USBHostSerial)
- Serial device drivers (Technically, you only need one, but to save time, all three have been included):
  - [CH34x USB-UART converter driver](https://components.espressif.com/components/espressif/usb_host_ch34x_vcp/versions/2.0.0)
  - [Silicon Labs CP210x USB-UART converter driver](https://components.espressif.com/components/espressif/usb_host_cp210x_vcp/versions/2.0.0)
  - [FTDI UART-USB converters driver](https://components.espressif.com/components/espressif/usb_host_ftdi_vcp/versions/2.0.0)

---

# Getting Started

Getting started involves following some instructions detailed in [Henry Tran's repository](https://github.com/henrytran720/Elegoo-AI-Robot) to assemble and prepare the Elegoo Robot car. After completing the steps listed in [The basics](https://github.com/henrytran720/Elegoo-AI-Robot?tab=readme-ov-file#the-basics) and [Assembly](https://github.com/henrytran720/Elegoo-AI-Robot?tab=readme-ov-file#assembly), you are ready to proceed to setup.

## Setup and Flashing

The setup process is very similar to Henry's. Start by opening your ESP-IDF environment, and cloning my repository:

```bash
git clone https://github.com/dnwitko/Elegoo-AI-Robot.git
```

Then, change into the project directory, and set your target:

```bash
cd Elegoo-AI-Robot
# This will set up the project to be compiled for the ESP32-S3.
# If you're compiling this project for another platform, please set the target appropriately for your environment (idf.py list-targets).
idf.py set-target esp32s3
```

Once the project finishes building, you are now ready to flash the project to the ESP32. Connect the ESP32-S3-EYE to your computer using the **data transfer** Micro USB cable. Run the flash command (this will build, then flash):

```bash
idf.py flash
# (Optional) You may choose a port to flash to by running: idf.py flash -p [YOUR-ESP32-PORT]
# (e.g., idf.py flash -p COM4 on Windows, or /dev/ttyS4 on Linux/WSL)
```

Once the flashing process completes, you can unplug the ESP32 and plug it in to the serial adapter using the Micro USB to Micro USB OTG cable. This is the completed flashing and setup process. You may now turn on the car and see if it works. Say "yes", and the car will drive forwards. Say "no" and the car will stop. These voice commands and responses to commands may be expanded on or changed, but it will require core project file modifications to function.

---

# Troubleshooting Known Issues (WIP)

This project ran into many issues throughout development. The current state is NOT a working build. Below is a list of known issues, and how to troubleshoot them.

## 1. Debugging Limitations Due to USB Port Usage

**Issue:**
Standard real-time serial debugging (viewing ESP_LOG output via USB CDC) is unavailable during normal operation. The ESP32-S3-EYE's USB port is configured in Host mode to communicate with the CP2102x serial adapter, precluding its use as a Device (CDC) for PC monitoring. Attempting to run `idf.py monitor` while connected to a PC results in a vague `ClearCommError` message:
```bash
Error: ClearCommError failed (PermissionError(13, 'The device does not recognize the command.', None, 22))
```
There is NO known fix for this, and there are NO known workarounds. Do NOT attempt to monitor the serial output after the device initializes USBHost mode. ONLY flash the code using `idf.py flash`. While this is quite a limiting error, encountering it will NOT break anything, you will simply be unable to debug the program. The hardware configuration requires the USB port to be used for host functionality. Simultaneous device-mode debugging is IMPOSSIBLE without additional hardware (e.g., UART adapter on separate pins). You may attempt to debug using alternative methods, like visual feedback or inferring the state from behavior, with varied results.

## 2. OpResolver Template Argument Missing

**Issue:** A `class template argument deduction failed` error occurred on the `tflite::MicroMutableOpResolver` declaration. Sometimes, the required template argument specifying the number of operators to be registered can be missing, if not added in manually:
```bash
In function 'void setup()':
error: class template argument deduction failed:
  98 |   static tflite::MicroMutableOpResolver micro_op_resolver;
     |                                         ^~~~~~~~~~~~~~~~~

note:   candidate expects 1 argument, 0 provided
```
This issue can be fixed by finding the line in `main/main_functions.cc` that reads: `static tflite::MicroMutableOpResolver micro_op_resolver;`

And changing it to: `static tflite::MicroMutableOpResolver<4> micro_op_resolver;` (or however many operators your model uses).

## 3. Interpreter Allocation Failure

**Issue:** The program logs `AllocateTensors() failed` during setup:
```bash
... (previous boot messages) ...
I (150) main_task: App framework initialized.
I (160) main_task: USB Host Serial Initialized at 115200 baud.
I (1170) main_task: Starting Main Loop
I (1180) main_functions: Attempting to allocate tensors...
E (1180) MicroML: AllocateTensors() failed
E (1190) main_task: Failed to initialize TensorFlow Lite Micro. Halting.
```
This issue can be fixed by checking the value of `kTensorArenaSize` in `main_functions.cc`. The default `micro_speech` model requires roughly 10-15 KB, but custom models or additional operations might need more. Verify is PSRAM is disabled in `idf.py menuconfig`. Check the list of operators added to `TFLMOpResolver` in `main_functions.cc`. If an operation required by the model is missing, allocation can fail. Add the operation to the list and modify the `<>` value in the declaration line. 

## 4. Model Input Tensor Mismatch

**Issue:** The program logs `Bad input tensor parameters in model` during setup, or inference produces nonsense results. 

This issue can be fixed by verifying the expected input tensor shape, size, and type (`kTfLiteInt8`) in `main_functions.cc::setup()` against the model's requirements. Check the constants `kFeatureSliceCount` and `kFeatureSliceSize`. Ensure `feature_provider` is generating features that match these dimensions.

## 5. No Audio Data / Failed Initialization

**Issue:** The `PopulationFeatureData` function in `feature_provider.cc` consistently receives no new slices, or the application logs errors related to I2S initialization:
```bash
... (previous boot messages) ...
I (150) main_task: App framework initialized.
I (160) main_task: USB Host Serial Initialized at 115200 baud.
I (1170) main_task: Starting Main Loop
I (1180) AUDIO_PROVIDER: Initializing I2S audio...
I (1180) AUDIO_PROVIDER: I2S Port: 0
I (1190) AUDIO_PROVIDER: I2S Pins: BCK=5, WS=6, DIN=7  (Example pins)
E (1190) i2s: i2s_driver_install(110): I2S port 0 driver install failed
E (1200) AUDIO_PROVIDER: Failed to install I2S driver (ESP_FAIL: -1)
E (1200) main_functions: Feature generation failed due to audio init error
E (1210) main_task: Error during TFLM loop setup or first run. Halting or restarting...
```
This issue can be fixed by checking the I2S pin definitions within `audio_provider.cc` against the [ESP32-S3-EYE schematic/datasheet](https://dl.espressif.com/dl/schematics/SCH_ESP32-S3-EYE-MB_20211201_V2.2.pdf) for the onboard microphone (BCK, WS/LRCLK, DIN/SDIN).

## 6. Build Fails After Adding USB Host Dependencies (C++ Exceptions)

**Issue:** After adding the USB Host dependencies (from Henry's original project) to this project's `idf_component.yml` and adding `USBHostSerial.cpp`/`.h`, the build failed while compiling the USB VCP components.
```bash
In static member function 'static CdcAcmDevice* esp_usb::VCP::open(uint16_t, uint16_t, const cdc_acm_host_device_config_t*, uint8_t)':
error: exception handling disabled, use '-fexceptions' to enable
   33 |                     } catch (esp_err_t &e) {
      |                                         ^
error: 'e' was not declared in this scope; did you mean 'std::numbers::e'?
   34 |                         switch (e) {
      |                                 ^
      |                                 std::numbers::e
... (similar errors for other catch blocks) ...
ninja: build stopped: subcommand failed.
```
This issue can be fixed by enabling C++ exceptions, which are disabled by default. Run `idf.py menuconfig`, and enable `Compiler Options` -> `Enable C++ Exceptions`. Save the file and run `idf.py build` or `idf.py reconfigure`.

## 7. Linker Error (Windows System Libraries)

**Issue:** The build fails during toolchain configuration/environment setup with a linker error:
```bash
FAILED: cmTC_d61b0.exe
    C:\WINDOWS\system32\cmd.exe /C "cd . && C:\Espressif\tools\esp-clang\16.0.1-fe4f10a809\esp-clang\bin\clang.exe   CMakeFiles/cmTC_d61b0.dir/testCCompiler.c.obj -o cmTC_d61b0.exe -Wl,--out-implib,libcmTC_d61b0.dll.a -Wl,--major-image-version,0,--minor-image-version,0  -lkernel32 -luser32 -lgdi32 -lwinspool -lshell32 -lole32 -loleaut32 -luuid -lcomdlg32 -ladvapi32 && cd ."
    lld: error: unable to find library -lkernel32
    lld: error: unable to find library -luser32
    lld: error: unable to find library -lgdi32
    lld: error: unable to find library -lwinspool
    lld: error: unable to find library -lshell32
    lld: error: unable to find library -lole32
    lld: error: unable to find library -loleaut32
    lld: error: unable to find library -luuid
    lld: error: unable to find library -lcomdlg32
    lld: error: unable to find library -ladvapi32
    lld: error: unable to find library -lmingw32
    lld: error: unable to find library -lunwind
    lld: error: unable to find library -lmoldname
    lld: error: unable to find library -lmingwex
    lld: error: unable to find library -lmsvcrt
    lld: error: unable to find library -ladvapi32
    lld: error: unable to find library -lshell32
    lld: error: unable to find library -luser32
    lld: error: unable to find library -lkernel32
    lld: error: unable to find library -lmingw32
    lld: error: too many errors emitted, stopping now
    clang: error: linker command failed with exit code 1 (use -v to see invocation)
    ninja: build stopped: subcommand failed.
```
This issue can be fixed by verifying your current working directory. If you are in the `C:\Espressif\frameworks\esp-idf-v5.3.2>` directory, you need to navigate into your project directory (`cd Elegoo-AI-Robot`), and try building again. If it still fails, run `idf.py set-target esp32s3` instead of building. 

## 7.1. Linker Error (Undefined Reference)

**Issue:** The build failed during linking with an `undefined reference to 'usbSerial'` error when processing `command_responder.cc.obj`:
```bash
FAILED: micro_speech.elf
C:\WINDOWS\system32\cmd.exe /C "cd . && C:\Espressif\tools\xtensa-esp-elf\esp-13.2.0_20240530\xtensa-esp-elf\bin\xtensa-esp32s3-elf-g++.exe ... @CMakeFiles\micro_speech.elf.rsp -o micro_speech.elf && cd ."
C:/Espressif/tools/xtensa-esp-elf/esp-13.2.0_20240530/xtensa-esp-elf/bin/../lib/gcc/xtensa-esp-elf/13.2.0/../../../../xtensa-esp-elf/bin/ld.exe: esp-idf/main/libmain.a(command_responder.cc.obj):(.literal._Z16RespondToCommandlPKcfb+0x4): undefined reference to `usbSerial'
collect2.exe: error: ld returned 1 exit status
ninja: build stopped: subcommand failed.
```
This issue can be fixed by moving the `UBSHostSerial usbSerial;` declaration outside the anonymous namespace in `main_functions.cc`, to give it external linkage.

## 8. Fullclean Error

**Issue:** When attempting to clean build files and managed components from the project directory, you may encounter this fullclean error (or similar):
```bash
C:\Espressif\frameworks\esp-idf-v5.3.2\Elegoo-AI-Robot> idf.py fullclean
Executing action: fullclean
Executing action: remove_managed_components
ERROR: Some components (espressif__tinyusb) in the "managed_components" directory were modified on the disk since the last run of the CMake. Content of this directory is managed automatically.
If you want to keep the changes, you can move the directory with the component to the "components"directory of your project.
```
This issue can be fixed by manually deleting the `build` and `managed_components` folders from your project, and running `idf.py fullclean` once again.

---

## Potential Issues

**Task Starvation/Timing:** The USB Host task might still be preempted or delayed by higher-priority or CPU-intensive audio processing and inference tasks, leading to failed USB transmissions.

**Subtle Differences:** Unidentified differences in task priorities, stack sizes, core affinities, or other `sdkconfig` parameters between this project and Henry's could be causes for failure.

**Other Possibilities:** Failure during CDC control transfers (e.g., setting line coding), subtle bugs in the `USBHostSerial` wrapper, and intermittent hardware issues could be other causes for failure.

## Future Work / Debugging Steps

1. Utilize alternative debugging (second serial adapter) to visualize task execution and USB timings.
2. Simplify the project (remove audio/TFLM temporarily) to test USB communication in isolation.
3. Conduct a line-by-line comparison of the `USBHostSerial` wrapper against official ESP-IDF USB Host examples.
4. Analyze differences between the full `sdkconfig` files from both projects.

---

## License

TensorFlow and Espressif's sample code is covered by the Apache 2.0 license.

bertmelis' USBHostSerial code is covered by the MIT license.

Modifications made to both codebases for this project are also covered by the included MIT license.

## Credits

This project is based on Henry Tran's [Elegoo-AI-Robot](https://github.com/henrytran720/Elegoo-AI-Robot). Special thanks to **Henry Tran** for his work on the original code.

Thanks to **bertmelis** for the [USBHostSerial code](https://github.com/bertmelis/USBHostSerial).

Thanks to **Espressif** for the [ESP32 SDK](https://github.com/espressif/esp-idf).
