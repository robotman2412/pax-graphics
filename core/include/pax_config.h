
// SPDX-License-Identifier: MIT

#ifndef PAX_CONFIG_H
#define PAX_CONFIG_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifndef PAX_DO_BICUBIC
    // Perform bicubic interpolation in text and images.
    #define PAX_DO_BICUBIC false
#endif

#ifndef PAX_AUTOREPORT
    // Log errors instead of just setting pax_last_error.
    #define PAX_AUTOREPORT true
#endif

#ifndef PAX_COMPILE_BEZIER
    // Compile in bezier curves.
    #define PAX_COMPILE_BEZIER true
#endif

#ifndef PAX_USE_EXPENSIVE_BEZIER
    // Use the more expensive, but more accurate algorithm in pax_draw_bezier.
    #define PAX_USE_EXPENSIVE_BEZIER false
#endif

#ifndef PAX_COMPILE_TRIANGULATE
    // Compile in triangulation (filling the outline of a shape).
    #define PAX_COMPILE_TRIANGULATE true
#endif

#ifndef PAX_COMPILE_FONT_INDEX
    // Compile in all fonts and the fonts index.
    #define PAX_COMPILE_FONT_INDEX false
#endif

#ifndef PAX_COMPILE_MCR
    // Compile in multi-core rendering.
    #define PAX_COMPILE_MCR false
#endif

#ifndef PAX_RANGE_SETTER
    // Compile in the range setter for faster opaque drawing.
    #define PAX_RANGE_SETTER true
#endif

#ifndef PAX_RANGE_MERGER
    // Compile in the range merger for faster alpha blending.
    // Set to 1 for per-BPP, or 2 for per-color-format.
    #define PAX_RANGE_MERGER 2
#endif

#ifndef PAX_COMPILE_ORIENTATION
    // Compile in buffer orientation settings.
    #define PAX_COMPILE_ORIENTATION true
#endif

#ifndef PAX_QUEUE_SIZE
    // Queue size to use for multi-core rendering.
    #define PAX_QUEUE_SIZE 32
#endif

#ifndef PAX_USE_FIXED_POINT
    // Whether to use fixed-point arithmetic internally.
    #define PAX_USE_FIXED_POINT true
#endif

#if PAX_USE_FIXED_POINT
    #ifndef PAX_USE_LONG_FIXED_POINT
        // Whether to use 64-bit instead of 32-bit fixed-point arithmetic.
        #define PAX_USE_LONG_FIXED_POINT true
    #endif
#endif

#endif // PAX_CONFIG_H
