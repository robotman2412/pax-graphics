# PAX Docs: Supported platforms

This page is about support for various hardware and software platforms.

I have created some overviews of supported platforms:
 - [Quick overview of supported platforms](#a-quick-overview)
 - [List of unsupported platforms](#list-of-unsupported-platforms)
 - [More detailed information](#more-detailed-information)
   - [ESP32](#details-esp32)
   - [Raspberry Pi Pico](#details-pi-pico)
 - [Why certain platforms are unsupported](#why-is-it-unsupported)
   - [Why ESP8266 is unsupported](#unsupported-esp8266)
   - [Why Arduino Uno/Nano are unsupported](#unsupported-arduino-sdk)



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
| Arduino SDK      | N/A            | Build system is too restricting



# More detailed information
About certain platforms support level.


## Details: ESP32
The original ESP32 is the original target platform for PAX.
PAX was made for the MCH2022 badge, for which the ESP32 was used as the main processor.

PAX can work as an ESP-IDF component, so simply clone it in to your `components` folder.


## Details: Pi Pico
Due both to request and personal projects, I am currently working on support for the Raspberry Pi Pico.
In its current state, all this does is compile without MCR support.

Due to the more "raw CMake" nature of the Pico SDK, it's slightly more complicated.
TL;DR: Clone it and link it with `add_subdirectory` and `target_link_libraries`.

But an actual explanation is better, so:

### 1. Clone PAX:
Just put in your project folder, next to your `CMakeLists.txt`.
```sh
git clone https://github.com/robotman2412/pax-graphics
```

### 2. Link it to your project:
Add to your `CMakeLists.txt`, after `target_include_directories`:
```cmake
# This tells CMake to build PAX for us.
add_subdirectory(pax-graphics)

# This tells CMake we would like to use PAX as a library.
target_link_libraries(your_project_name pax_graphics)
```

### 3. Profit!
Raspberry Pi Pico support is currently in beta.



# Why is it unsupported?
More detail about why certain platforms are unsupported.
This is only about the more popular microcontrollers.


## Unsupported: ESP8266
The ESP8266 is technically powerful enough to run PAX, but PAX supports ESP-IDF and the ESP8266 does not use ESP-IDF.
With enough requests, I could theoretically add support for ESP8266.


## Unsupported: Arduino SDK
The Arduino SDK is unsupported because its build system is too limiting.
Thing the Arduino build system can't do include setting include directories, explicitly listing source files to compile (Arduino builds all .c and .cpp files without any further consideration), and more.
This is just barely limiting enough that the effort of porting to Arduino is not worth it to me.
