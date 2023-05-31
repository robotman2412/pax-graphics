# PAX graphics documentation

The PAX graphics stack is being developed for the [MCH2022 badge](https://bodge.team/docs/badges/mch2022/).
It's goal is to allow anyone to, in C and/or C++, use a powerful list of drawing features with good optimisation.

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



# Mixing C and C++ APIs

It is possible to use *both* the C API and C++ API at the same time.

The C++ API accepts types from the C API where relevant.
Some other types can be implicitly converted.


## `pax_*vec2f` and related types

These types have aliased names (`pax::Vec2f` for example) and added operators.

They are fully backwards compatible and can thus be constructed in C++ and passed to C API functions.


## `pax_buf_t`

Most C api functions use a `pax_buf_t *` parameter.

There is an implicit conversion to `pax::Buffer`, the drawing API made of member functions.
This object becomes a **non-owning** reference (C-style `pax_buf_t` not destroyed when C++ `pax::Buffer` is destroyed).

Example:
```c++
extern pax_buf_t *graphics;
void myFunction() {
	// Simple conversion that doesn't do any allocation.
	pax::Buffer betterGraphics(graphics);
	
	// Example C++ API usage.
	betterGraphics.drawCircle(10, 10, 50);
}
```

The opposite conversion direction is also possible: the function `pax::Buffer::getInternal` returns a **non-owning** pointer to the C-style `pax_buf_t`.

Example:
```c++
extern pax::Buffer betterGraphics;
void myFunction() {
	// Simple conversion that doesn't do any allocation.
	pax_buf_t *graphics = betterGraphics.getInternal();
	
	// Example C API usage.
	pax_draw_circle(graphics, 0xffff0000, 10, 10, 50);
}
```

## `pax_font_t *`

The C++ api currently uses this type directly.
If this ever changes, there will be an implicit conversion to the C++ type.



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


