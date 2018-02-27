# esp32-weatherman

## Short ESP32 program which grabs the weather information from OpenWeatherMap and displays it onto the HD44780 LCD.

## This is taken and modified from the example project: esp-idf/examples/protocols/http_request

## Description:
The program grabs the weather information on the desired area (using HTTP GET) from OpenWeatherMap.org in the form of JSON. A small C JSON parser (cJSON) inside decodes the JSON and places the weather information and the temperature into temporary buffers.

The said information is printed onto the LCD after taking in the data.

This app deep sleeps for approximately 20 minutes and then wakes up to grab the data from the OpenWeatherMap.

## Connections: 

- SCL -> pin 21 on ESP32
- SDA -> pin 22 on ESP32

![alt tag](https://github.com/uncle-yong/esp32-weatherman/blob/master/i2c%20LCD.PNG)

## Requirements:
- ESP32-IDF
- Visual Studio Code
- I2C LCD module (see the picture)
- Level shifter module (because the LCD module is +5V and the ESP32 is only working at +3.3V)

## For more information of installing the toolchain w/ Visual Studio code, proceed to the hackaday page:

1. https://hackaday.io/project/53259-deploying-standalone-esp32-idf-sdk-platform (without PlatformIO)
2. https://hackaday.io/project/43374-esp32-idf-deploying-the-development-platform (with PlatformIO)

Microsecond delay code is from: https://github.com/espressif/arduino-esp32

## Extra notes:
1. Please get your own OpenWeatherMap account - take the free one first. 
2. Do not repeatedly restart the app if you are connected to your free account - there is a limit to how many times you can retrieve the information per hour!
3. If you have to test your app, use the OpenWeatherMap sample first!
