
// SPDX-License-Identifier: MIT

#ifndef PAX_CONFIG_H
#define PAX_CONFIG_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef ESP_PLATFORM
    #include <sdkconfig.h>
#endif

#ifndef CONFIG_PAX_BOUNDS_CHECK
    // Add array index out of bounds checks to all PAX framebuffer access.
    // WARNING: Enabling may significantly degrade performance.
    #define CONFIG_PAX_BOUNDS_CHECK false
#endif

#ifndef CONFIG_PAX_DO_BICUBIC
    // Perform bicubic interpolation in text and images.
    #define CONFIG_PAX_DO_BICUBIC false
#endif

#ifndef CONFIG_PAX_COMPILE_BEZIER
    // Compile in bezier curves support.
    #define CONFIG_PAX_COMPILE_BEZIER true
#endif

#ifndef CONFIG_PAX_USE_EXPENSIVE_BEZIER
    // Use the more expensive, but more accurate algorithm in pax_draw_bezier.
    #define CONFIG_PAX_USE_EXPENSIVE_BEZIER false
#endif

#ifndef CONFIG_PAX_COMPILE_TRIANGULATE
    // Compile in triangulation (filling the outline of a shape).
    #define CONFIG_PAX_COMPILE_TRIANGULATE true
#endif

#ifndef CONFIG_PAX_COMPILE_FONT_INDEX
    // Compile in all fonts and the fonts index.
    #define CONFIG_PAX_COMPILE_FONT_INDEX false
#endif

#ifndef CONFIG_PAX_COMPILE_ESP32P4_PPA_RENDERER
    #if CONFIG_IDF_TARGET_ESP32P4
        // Compile in the ESP32-P4 hybrid renderer.
        #define CONFIG_PAX_COMPILE_ESP32P4_PPA_RENDERER true
    #else
        // Compile in the ESP32-P4 hybrid renderer.
        #define CONFIG_PAX_COMPILE_ESP32P4_PPA_RENDERER false
    #endif
#endif

#ifndef CONFIG_PAX_COMPILE_ASYNC_RENDERER
    #if PAX_COMPILE_ASYNC_RENDERER_NONE
        // Compile in async renderer.
        // Set to 0 to disable, 1 for async single-threaded only, 2 for async single- or multi-threaded.
        #define CONFIG_PAX_COMPILE_ASYNC_RENDERER 0
    #elifdef PAX_COMPILE_ASYNC_RENDERER_SINGLETHREAD
        // Compile in async renderer.
        // Set to 0 to disable, 1 for async single-threaded only, 2 for async single- or multi-threaded.
        #define CONFIG_PAX_COMPILE_ASYNC_RENDERER 1
    #else
        // Compile in async renderer.
        // Set to 0 to disable, 1 for async single-threaded only, 2 for async single- or multi-threaded.
        #define CONFIG_PAX_COMPILE_ASYNC_RENDERER 2
    #endif
#endif

#ifndef CONFIG_PAX_RANGE_SETTER
    // Compile in the range setter for faster opaque drawing.
    #define CONFIG_PAX_RANGE_SETTER true
#endif

#ifndef CONFIG_PAX_RANGE_MERGER
    // Compile in the range merger for faster alpha blending.
    // Creates a function per color format, trading memory for more speed.
    #define CONFIG_PAX_RANGE_MERGER true
#endif

#ifndef CONFIG_PAX_COMPILE_ORIENTATION
    // Compile in buffer orientation settings.
    #define CONFIG_PAX_COMPILE_ORIENTATION true
#endif

#ifndef CONFIG_PAX_QUEUE_SIZE
    // Queue size to use for multi-core rendering.
    #define CONFIG_PAX_QUEUE_SIZE 32
#endif

#ifndef CONFIG_PAX_USE_FIXED_POINT
    // Whether to use fixed-point arithmetic internally.
    #define CONFIG_PAX_USE_FIXED_POINT true
#endif

#if CONFIG_PAX_USE_FIXED_POINT
    #ifndef CONFIG_PAX_USE_LONG_FIXED_POINT
        // Whether to use 64-bit instead of 32-bit fixed-point arithmetic.
        #define CONFIG_PAX_USE_LONG_FIXED_POINT true
    #endif
#endif

#ifndef CONFIG_PAX_TEXT_BUCKET_SIZE
    // How many glyphs can be rendered at once before `pax_join()` is implicitly called.
    // Low numbers may significantly hamper text performance if using asynchronous rendering.
    // Uses 8 or 16 bytes of stack space per glyph for 32-bit and 64-bit systems respectively.
    #define CONFIG_PAX_TEXT_BUCKET_SIZE 32
#endif

#endif // PAX_CONFIG_H
