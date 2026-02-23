This program acts as a BTC ticker for a 64x32 LED Matrix, and uses the following libraries:

#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <math.h>

The program was compiled, installed, and libraries linked via Arduino IDE

For wiring guide, please see Hub75 Documentation.

To use, modify the main file to populate wifi SSID and password for BTC price API to fetch correctly.

Please note: I had to Downgrade to version 2.0.14 of esp32 by Espressif Systems for compatibility.

Also note, first time board users may need to install CP210x_Universal_Windows_Driver for board to be picked up properly on IDE