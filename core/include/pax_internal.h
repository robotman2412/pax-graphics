
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
    #if CONFIG_PAX_COMPILE_ASYNC_RENDERER
        #include <pthread.h>
    #endif
    #ifdef __cplusplus
extern "C" {
    #endif

    #if CONFIG_PAX_COMPILE_ASYNC_RENDERER
extern pthread_mutex_t pax_log_mutex;
extern bool            pax_log_use_mutex;

        #define PRIVATE_PAX_LOG_HELPER(file, prefix, tag, ...)                                                         \
            do {                                                                                                       \
                if (pax_log_use_mutex) {                                                                               \
                    pthread_mutex_lock(&pax_log_mutex);                                                                \
                }                                                                                                      \
                fprintf(file, prefix "%s: ", (tag));                                                                   \
                fprintf(file, __VA_ARGS__);                                                                            \
                fputs("\033[0m\r\n", file);                                                                            \
                if (pax_log_use_mutex) {                                                                               \
                    pthread_mutex_unlock(&pax_log_mutex);                                                              \
                }                                                                                                      \
            } while (0)
    #else
        #define PRIVATE_PAX_LOG_HELPER(file, prefix, tag, ...)                                                         \
            do {                                                                                                       \
                fprintf(file, prefix "%s: ", (tag));                                                                   \
                fprintf(file, __VA_ARGS__);                                                                            \
                fputs("\033[0m\r\n", file);                                                                            \
            } while (0)
    #endif

    #define PAX_LOGE(tag, ...) PRIVATE_PAX_LOG_HELPER(stderr, "\033[91mError ", tag, __VA_ARGS__)
    #define PAX_LOGI(tag, ...) PRIVATE_PAX_LOG_HELPER(stdout, "\033[32mInfo  ", tag, __VA_ARGS__)
    #define PAX_LOGW(tag, ...) PRIVATE_PAX_LOG_HELPER(stderr, "\033[33mWarn  ", tag, __VA_ARGS__)

    #ifdef PAX_ENABLE_DEBUG_LOGS
        #define PAX_LOGD(tag, ...) PRIVATE_PAX_LOG_HELPER(stdout, "\033[94mDebug ", tag, __VA_ARGS__)
    #else
        #define PAX_LOGD(...)                                                                                          \
            do                                                                                                         \
                ;                                                                                                      \
            while (0)
    #endif

#else

    #define PAX_PERF_CRITICAL_ATTR __attribute__((hot))

    #define PRIVATE_PAX_LOG_HELPER(file, prefix, tag, ...)                                                             \
        do {                                                                                                           \
            fprintf(file, prefix "%s: ", (tag));                                                                       \
            fprintf(file, __VA_ARGS__);                                                                                \
            fputs("\033[0m\r\n", file);                                                                                \
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

// Swap two variables.
#define PAX_SWAP(type, a, b)                                                                                           \
    {                                                                                                                  \
        type tmp = a;                                                                                                  \
        a        = b;                                                                                                  \
        b        = tmp;                                                                                                \
    }

// Set error code.
void pax_set_err(pax_err_t ec);

// Set error code to OK.
static inline void pax_set_ok() {
    pax_set_err(PAX_OK);
}

#define PAX_ERROR(ec, ...)                                                                                             \
    do {                                                                                                               \
        pax_set_err(ec);                                                                                               \
        return __VA_ARGS__;                                                                                            \
    } while (0)

#define PAX_BUF_CHECK(var, ...)                                                                                        \
    do {                                                                                                               \
        if (!(var)) {                                                                                                  \
            pax_set_err(PAX_ERR_NOBUF);                                                                                \
            return __VA_ARGS__;                                                                                        \
        } else {                                                                                                       \
            pax_set_ok();                                                                                              \
        }                                                                                                              \
    } while (0)

#define PAX_NULL_CHECK(var, ...)                                                                                       \
    do {                                                                                                               \
        if (!(var)) {                                                                                                  \
            pax_set_err(PAX_ERR_PARAM);                                                                                \
            return __VA_ARGS__;                                                                                        \
        } else {                                                                                                       \
            pax_set_ok();                                                                                              \
        }                                                                                                              \
    } while (0)



/* ===== GETTERS AND SETTERS ===== */

// Gets the index getters and setters for the given buffer.
void pax_get_setters(
    pax_buf_t const    *buf,
    pax_index_getter_t *getter,
    pax_index_setter_t *setter,
    pax_range_setter_t *range_setter,
    pax_range_setter_t *range_merger
);

// Gets the most efficient index setter for the occasion.
// Also converts the color, if applicable.
// Returns NULL when setting is not required.
pax_index_setter_t pax_get_setter(pax_buf_t const *buf, pax_col_t *col, pax_shader_t const *shader);
// Gets the most efficient range setter/merger for the occasion.
// Returns NULL when setting is not required.
pax_range_setter_t pax_get_range_setter(pax_buf_t const *buf, pax_col_t *col);

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

#if CONFIG_PAX_RANGE_SETTER
// Sets a raw value range from a 1BPP buffer.
void pax_range_setter_1bpp(pax_buf_t *buf, pax_col_t color, int index, int count);
// Sets a raw value range from a 2BPP buffer.
void pax_range_setter_2bpp(pax_buf_t *buf, pax_col_t color, int index, int count);
// Sets a raw value range from a 4BPP buffer.
void pax_range_setter_4bpp(pax_buf_t *buf, pax_col_t color, int index, int count);
// Sets a raw value range from a 8BPP buffer.
void pax_range_setter_8bpp(pax_buf_t *buf, pax_col_t color, int index, int count);
// Sets a raw value range from a 16BPP buffer.
void pax_range_setter_16bpp(pax_buf_t *buf, pax_col_t color, int index, int count);
// Sets a raw value range from a 24BPP buffer.
void pax_range_setter_24bpp(pax_buf_t *buf, pax_col_t color, int index, int count);
// Sets a raw value range from a 32BPP buffer.
void pax_range_setter_32bpp(pax_buf_t *buf, pax_col_t color, int index, int count);
// Sets a raw value range from a 16BPP buffer, reversed endianness.
void pax_range_setter_16bpp_rev(pax_buf_t *buf, pax_col_t color, int index, int count);
// Sets a raw value range from a 24BPP buffer, reversed endianness.
void pax_range_setter_24bpp_rev(pax_buf_t *buf, pax_col_t color, int index, int count);
// Sets a raw value range from a 32BPP buffer, reversed endianness.
void pax_range_setter_32bpp_rev(pax_buf_t *buf, pax_col_t color, int index, int count);
#else
// Merges a single 32-bit ARGB color into a range of pixels.
void pax_range_setter_generic(pax_buf_t *buf, pax_col_t color, int index, int count);
#endif

#if CONFIG_PAX_RANGE_MERGER
    #define PAX_DEF_BUF_TYPE_PAL(bpp, name)                                                                            \
        void pax_range_merger_##bpp##_pal(pax_buf_t *buf, pax_col_t color, int index, int count);
    #define PAX_DEF_BUF_TYPE_GREY(bpp, name)                                                                           \
        void pax_range_merger_##bpp##_grey(pax_buf_t *buf, pax_col_t color, int index, int count);
    #define PAX_DEF_BUF_TYPE_ARGB(bpp, a, r, g, b, name)                                                               \
        void pax_range_merger_##a##r##g##b##argb(pax_buf_t *buf, pax_col_t color, int index, int count);
    #define PAX_DEF_BUF_TYPE_RGB(bpp, r, g, b, name)                                                                   \
        void pax_range_merger_##r##g##b##rgb(pax_buf_t *buf, pax_col_t color, int index, int count);
    #include "helpers/pax_buf_type.inc"
#else
// Merges a single 32-bit ARGB color into a range of pixels.
void pax_range_merger_generic(pax_buf_t *buf, pax_col_t color, int index, int count);
#endif


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
pax_col_t pax_col_to_332rgb(pax_buf_t const *buf, pax_col_t color);
// Converts ARGB to 5, 6, 5 bit RGB.
pax_col_t pax_col_to_565rgb(pax_buf_t const *buf, pax_col_t color);

// Converts ARGB to 1 bit per channel ARGB.
pax_col_t pax_col_to_1111argb(pax_buf_t const *buf, pax_col_t color);
// Converts ARGB to 2 bit per channel ARGB.
pax_col_t pax_col_to_2222argb(pax_buf_t const *buf, pax_col_t color);
// Converts ARGB to 4 bit per channel ARGB.
pax_col_t pax_col_to_4444argb(pax_buf_t const *buf, pax_col_t color);

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
pax_col_t pax_332rgb_to_col(pax_buf_t const *buf, pax_col_t color);
// Converts 5, 6, 5 bit RGB to ARGB.
pax_col_t pax_565rgb_to_col(pax_buf_t const *buf, pax_col_t color);

// Converts 1 bit per channel ARGB to ARGB.
pax_col_t pax_1111argb_to_col(pax_buf_t const *buf, pax_col_t color);
// Converts 2 bit per channel ARGB to ARGB.
pax_col_t pax_2222argb_to_col(pax_buf_t const *buf, pax_col_t color);
// Converts 4 bit per channel ARGB to ARGB.
pax_col_t pax_4444argb_to_col(pax_buf_t const *buf, pax_col_t color);
// Converts 8 bit per channel RGB to ARGB.
pax_col_t pax_888rgb_to_col(pax_buf_t const *buf, pax_col_t color);

#define pax_col_to_8888argb pax_col_conv_dummy
#define pax_8888argb_to_col pax_col_conv_dummy
#define pax_col_to_888rgb   pax_col_conv_dummy



/* ======= INLINE INTERNAL ======= */

// Determine whether or not to draw given a color.
// Non-palette buffers: Draw if alpha > 0.
// Palette buffers: Draw if color in bounds.
static inline bool pax_do_draw_col(pax_buf_t const *buf, pax_col_t col) {
    if (buf->type_info.fmt_type == PAX_BUF_SUBTYPE_PALETTE) {
        return col < buf->palette_size;
    } else {
        return col & 0xff000000;
    }
}

// A linear interpolation based only on ints.
static inline uint8_t pax_lerp(uint8_t part, uint8_t from, uint8_t to) __attribute__((always_inline));
static inline uint8_t pax_lerp(uint8_t part, uint8_t from, uint8_t to) {
    // This funny line converts part from 0-255 to 0-256.
    // Then, it applies an integer multiplication and the result is shifted right by 8.
    uint16_t part2 = (part + (part >> 7));
    return from + (((to - from) * part2) >> 8);
}

// A linear interpolation based only on ints.
static inline uint32_t pax_lerp_mask(uint32_t mask, uint8_t part, uint32_t from, uint32_t to)
    __attribute__((always_inline));
static inline uint32_t pax_lerp_mask(uint32_t mask, uint8_t part, uint32_t from, uint32_t to) {
    // This funny line converts part from 0-255 to 0-256.
    // Then, it applies an integer multiplication and the result is shifted right by 8.
    uint64_t part2  = (part + (part >> 7));
    from           &= mask;
    to             &= mask;
    return mask & (from + (((to - from) * part2) >> 8));
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
PAX_PERF_CRITICAL_ATTR static inline uint16_t pax_rev_endian_16(uint16_t in) {
    return (in >> 8) | (in << 8);
}

// Reverse endianness for 24-bit things.
PAX_PERF_CRITICAL_ATTR static inline uint32_t pax_rev_endian_24(uint32_t in) {
    return ((in >> 16) & 0x00000ff) | (in & 0x00ff00) | ((in << 16) & 0xff0000);
}

// Reverse endianness for 32-bit things.
PAX_PERF_CRITICAL_ATTR static inline uint32_t pax_rev_endian_32(uint32_t in) {
    return (in >> 24) | ((in >> 8) & 0x0000ff00) | ((in << 8) & 0x00ff0000) | (in << 24);
}



/* ======= DRAWING HELPERS ======= */

// Gets the correct callback function for the shader.
pax_shader_ctx_t pax_get_shader_ctx(pax_buf_t *buf, pax_col_t color, pax_shader_t const *shader);

// The scheduler for multicore rendering.
void paxmcr_add_task(pax_task_t *task);


#ifdef __cplusplus
}
#endif //__cplusplus

#endif // PAX_INTERNAL_H
