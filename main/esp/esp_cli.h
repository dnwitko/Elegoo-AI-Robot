// Copyright 2020-2021 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// main/esp/esp_cli.h

#ifndef APP_ESP_CLI_H_ // <<< Added include guard
#define APP_ESP_CLI_H_

#ifdef __cplusplus
extern "C" {
#endif

// Initializes and starts the ESP-IDF command line interface over UART/USB.
int esp_cli_start();

#ifdef __cplusplus
}
#endif

#endif // APP_ESP_CLI_H_