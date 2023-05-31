# PAX graphics documentation

The PAX graphics stack is being developed for the [MCH2022 badge](https://bodge.team/docs/badges/mch2022/).
It's goal is to allow anyone to, in C, use a powerful list of drawing features with good optimisation.

This library is the successor of the revised graphics API for [the old badge.team firmware](https://github.com/badgeteam/ESP32-platform-firmware).

For supported platforms, [see this link](supported-platforms.md).

## Overview
- [For C developers](#for-c-developers)
- [Build system](#build-system)
    - [For ESP32](#build-system-for-esp32)
    - [For Pi Pico](#build-system-for-pi-pico)
- [Image data format](pixelformat.md)



# For C developers

PAX is built on [a C API](c/README.md) which exposes 2D primitives and some simple compound shapes. It features matrix transformations, a clipping rectangle, highly accurate rendering and more.


## Overview

From the [C API overview](c/README.md):
- [Getting started](c/README.md#getting-started)
- [API overview](c/README.md#api-overview)
- [API reference](c/README.md#api-reference)
    - [Setup](c/README.md#api-reference-setup)
    - [Basic drawing](c/README.md#api-reference-basic-drawing)
    - [Colors](c/README.md#api-reference-colors)
    - [Text](c/README.md#api-reference-text)
- [Advanced API reference](c/README.md#api-reference-advanced-features)
    - [Clipping](c/README.md#api-reference-clipping)
    - [Matrix transformations](c/README.md#api-reference-matrix-transformations)
    - [Shaders](c/README.md#api-reference-shaders)



# Build system

In order to support multiple targets and platforms, the build system for PAX has grown somewhat complex.

## Build system: for ESP32

Similarly, PAX can be used directly as an ESP-IDF component when placed in your components folder.

ESP32 is an officially supported platform.
For other supported platforms, [see this link](supported-platforms.md).

## Build system: for Pi Pico

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
target_link_libraries(your_project_name ${CMAKE_CURRENT_LIST_DIR}/pax-graphics/build/pax_graphics)
```

### 3. Profit!
Raspberry Pi Pico support is currently in beta.
For supported platforms, [see this link](supported-platforms.md).


