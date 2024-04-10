
// SPDX-License-Identifier: MIT

#ifndef PAX_INTERNAL_H
#define PAX_INTERNAL_H

#include "helpers/pax_precalculated.h"
#include "pax_gfx.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

#if (defined(ESP32) || defined(ESP8266) || defined(ESP_PLATFORM)) && !defined(PAX_ESP_IDF)
#define PAX_ESP_IDF
#endif

#ifdef PAX_ESP_IDF

#ifdef __cplusplus
}
#endif
#include <esp_err.h>
#include <esp_log.h>
#include <esp_system.h>
#include <sdkconfig.h>
#ifdef __cplusplus
extern "C" {
#endif

#define PAX_PERF_CRITICAL_ATTR IRAM_ATTR

#define PAX_LOGE(...) ESP_LOGE(__VA_ARGS__)
#define PAX_LOGI(...) ESP_LOGI(__VA_ARGS__)
#define PAX_LOGW(...) ESP_LOGW(__VA_ARGS__)

#ifdef PAX_ENABLE_DEBUG_LOGS
#define PAX_LOGD(...) ESP_LOGD(__VA_ARGS__)
#else
#define PAX_LOGD(...)
#endif

#elif defined(PAX_STANDALONE)

#define PAX_PERF_CRITICAL_ATTR __attribute__((hot))

#ifdef __cplusplus
}
#endif
#if PAX_COMPILE_MCR
#include <pthread.h>
#endif
#ifdef __cplusplus
extern "C" {
#endif

#if PAX_COMPILE_MCR
extern pthread_mutex_t pax_log_mutex;
extern bool            pax_log_use_mutex;

#define PRIVATE_PAX_LOG_HELPER(file, prefix, tag, ...)                                                                 \
    do {                                                                                                               \
        if (pax_log_use_mutex) {                                                                                       \
            pthread_mutex_lock(&pax_log_mutex);                                                                        \
        }                                                                                                              \
        fprintf(file, prefix "%s: ", (tag));                                                                           \
        fprintf(file, __VA_ARGS__);                                                                                    \
        fputs("\033[0m\r\n", file);                                                                                    \
        if (pax_log_use_mutex) {                                                                                       \
            pthread_mutex_unlock(&pax_log_mutex);                                                                      \
        }                                                                                                              \
    } while (0)
#else
#define PRIVATE_PAX_LOG_HELPER(file, prefix, tag, ...)                                                                 \
    do {                                                                                                               \
        fprintf(file, prefix "%s: ", (tag));                                                                           \
        fprintf(file, __VA_ARGS__);                                                                                    \
        fputs("\033[0m\r\n", file);                                                                                    \
    } while (0)
#endif

#define PAX_LOGE(tag, ...) PRIVATE_PAX_LOG_HELPER(stderr, "\033[91mError ", tag, __VA_ARGS__)
#define PAX_LOGI(tag, ...) PRIVATE_PAX_LOG_HELPER(stdout, "\033[32mInfo  ", tag, __VA_ARGS__)
#define PAX_LOGW(tag, ...) PRIVATE_PAX_LOG_HELPER(stderr, "\033[33mWarn  ", tag, __VA_ARGS__)

#ifdef PAX_ENABLE_DEBUG_LOGS
#define PAX_LOGD(tag, ...) PRIVATE_PAX_LOG_HELPER(stdout, "\033[94mDebug ", tag, __VA_ARGS__)
#else
#define PAX_LOGD(...)                                                                                                  \
    do                                                                                                                 \
        ;                                                                                                              \
    while (0)
#endif

#else

#define PAX_PERF_CRITICAL_ATTR __attribute__((hot))

#define PRIVATE_PAX_LOG_HELPER(file, prefix, tag, ...)                                                                 \
    do {                                                                                                               \
        fprintf(file, prefix "%s: ", (tag));                                                                           \
        fprintf(file, __VA_ARGS__);                                                                                    \
        fputs("\033[0m\r\n", file);                                                                                    \
    } while (0)

#define PAX_LOGE(tag, ...) PRIVATE_PAX_LOG_HELPER(stderr, "\033[91mError ", tag, __VA_ARGS__)
#define PAX_LOGI(tag, ...) PRIVATE_PAX_LOG_HELPER(stdout, "\033[32mInfo  ", tag, __VA_ARGS__)
#define PAX_LOGW(tag, ...) PRIVATE_PAX_LOG_HELPER(stderr, "\033[33mWarn  ", tag, __VA_ARGS__)

#ifdef PAX_ENABLE_DEBUG_LOGS
#define PAX_LOGD(tag, ...) PRIVATE_PAX_LOG_HELPER(stdout, "\033[94mDebug ", tag, __VA_ARGS__)
#else
#define PAX_LOGD(...)
#endif

#endif

/* ======= GENERIC HELPERS ======= */

// Whether multi-core rendering is enabled.
extern bool pax_do_multicore;

// Macros for errors.
#ifdef PAX_AUTOREPORT
#define PAX_ERROR(where, errno)                                                                                        \
    {                                                                                                                  \
        pax_report_error(where, errno);                                                                                \
        pax_last_error = errno;                                                                                        \
        return;                                                                                                        \
    }
#define PAX_ERROR1(where, errno, retval)                                                                               \
    {                                                                                                                  \
        pax_report_error(where, errno);                                                                                \
        pax_last_error = errno;                                                                                        \
        return retval;                                                                                                 \
    }
#else
#define PAX_ERROR(where, errno)                                                                                        \
    {                                                                                                                  \
        pax_last_error = errno;                                                                                        \
        return;                                                                                                        \
    }
#define PAX_ERROR1(where, errno, retval)                                                                               \
    {                                                                                                                  \
        pax_last_error = errno;                                                                                        \
        return retval;                                                                                                 \
    }
#endif

#define PAX_SUCCESS()                                                                                                  \
    { pax_last_error = PAX_OK; }

// Buffer sanity check.
#define PAX_BUF_CHECK(where)                                                                                           \
    {                                                                                                                  \
        if (!(buf) || !(buf)->buf)                                                                                     \
            PAX_ERROR(where, PAX_ERR_NOBUF);                                                                           \
    }
// Buffer sanity check.
#define PAX_BUF_CHECK1(where, retval)                                                                                  \
    {                                                                                                                  \
        if (!(buf) || !(buf)->buf)                                                                                     \
            PAX_ERROR1(where, PAX_ERR_NOBUF, retval);                                                                  \
    }

// Swap two variables.
#define PAX_SWAP(type, a, b)                                                                                           \
    {                                                                                                                  \
        type tmp = a;                                                                                                  \
        a        = b;                                                                                                  \
        b        = tmp;                                                                                                \
    }

// Prints a simple error report of an error code.
void pax_report_error(char const *where, pax_err_t errno);



/* ===== GETTERS AND SETTERS ===== */

// Select a number of divisions for an arc.
int pax_pick_arc_divs(matrix_2d_t const *matrix, float radius, float a0, float a1);
// Select an appropriate precalculated circle.
int pax_pick_circle(matrix_2d_t const *matrix, float radius, pax_vec2f const **vertex, pax_trif const **uv);

// Gets the index getters and setters for the given buffer.
void pax_get_setters(pax_buf_t const *buf, pax_index_getter_t *getter, pax_index_setter_t *setter);

// Gets the most efficient index setter for the occasion.
// Also converts the color, if applicable.
// Returns NULL when setting is not required.
pax_index_setter_t pax_get_setter(pax_buf_t const *buf, pax_col_t *col, pax_shader_t const *shader);

// Gets a raw value from a 1BPP buffer.
pax_col_t pax_index_getter_1bpp(pax_buf_t const *buf, int index);
// Gets a raw value from a 2BPP buffer.
pax_col_t pax_index_getter_2bpp(pax_buf_t const *buf, int index);
// Gets a raw value from a 4BPP buffer.
pax_col_t pax_index_getter_4bpp(pax_buf_t const *buf, int index);
// Gets a raw value from a 8BPP buffer.
pax_col_t pax_index_getter_8bpp(pax_buf_t const *buf, int index);
// Gets a raw value from a 16BPP buffer.
pax_col_t pax_index_getter_16bpp(pax_buf_t const *buf, int index);
// Gets a raw value from a 24BPP buffer.
pax_col_t pax_index_getter_24bpp(pax_buf_t const *buf, int index);
// Gets a raw value from a 32BPP buffer.
pax_col_t pax_index_getter_32bpp(pax_buf_t const *buf, int index);
// Gets a raw value from a 16BPP buffer, reversed endianness.
pax_col_t pax_index_getter_16bpp_rev(pax_buf_t const *buf, int index);
// Gets a raw value from a 24BPP buffer, reversed endianness.
pax_col_t pax_index_getter_24bpp_rev(pax_buf_t const *buf, int index);
// Gets a raw value from a 32BPP buffer, reversed endianness.
pax_col_t pax_index_getter_32bpp_rev(pax_buf_t const *buf, int index);

// Sets a raw value from a 1BPP buffer.
void pax_index_setter_1bpp(pax_buf_t *buf, pax_col_t color, int index);
// Sets a raw value from a 2BPP buffer.
void pax_index_setter_2bpp(pax_buf_t *buf, pax_col_t color, int index);
// Sets a raw value from a 4BPP buffer.
void pax_index_setter_4bpp(pax_buf_t *buf, pax_col_t color, int index);
// Sets a raw value from a 8BPP buffer.
void pax_index_setter_8bpp(pax_buf_t *buf, pax_col_t color, int index);
// Sets a raw value from a 16BPP buffer.
void pax_index_setter_16bpp(pax_buf_t *buf, pax_col_t color, int index);
// Sets a raw value from a 24BPP buffer.
void pax_index_setter_24bpp(pax_buf_t *buf, pax_col_t color, int index);
// Sets a raw value from a 32BPP buffer.
void pax_index_setter_32bpp(pax_buf_t *buf, pax_col_t color, int index);
// Sets a raw value from a 16BPP buffer, reversed endianness.
void pax_index_setter_16bpp_rev(pax_buf_t *buf, pax_col_t color, int index);
// Sets a raw value from a 24BPP buffer, reversed endianness.
void pax_index_setter_24bpp_rev(pax_buf_t *buf, pax_col_t color, int index);
// Sets a raw value from a 32BPP buffer, reversed endianness.
void pax_index_setter_32bpp_rev(pax_buf_t *buf, pax_col_t color, int index);

// Gets based on index instead of coordinates.
// Does no bounds checking nor color conversion.
pax_col_t pax_get_index(pax_buf_t const *buf, int index);

// Gets based on index instead of coordinates.
// Does no bounds checking.
pax_col_t pax_get_index_conv(pax_buf_t const *buf, int index);

// Sets based on index instead of coordinates.
// Does no bounds checking nor color conversion.
void pax_set_index(pax_buf_t *buf, pax_col_t col, int index);

// Sets based on index instead of coordinates.
// Does no bounds checking.
void pax_set_index_conv(pax_buf_t *buf, pax_col_t col, int index);

// Merges based on index instead of coordinates.
// Does no bounds checking.
void pax_merge_index(pax_buf_t *buf, pax_col_t col, int index);



/* ======= COLOR CONVERSION ====== */

// Get the correct color conversion methods for the buffer type.
void pax_get_col_conv(pax_buf_t const *buf, pax_col_conv_t *col2buf, pax_col_conv_t *buf2col);

// Dummy color converter, returns color input directly.
pax_col_t pax_col_conv_dummy(pax_buf_t const *buf, pax_col_t color);

// Truncates input to 1 bit.
pax_col_t pax_trunc_to_1(pax_buf_t const *buf, pax_col_t color);
// Truncates input to 2 bit.
pax_col_t pax_trunc_to_2(pax_buf_t const *buf, pax_col_t color);
// Truncates input to 4 bit.
pax_col_t pax_trunc_to_4(pax_buf_t const *buf, pax_col_t color);
// Truncates input to 8 bit.
pax_col_t pax_trunc_to_8(pax_buf_t const *buf, pax_col_t color);
// Truncates input to 16 bit.
pax_col_t pax_trunc_to_16(pax_buf_t const *buf, pax_col_t color);

// Converts ARGB to 1-bit greyscale (AKA black/white).
pax_col_t pax_col_to_1_grey(pax_buf_t const *buf, pax_col_t color);
// Converts ARGB to 2-bit greyscale.
pax_col_t pax_col_to_2_grey(pax_buf_t const *buf, pax_col_t color);
// Converts ARGB to 4-bit greyscale.
pax_col_t pax_col_to_4_grey(pax_buf_t const *buf, pax_col_t color);
// Converts ARGB to 8-bit greyscale.
pax_col_t pax_col_to_8_grey(pax_buf_t const *buf, pax_col_t color);

// Converts ARGB to 3, 3, 2 bit RGB.
pax_col_t pax_col_to_332_rgb(pax_buf_t const *buf, pax_col_t color);
// Converts ARGB to 5, 6, 5 bit RGB.
pax_col_t pax_col_to_565_rgb(pax_buf_t const *buf, pax_col_t color);

// Converts ARGB to 1 bit per channel ARGB.
pax_col_t pax_col_to_1111_argb(pax_buf_t const *buf, pax_col_t color);
// Converts ARGB to 2 bit per channel ARGB.
pax_col_t pax_col_to_2222_argb(pax_buf_t const *buf, pax_col_t color);
// Converts ARGB to 4 bit per channel ARGB.
pax_col_t pax_col_to_4444_argb(pax_buf_t const *buf, pax_col_t color);

// Performs a palette lookup based on the input.
pax_col_t pax_pal_lookup(pax_buf_t const *buf, pax_col_t color);

// Converts 1-bit greyscale (AKA black/white) to ARGB.
pax_col_t pax_1_grey_to_col(pax_buf_t const *buf, pax_col_t color);
// Converts 2-bit greyscale to ARGB.
pax_col_t pax_2_grey_to_col(pax_buf_t const *buf, pax_col_t color);
// Converts 4-bit greyscale to ARGB.
pax_col_t pax_4_grey_to_col(pax_buf_t const *buf, pax_col_t color);
// Converts 8-bit greyscale to ARGB.
pax_col_t pax_8_grey_to_col(pax_buf_t const *buf, pax_col_t color);

// Converts 3, 3, 2 bit RGB to ARGB.
pax_col_t pax_332_rgb_to_col(pax_buf_t const *buf, pax_col_t color);
// Converts 5, 6, 5 bit RGB to ARGB.
pax_col_t pax_565_rgb_to_col(pax_buf_t const *buf, pax_col_t color);

// Converts 1 bit per channel ARGB to ARGB.
pax_col_t pax_1111_argb_to_col(pax_buf_t const *buf, pax_col_t color);
// Converts 2 bit per channel ARGB to ARGB.
pax_col_t pax_2222_argb_to_col(pax_buf_t const *buf, pax_col_t color);
// Converts 4 bit per channel ARGB to ARGB.
pax_col_t pax_4444_argb_to_col(pax_buf_t const *buf, pax_col_t color);
// Converts 8 bit per channel RGB to ARGB.
pax_col_t pax_888_rgb_to_col(pax_buf_t const *buf, pax_col_t color);



/* ======= INLINE INTERNAL ======= */

// Determine whether or not to draw given a color.
// Non-palette buffers: Draw if alpha > 0.
// Palette buffers: Draw if color in bounds.
static inline bool pax_do_draw_col(pax_buf_t const *buf, pax_col_t col) {
    if (PAX_IS_PALETTE(buf->type)) {
        return col < buf->palette_size;
    } else {
        return col & 0xff000000;
    }
}

// A linear interpolation based only on ints.
static inline uint8_t pax_lerp(uint8_t part, uint8_t from, uint8_t to) {
    // This funny line converts part from 0-255 to 0-256.
    // Then, it applies an integer multiplication and the result is shifted right by 8.
    return from + (((to - from) * (part + (part >> 7))) >> 8);
}

// UV interpolation helper for the circle methods.
static inline float pax_flerp4(float x, float y, float e0, float e1, float e2, float e3) {
    x       = x * 0.5 + 0.5;
    y       = y * -0.5 + 0.5;
    float a = e0 + (e1 - e0) * x;
    float b = e2 + (e3 - e2) * x;
    return a + (b - a) * y;
}

// Reverse endianness for 16-bit things.
static inline uint16_t pax_rev_endian_16(uint16_t in) {
    return (in >> 8) | (in << 8);
}

// Reverse endianness for 24-bit things.
static inline uint32_t pax_rev_endian_24(uint32_t in) {
    return ((in >> 16) & 0x00000ff) | (in & 0x00ff00) | ((in << 16) & 0xff0000);
}

// Reverse endianness for 32-bit things.
static inline uint32_t pax_rev_endian_32(uint32_t in) {
    return (in >> 24) | ((in >> 8) & 0x0000ff00) | ((in << 8) & 0x00ff0000) | (in << 24);
}



/* ======= DRAWING HELPERS ======= */

// Gets the correct callback function for the shader.
pax_shader_ctx_t pax_get_shader_ctx(pax_buf_t *buf, pax_col_t color, pax_shader_t const *shader);

// The scheduler for multicore rendering.
void paxmcr_add_task(pax_task_t *task);

// Multi-core method for shaded triangles.
// Assumes points are sorted by Y.
// If odd_scanline is true, the odd (counted from 0) lines are drawn, otherwise the even lines are drawn.
void paxmcr_tri_shaded(
    bool                odd_scanline,
    pax_buf_t          *buf,
    pax_col_t           color,
    pax_shader_t const *shader,
    float               x0,
    float               y0,
    float               x1,
    float               y1,
    float               x2,
    float               y2,
    float               u0,
    float               v0,
    float               u1,
    float               v1,
    float               u2,
    float               v2
);

//  Multi-core optimisation which maps a buffer directly onto another.
// If odd_scanline is true, the odd (counted from 0) lines are drawn, otherwise the even lines are drawn.
void paxmcr_overlay_buffer(
    bool odd_scanline, pax_buf_t *base, pax_buf_t *top, int x, int y, int width, int height, bool assume_opaque
);

//  Multi-core  method for shaded rects.
// If odd_scanline is true, the odd (counted from 0) lines are drawn, otherwise the even lines are drawn.
void paxmcr_rect_shaded(
    bool                odd_scanline,
    pax_buf_t          *buf,
    pax_col_t           color,
    pax_shader_t const *shader,
    float               x,
    float               y,
    float               width,
    float               height,
    float               u0,
    float               v0,
    float               u1,
    float               v1,
    float               u2,
    float               v2,
    float               u3,
    float               v3
);



// Multi-core method for unshaded triangles.
// Assumes points are sorted by Y.
// If odd_scanline is true, the odd (counted from 0) lines are drawn, otherwise the even lines are drawn.
void paxmcr_tri_unshaded(
    bool odd_scanline, pax_buf_t *buf, pax_col_t color, float x0, float y0, float x1, float y1, float x2, float y2
);

// Multi-core method for rectangle drawing.
// If odd_scanline is true, the odd (counted from 0) lines are drawn, otherwise the even lines are drawn.
void paxmcr_rect_unshaded(
    bool odd_scanline, pax_buf_t *buf, pax_col_t color, float x, float y, float width, float height
);



// Internal method for shaded triangles.
// Assumes points are sorted by Y.
void pax_tri_shaded(
    pax_buf_t          *buf,
    pax_col_t           color,
    pax_shader_t const *shader,
    float               x0,
    float               y0,
    float               x1,
    float               y1,
    float               x2,
    float               y2,
    float               u0,
    float               v0,
    float               u1,
    float               v1,
    float               u2,
    float               v2
);

// Optimisation which maps a buffer directly onto another.
// If assume_opaque is true, the overlay is done without transparency.
void pax_overlay_buffer(pax_buf_t *base, pax_buf_t *top, int x, int y, int width, int height, bool assume_opaque);

// Internal method for shaded rects.
void pax_rect_shaded(
    pax_buf_t          *buf,
    pax_col_t           color,
    pax_shader_t const *shader,
    float               x,
    float               y,
    float               width,
    float               height,
    float               u0,
    float               v0,
    float               u1,
    float               v1,
    float               u2,
    float               v2,
    float               u3,
    float               v3
);

// Internal method for line drawing.
void pax_line_shaded(
    pax_buf_t          *buf,
    pax_col_t           color,
    pax_shader_t const *shader,
    float               u0,
    float               v0,
    float               u1,
    float               v1,
    float               x0,
    float               y0,
    float               x1,
    float               y1
);



// Internal method for unshaded triangles.
// Assumes points are sorted by Y.
void pax_tri_unshaded(pax_buf_t *buf, pax_col_t color, float x0, float y0, float x1, float y1, float x2, float y2);

// Internal method for rectangle drawing.
void pax_rect_unshaded(pax_buf_t *buf, pax_col_t color, float x, float y, float width, float height);

// Internal method for line drawing.
void pax_line_unshaded(pax_buf_t *buf, pax_col_t color, float x0, float y0, float x1, float y1);



#ifdef __cplusplus
}
#endif //__cplusplus

#endif // PAX_INTERNAL_H
