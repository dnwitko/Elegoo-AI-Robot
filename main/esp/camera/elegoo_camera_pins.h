// main/esp/camera/elegoo_camera_pins.h
#ifndef ELEGOO_CAMERA_PINS_H_
#define ELEGOO_CAMERA_PINS_H_

// Pin definitions based on ESP32-S3-EYE mapping provided
// Assuming Elegoo AI Robot uses similar/same pinout for the camera

#define CAMERA_PIN_PWDN    -1  // From PWDN_GPIO_NUM
#define CAMERA_PIN_RESET   -1  // From RESET_GPIO_NUM
#define CAMERA_PIN_XCLK    15  // From XCLK_GPIO_NUM
#define CAMERA_PIN_SIOD    4   // From SIOD_GPIO_NUM (Data for I2C)
#define CAMERA_PIN_SIOC    5   // From SIOC_GPIO_NUM (Clock for I2C)

// Map D0-D7 to Y2-Y9 (Common mapping convention)
#define CAMERA_PIN_D0      11  // From Y2_GPIO_NUM
#define CAMERA_PIN_D1      9   // From Y3_GPIO_NUM
#define CAMERA_PIN_D2      8   // From Y4_GPIO_NUM
#define CAMERA_PIN_D3      10  // From Y5_GPIO_NUM
#define CAMERA_PIN_D4      12  // From Y6_GPIO_NUM
#define CAMERA_PIN_D5      18  // From Y7_GPIO_NUM
#define CAMERA_PIN_D6      17  // From Y8_GPIO_NUM
#define CAMERA_PIN_D7      16  // From Y9_GPIO_NUM

#define CAMERA_PIN_VSYNC   6   // From VSYNC_GPIO_NUM
#define CAMERA_PIN_HREF    7   // From HREF_GPIO_NUM
#define CAMERA_PIN_PCLK    13  // From PCLK_GPIO_NUM

#endif // ELEGOO_CAMERA_PINS_H_