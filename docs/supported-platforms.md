# PAX Docs: Supported platforms

This page is about support for various hardware and software platforms.

I have created a overviews of supported platforms:
 - [Quick overview of supported platforms](#a-quick-overview)
 - [List of unsupported platforms](#list-of-unsupported-platforms)
 - [More detailed information](#more-detailed-information)
   - [ESP32](#details-esp32)
   - [Raspberry Pi Pico](#details-pi-pico)
 - [Why certain platforms are unsupported](#why-is-it-unsupported)
   - [Why ESP8266 is unsupported](#unsupported-esp8266)
   - [Why Arduino Uno/Nano are unsupported](#unsupported-arduino-uno-nano)



# A quick overview   
| platform | support level          | Has MCR? | Notes
| :------- | :--------------------- | :------- | :----
| ESP32    | Full                   | Yes      | Original target platform.
| ESP32-*  | Untested               | Yes      | Theoretically supported but untested.
| Pi Pico  | Partial                | No       | Support currently in development.



# List of unsupported platforms.
This is a list of platforms that are known not to be powerful enough to run PAX.

| platform         | processor      | reason
| :--------------- | :------------- | :-----
| ESP8266          | Tensilica L106 | Powerful enough but doesn't use ESP-IDF
| Arduino Nano/Uno | ATMega/ATTiny  | Too slow, not enough memory



# More detailed information
About certain platforms support level.


## Details: ESP32
The original ESP32 is the original target platform for PAX.
PAX was made for the MCH2022 badge, for which the ESP32 was used as the main processor.

PAX can work as an ESP-IDF component, so simply clone it in to your `components` folder.


## Details: Pi Pico
Due both to request and personal projects, I am currently working on support for the Raspberry Pi Pico.
In its current state, all this does is compile without MCR support for ARM Cortex-M0+ devices.

TL;DR: Clone it, build it with `make -f Pi_Pico.mk` and link `build/pax_graphics` in your `CMakeLists.txt`.

But an actual explanation is better, so:

### 1. Clone PAX:
Just put in your project folder, next to your `CMakeLists.txt`.
```sh
git clone https://github.com/robotman2412/pax-graphics
```

### 2. Build PAX:
This step needs to be repeated on every update, I recommend adding it to your build script somewhere.

Assuming you have some sort of build script or Makefile, run:
```sh
make -C pax-graphics -f Pi_Pico.mk
```

### 3. Link it to your project:
Add to your `CMakeLists.txt`, after `target_include_directories`:
```cmake
# Add any user requested libraries
target_link_libraries(your_project_name ${CMAKE_CURRENT_LIST_DIR}/pax-graphics/build/pax_graphics)
```


# Why is it unsupported?
More detail about why certain platforms are unsupported.
This is only about the more popular microcontrollers.


## Unsupported: ESP8266
The ESP8266 is technically powerful enough to run PAX, but PAX supports ESP-IDF and the ESP8266 does not.
With enough requests, I could theoretically add support for ESP8266.


## Unsupported: Arduino Uno/Nano
Arduino Uno and Arduino Nano, specifically the AVR-based ones.
This is because they do not have enough memory to support PAX.
And although speed isn't technically preventing it, the AVR arduinos are usually about 16MHz, which is quite slow for software rendering.
