
// SPDX-License-Identifier: MIT

static char const *TAG = "pax_gfx";

#include "pax_internal.h"
#include "pax_shaders.h"

#include <malloc.h>
#include <math.h>
#include <string.h>

#ifdef PAX_ESP_IDF
    #include <esp_timer.h>
#endif

// The last error reported.
pax_err_t pax_last_error   = PAX_OK;
// Whether multi-core rendering is enabled.
// You should not modify this variable.
bool      pax_do_multicore = false;

#if PAX_COMPILE_MCR

// Whether or not the multicore task is currently busy.
bool multicore_busy = false;

    #ifdef PAX_ESP_IDF

        #include <freertos/FreeRTOS.h>
        #include <freertos/queue.h>
        #include <freertos/task.h>

// The task handle for the main core.
TaskHandle_t  main_handle      = NULL;
// The task handle for the other core.
TaskHandle_t  multicore_handle = NULL;
// The render queue for the other core.
QueueHandle_t queue_handle     = NULL;

    #elif defined(PAX_STANDALONE)

        #include <pthread.h>
        #include <ptq.h>
        #include <unistd.h>

// The thread for multicore helper stuff.
pthread_t       multicore_handle;
// The mutex used to determine IDLE.
pthread_mutex_t multicore_mutex = PTHREAD_MUTEX_INITIALIZER;
// The render queue for the multicore helper.
ptq_queue_t     queue_handle    = NULL;

    #endif
#endif

#if defined(PAX_STANDALONE) && PAX_COMPILE_MCR
pthread_mutex_t pax_log_mutex     = PTHREAD_MUTEX_INITIALIZER;
bool            pax_log_use_mutex = false;
#endif



/* ============ DEBUG ============ */

static __thread pax_err_t last_err = PAX_OK;

// Get the last error reported on this thread.
pax_err_t pax_get_err() {
    return last_err;
}

// Set error code.
void pax_set_err(pax_err_t ec) {
    last_err = ec;
}

// Describe error.
char const *pax_desc_err(pax_err_t error) {
    char const       *unknown = "Unknown error";
    char const *const desc[]  = {
        "Success",
        unknown,
        "No framebuffer",
        "No memory",
        "Invalid parameters",
        "Infinite parameters",
        "Out of bounds",
        "Matrix stack underflow",
        "Out of data",
        "Image decoding error",
        "Unsupported operation",
        "Corrupted buffer",
        "Image encoding error",
    };
    size_t n_desc = sizeof(desc) / sizeof(char *);
    if (error > 0 || -error > n_desc)
        return unknown;
    else
        return desc[-error];
}



/* ======= DRAWING HELPERS ======= */

// Select a number of divisions for an arc.
int pax_pick_arc_divs(matrix_2d_t const *matrix, float r, float a0, float a1) {
    float c_r = r * sqrtf(matrix->a0 * matrix->a0 + matrix->b0 * matrix->b0)
                * sqrtf(matrix->a1 * matrix->a1 + matrix->b1 * matrix->b1);
    int n_div;
    if (c_r > 30) {
        n_div = (a1 - a0) / M_PI * 24;
    } else if (c_r > 7) {
        n_div = (a1 - a0) / M_PI * 16;
    } else {
        n_div = (a1 - a0) / M_PI * 8;
    }
    return n_div <= 1 ? 1 : n_div;
}

// Select an appropriate precalculated circle.
int pax_pick_circle(matrix_2d_t const *matrix, float r, pax_vec2f const **vertex, pax_trif const **uv) {
    float c_r = r * sqrtf(matrix->a0 * matrix->a0 + matrix->b0 * matrix->b0)
                * sqrtf(matrix->a1 * matrix->a1 + matrix->b1 * matrix->b1);
    if (c_r > 30) {
        *vertex = pax_precalc_circle_24;
        *uv     = pax_precalc_uv_circle_24;
        return 24;
    } else if (c_r > 7) {
        *vertex = pax_precalc_circle_16;
        *uv     = pax_precalc_uv_circle_16;
        return 16;
    } else {
        *vertex = pax_precalc_circle_8;
        *uv     = pax_precalc_uv_circle_8;
        return 8;
    }
}

// A wrapper callback to support V0 shader callbacks.
static pax_col_t
    pax_shader_wrapper_for_v0(pax_col_t tint, pax_col_t existing, int x, int y, float u, float v, void *args0) {
    pax_shader_t        *args = args0;
    pax_shader_func_v0_t v0   = args->callback;
    return pax_col_merge(existing, v0(tint, x, y, u, v, args->callback_args));
}

// Gets the correct callback function for the shader.
pax_shader_ctx_t pax_get_shader_ctx(pax_buf_t *buf, pax_col_t color, pax_shader_t const *shader) {
    if (shader->schema_version != ~shader->schema_complement) {
        // TODO: Bad.
    }
    if (shader->schema_version == 0) {
        // Use the old version.
        return (pax_shader_ctx_t){
            .callback      = pax_shader_wrapper_for_v0,
            .callback_args = (void *)shader,
            .do_getter     = true,
            .skip          = false,
        };
    }

    // Use the new version.
    return (pax_shader_ctx_t){
        .callback      = shader->callback,
        .callback_args = shader->callback_args,
        .do_getter     = true,
        .skip          = false,
    };
}

// Dummy UVs used for quad UVs where NULL is provided.
static pax_quadf const dummy_quad_uvs = {.x0 = 0, .y0 = 0, .x1 = 1, .y1 = 0, .x2 = 1, .y2 = 1, .x3 = 0, .y3 = 1};

// Dummy UVs used for tri UVs where NULL is provided.
static pax_trif const dummy_tri_uvs = {.x0 = 0, .y0 = 0, .x1 = 1, .y1 = 0, .x2 = 0, .y2 = 1};



/* ============ BUFFER =========== */

// Buffer type info table.
static pax_buf_type_info_t const info_tab[] = {
#define PAX_DEF_BUF_TYPE_PAL(bpp, name)              [name] = {bpp, 1, 0, 0, 0, 1},
#define PAX_DEF_BUF_TYPE_GREY(bpp, name)             [name] = {bpp, 0, 0, 0, 0, 2},
#define PAX_DEF_BUF_TYPE_ARGB(bpp, a, r, g, b, name) [name] = {bpp, a, r, g, b, 3},
#include "helpers/pax_buf_type.inc"
};

// Get buffer type info.
pax_buf_type_info_t pax_buf_type_info(pax_buf_type_t type) {
    if (type < 0 || type > sizeof(info_tab) / sizeof(pax_buf_type_info_t)) {
        pax_set_err(PAX_ERR_PARAM);
        return (pax_buf_type_info_t){0};
    } else {
        pax_set_ok();
        return info_tab[type];
    }
}

// Create a new buffer.
// If mem is NULL, a new area is allocated.
pax_buf_t *pax_buf_init(void *mem, int width, int height, pax_buf_type_t type) {
    pax_buf_t *buf = malloc(sizeof(pax_buf_t));
    if (!buf) {
        pax_set_err(PAX_ERR_NOMEM);
        return NULL;
    }
    bool use_alloc = !mem;
    if (use_alloc) {
        // Allocate the right amount of bytes.
        mem = malloc(PAX_BUF_CALC_SIZE(width, height, type));
        if (!mem) {
            pax_set_err(PAX_ERR_NOMEM);
            free(buf);
            return NULL;
        }
    }
    *buf = (pax_buf_t){
        // Buffer size information.
        .type               = type,
        .buf                = mem,
        .width              = width,
        .height             = height,
        .bpp                = PAX_GET_BPP(type),
        // Defaults.
        .stack_2d           = {.parent = NULL, .value = matrix_2d_identity()},
        .reverse_endianness = false,
        // Memory management information.
        .do_free            = use_alloc,
        .do_free_pal        = false,
        .palette            = NULL,
    };
    // Update getters and setters.
    pax_get_col_conv(buf, &buf->col2buf, &buf->buf2col);
    pax_get_setters(buf, &buf->getter, &buf->setter, &buf->range_setter, &buf->range_merger);
    // The clip rectangle is disabled by default.
    pax_noclip(buf);
    // Easter egg.
    if (use_alloc) {
        pax_background(buf, 0);
        pax_draw_text(buf, PAX_IS_PALETTE(type) ? 1 : 0xffffffff, pax_font_sky, 9, 5, 5, "Julian Wuz Here");
    }
    pax_set_ok();
    return buf;
}

// Set the palette for buffers with palette types.
// Creates an internal copy of the palette.
void pax_buf_set_palette(pax_buf_t *buf, pax_col_t const *palette, size_t palette_len) {
    PAX_BUF_CHECK(buf);
    if (!PAX_IS_PALETTE(buf->type)) {
        PAX_ERROR(PAX_ERR_UNSUPPORTED);
    }
    void *mem = malloc(sizeof(pax_col_t) * palette_len);
    if (!mem) {
        PAX_ERROR(PAX_ERR_NOMEM);
    }
    if (buf->do_free_pal) {
        free((pax_col_t *)buf->palette);
    }
    buf->palette      = mem;
    buf->palette_size = palette_len;
    memcpy(mem, palette, sizeof(pax_col_t) * palette_len);
}

// Set the palette for buffers with palette types.
// Does not create internal copy of the palette.
void pax_buf_set_palette_rom(pax_buf_t *buf, pax_col_t const *palette, size_t palette_len) {
    PAX_BUF_CHECK(buf);
    if (!PAX_IS_PALETTE(buf->type)) {
        PAX_ERROR(PAX_ERR_UNSUPPORTED);
    }
    if (buf->do_free_pal) {
        free((pax_col_t *)buf->palette);
        buf->do_free_pal = false;
    }
    buf->palette = palette;
}

// Get the palette for buffers with palette types.
pax_col_t const *pax_buf_get_palette(pax_buf_t *buf, size_t *palette_len) {
    PAX_BUF_CHECK(buf, NULL);
    if (!PAX_IS_PALETTE(buf->type)) {
        PAX_ERROR(PAX_ERR_UNSUPPORTED, NULL);
    }
    *palette_len = buf->palette_size;
    return buf->palette;
}

// Enable/disable the reversing of endianness for `buf`.
// Some displays might require a feature like this one.
void pax_buf_reversed(pax_buf_t *buf, bool reversed_endianness) {
    PAX_BUF_CHECK(buf);

    // Update endianness flag.
    buf->reverse_endianness = reversed_endianness;
    // Update getters and setters.
    pax_get_col_conv(buf, &buf->col2buf, &buf->buf2col);
    pax_get_setters(buf, &buf->getter, &buf->setter, &buf->range_setter, &buf->range_merger);
}

// Destroy the buffer, freeing its memory.
void pax_buf_destroy(pax_buf_t *buf) {
    PAX_BUF_CHECK(buf);

    // Recursively unlink the matrix stack.
    matrix_stack_2d_t *current = buf->stack_2d.parent;
    while (current) {
        matrix_stack_2d_t *next = current->parent;
        free(current);
        current = next;
    }

    // Free allocated memory.
    if (buf->do_free) {
        free(buf->buf);
    }
    if (buf->palette && buf->do_free_pal) {
        free((pax_col_t *)buf->palette);
    }

    free(buf);
}


// Retrieve the width of the buffer.
int pax_buf_get_width(pax_buf_t const *buf) {
    PAX_BUF_CHECK(buf, 0);
    if (buf->orientation & 1) {
        return buf->height;
    } else {
        return buf->width;
    }
}
// Retrieve the height of the buffer.
int pax_buf_get_height(pax_buf_t const *buf) {
    PAX_BUF_CHECK(buf, 0);
    if (buf->orientation & 1) {
        return buf->width;
    } else {
        return buf->height;
    }
}
// Retrieve dimensions of the buffer.
pax_vec2i pax_buf_get_dims(pax_buf_t const *buf) {
    PAX_BUF_CHECK(buf, (pax_vec2i){0, 0});
    if (buf->orientation & 1) {
        return (pax_vec2i){buf->height, buf->width};
    } else {
        return (pax_vec2i){buf->width, buf->height};
    }
}
// Retrieve the type of the buffer.
pax_buf_type_t pax_buf_get_type(pax_buf_t const *buf) {
    PAX_BUF_CHECK(buf, -1);
    return buf->type;
}

// Get a const pointer to the image data.
void const *pax_buf_get_pixels(pax_buf_t const *buf) {
    PAX_BUF_CHECK(buf, NULL);
    return buf->buf;
}
// Get a non-const pointer to the image data.
void *pax_buf_get_pixels_rw(pax_buf_t *buf) {
    PAX_BUF_CHECK(buf, NULL);
    return buf->buf;
}
// Get the byte size of the image data.
size_t pax_buf_get_size(pax_buf_t const *buf) {
    PAX_BUF_CHECK(buf, 0);
    return PAX_BUF_CALC_SIZE(buf->width, buf->height, buf->type);
}

// Set orientation of the buffer.
void pax_buf_set_orientation(pax_buf_t *buf, pax_orientation_t x) {
    PAX_BUF_CHECK(buf);
    buf->orientation = x & 7;
}

// Get orientation of the buffer.
pax_orientation_t pax_buf_get_orientation(pax_buf_t const *buf) {
    PAX_BUF_CHECK(buf, 0);
    return buf->orientation;
}


// Scroll the buffer, filling with a placeholder color.
void pax_buf_scroll(pax_buf_t *buf, pax_col_t placeholder, int x, int y) {
    PAX_BUF_CHECK(buf);
#if PAX_COMPILE_MCR
    pax_join();
#endif

#if PAX_COMPILE_ORIENTATION
    {
        int x0 = x, y0 = y;
        // Fix direction of scrolling.
        switch (buf->orientation & 3) {
            default:
            case 0: break;
            case 1:
                y = -x0;
                x = y0;
                break;
            case 2:
                x = -x0;
                y = -y0;
                break;
            case 3:
                y = x0;
                x = -y0;
                break;
        }
        if (buf->orientation & 4) {
            x = -x;
        }
    }
#endif

    // Edge case: Scrolls too far.
    if (x >= buf->width || x <= -buf->width || y >= buf->height || y <= -buf->height) {
        pax_background(buf, placeholder);
        return;
    }

    // Pixel index offset for the copy.
    ptrdiff_t off   = x + y * buf->width;
    // Number of pixels that must be copied.
    size_t    count = buf->width * buf->height - labs(off);

    // Bit index version of the offset.
    ptrdiff_t bit_off   = PAX_GET_BPP(buf->type) * off;
    // Number of bits to copy.
    size_t    bit_count = PAX_GET_BPP(buf->type) * count;

    if ((bit_off & 7) == 0) {
        // If bit offset lines up to a byte, use memmove.
        ptrdiff_t byte_off   = bit_off / 8;
        size_t    byte_count = bit_count / 8;

        if (byte_off > 0) {
            memmove(buf->buf_8bpp + byte_off, buf->buf_8bpp, byte_count);
        } else {
            memmove(buf->buf_8bpp, buf->buf_8bpp - byte_off, byte_count);
        }

    } else {
        // If it does not, an expensive copy must be performed.
        if (off > 0) {
            for (ptrdiff_t i = count - 1; i >= 0; i--) {
                pax_col_t value = buf->getter(buf, off + i);
                buf->setter(buf, value, i);
            }
        } else {
            for (ptrdiff_t i = 0; i < count; i++) {
                pax_col_t value = buf->getter(buf, i);
                buf->setter(buf, value, off + i);
            }
        }
    }

#if PAX_COMPILE_ORIENTATION
    // Ignore orientation for a moment.
    int rot          = buf->orientation;
    buf->orientation = 0;
#endif

    // Fill the edges.
    if (x > 0) {
        pax_simple_rect(buf, placeholder, 0, y, x, buf->height - y);
    } else if (x < 0) {
        pax_simple_rect(buf, placeholder, buf->width, y, x, buf->height - y);
    }
    if (y > 0) {
        pax_simple_rect(buf, placeholder, 0, 0, buf->width, y);
    } else if (y < 0) {
        pax_simple_rect(buf, placeholder, 0, buf->height, buf->width, y);
    }

#if PAX_COMPILE_ORIENTATION
    // Restore previous orientation.
    buf->orientation = rot;
#endif
}


// Clip the buffer to the desired rectangle.
void pax_clip(pax_buf_t *buf, int x, int y, int width, int height) {
    PAX_BUF_CHECK(buf);
    if (width == 0 || height == 0) {
        buf->clip.w = 0;
        buf->clip.h = 0;
        return;
    }
    // Apply orientation.
    pax_vec2i p0 = pax_orient_det_vec2i(buf, (pax_vec2i){x, y});
    pax_vec2i p1 = pax_orient_det_vec2i(buf, (pax_vec2i){x + width - 1, y + height - 1});
    // Sort the points.
    if (p0.x > p1.x) {
        int t = p0.x;
        p0.x  = p1.x;
        p1.x  = t;
    }
    if (p0.y > p1.y) {
        int t = p0.y;
        p0.y  = p1.y;
        p1.y  = t;
    }
    // Clip points to be within buffer.
    if (p0.x < 0) {
        p0.x = 0;
    }
    if (p0.y < 0) {
        p0.y = 0;
    }
    if (p1.x >= buf->width) {
        p1.x = buf->width - 1;
    }
    if (p1.y >= buf->height) {
        p1.y = buf->height - 1;
    }
    // Apply the clip.
    buf->clip = (pax_recti){
        p0.x,
        p0.y,
        p1.x - p0.x + 1,
        p1.y - p0.y + 1,
    };
}

// Get the current clip rectangle.
pax_recti pax_get_clip(pax_buf_t const *buf) {
    PAX_BUF_CHECK(buf, (pax_recti){0, 0});
    return pax_recti_abs(pax_unorient_det_recti(buf, buf->clip));
}

// Clip the buffer to it's full size.
void pax_noclip(pax_buf_t *buf) {
    PAX_BUF_CHECK(buf);
    buf->clip = (pax_recti){.x = 0, .y = 0, .w = buf->width, .h = buf->height};
}

// Check whether the buffer is dirty.
bool pax_is_dirty(pax_buf_t const *buf) {
    PAX_BUF_CHECK(buf, false);
    return buf->dirty_x0 < buf->dirty_x1;
}

// Get a copy of the dirty rectangle.
pax_recti pax_get_dirty(pax_buf_t const *buf) {
    PAX_BUF_CHECK(buf, (pax_recti){0, 0});
    return (pax_recti){
        buf->dirty_x0,
        buf->dirty_y0,
        buf->dirty_x1 - buf->dirty_x0 + 1,
        buf->dirty_y1 - buf->dirty_y0 + 1,
    };
}

// Mark the entire buffer as clean.
void pax_mark_clean(pax_buf_t *buf) {
    PAX_BUF_CHECK(buf);
    buf->dirty_x0 = buf->width;
    buf->dirty_y0 = buf->height;
    buf->dirty_x1 = -1;
    buf->dirty_y1 = -1;
}

// Mark the entire buffer as dirty.
void pax_mark_dirty0(pax_buf_t *buf) {
    PAX_BUF_CHECK(buf);
    buf->dirty_x0 = 0;
    buf->dirty_y0 = 0;
    buf->dirty_x1 = buf->width - 1;
    buf->dirty_y1 = buf->height - 1;
}

// Mark a single point as dirty.
void pax_mark_dirty1(pax_buf_t *buf, int x, int y) {
    PAX_BUF_CHECK(buf);

    if (x < 0)
        x = 0;
    if (y < 0)
        y = 0;
    if (x >= buf->width)
        y = buf->width - 1;
    if (y >= buf->height)
        y = buf->height - 1;

    if (x < buf->dirty_x0)
        buf->dirty_x0 = x;
    if (x > buf->dirty_x1)
        buf->dirty_x1 = x;
    if (y < buf->dirty_y0)
        buf->dirty_y0 = y;
    if (y > buf->dirty_y1)
        buf->dirty_y1 = y;
}

// Mark a rectangle as dirty.
void pax_mark_dirty2(pax_buf_t *buf, int x, int y, int width, int height) {
    PAX_BUF_CHECK(buf);

    if (x < buf->dirty_x0)
        buf->dirty_x0 = x;
    if (x + width - 1 > buf->dirty_x1)
        buf->dirty_x1 = x + width - 1;
    if (y < buf->dirty_y0)
        buf->dirty_y0 = y;
    if (y + height - 1 > buf->dirty_y1)
        buf->dirty_y1 = y + height - 1;

    if (buf->dirty_x0 < 0)
        buf->dirty_x0 = 0;
    if (buf->dirty_y0 < 0)
        buf->dirty_y0 = 0;
    if (buf->dirty_x0 >= buf->width)
        buf->dirty_x0 = buf->width - 1;
    if (buf->dirty_y0 >= buf->height)
        buf->dirty_y0 = buf->height - 1;
}



/* ============ COLORS =========== */

// 8-bit + 8-bit fractional (0x00ff=1) division.
static inline uint16_t pax_frac_div16(uint16_t a, uint8_t b) {
    return (a << 8) / (b + (b >> 7));
}


// Internal method for AHSV to ARGB.
// Ranges are 0xff, 0x5ff, 0xff, 0xff.
pax_col_t PRIVATE_pax_col_hsv(uint8_t a, uint16_t h, uint8_t s, uint8_t v) {
    uint8_t phase = h >> 8;
    // Parts of HSV.
    uint8_t up, down, other;
    other = ~s;
    if (h & 0x100) {
        // Down goes away.
        up   = 0xff;
        down = pax_lerp(s, 0xff, ~h & 0xff);
    } else {
        // Up comes in.
        up   = pax_lerp(s, 0xff, h & 0xff);
        down = 0xff;
    }
    // Apply brightness.
    up    = pax_lerp(v, 0, up);
    down  = pax_lerp(v, 0, down);
    other = pax_lerp(v, 0, other);
    // Apply to RGB.
    uint8_t r, g, b;
    switch (phase >> 1) {
        default /* case 0 */:
            // From R to G.
            r = down;
            g = up;
            b = other;
            break;
        case 1:
            // From G to B.
            r = other;
            g = down;
            b = up;
            break;
        case 2:
            // From B to R.
            r = up;
            g = other;
            b = down;
            break;
    }
    // Merge.
    return (a << 24) | (r << 16) | (g << 8) | b;
}

// Internal method for RGB to HSV.
// Ranges are 0x5ff, 0xff, 0xff.
void PRIVATE_pax_undo_col_hsv(pax_col_t in, uint16_t *h, uint8_t *s, uint8_t *v) {
    // Split the RGB.
    uint8_t r = in >> 16;
    uint8_t g = in >> 8;
    uint8_t b = in;

    // Edge case: Equal brightness.
    if (r == g && g == b) {
        *h = 0;
        *s = 0;
        *v = r;
        return;
    }

    // Sort levels.
    uint8_t high = r, middle = g, low = b;
    if (high < middle) {
        uint8_t tmp = high;
        high        = middle;
        middle      = tmp;
    }
    if (middle < low) {
        uint8_t tmp = middle;
        middle      = low;
        low         = tmp;
    }
    if (high < middle) {
        uint8_t tmp = high;
        high        = middle;
        middle      = tmp;
    }

    // Factor out brightness.
    *v     = high;
    middle = middle * 255 / high;
    low    = low * 255 / high;
    r      = r * 255 / high;
    g      = g * 255 / high;
    b      = b * 255 / high;
    high   = 255;

    // Factor out saturation.
    *s = ~low;

    // How I inverted the function (where 1.0=0xff):

    // middle = lerp(s, 1, X)
    // middle = 1 + s * (X - 1)
    // middle = 1 + s * X - s * 1

    // middle - 1 + s * 1 = s * X
    // s * X = middle - 1 + s * 1

    // X = (middle - 1 + s * 1) / s
    // X = (middle - 1) / s + 1

    // This is it, written in code.
    // Here, `x` is either `~h` or `h` in a 9-bit context,
    // From the interpolation of `up` and `down` in hsv.
    uint16_t x = pax_frac_div16(middle - 0xff + *s, *s);

    // Reason about hue.
    uint16_t l_h;
    if (r == high) {
        if (g == middle) {
            // R = down, [G = up], h < 0x100
            l_h = 0x000 | x;
        } else {
            // [B = down], R = up, h > 0x100
            l_h = 0x500 | (255 - x);
        }
    } else if (g == high) {
        if (b == middle) {
            // G = down, [B = up], h < 0x100
            l_h = 0x200 | x;
        } else {
            // [R = down], G = up, h > 0x100
            l_h = 0x100 | (255 - x);
        }
    } else /* b == high */ {
        if (r == middle) {
            // B = down, [R = up], h < 0x100
            l_h = 0x400 | x;
        } else {
            // [G = down], B = up, h > 0x100
            l_h = 0x300 | (255 - x);
        }
    }

    *h = l_h;
}


// Converts HSV to ARGB, ranges are 0-255.
pax_col_t pax_col_hsv(uint8_t h, uint8_t s, uint8_t v) {
    return PRIVATE_pax_col_hsv(255, h * 6, s, v);
}

// Converts AHSV to ARGB, ranges are 0-255.
pax_col_t pax_col_ahsv(uint8_t a, uint8_t h, uint8_t s, uint8_t v) {
    return PRIVATE_pax_col_hsv(a, h * 6, s, v);
}

// Converts HSV to ARGB, ranges are 0-359.
pax_col_t pax_col_hsv_alt(uint16_t h, uint8_t s, uint8_t v) {
    return PRIVATE_pax_col_hsv(255, h % 360 * 6 * 255 / 359, s, v);
}

// Converts AHSV to ARGB.
pax_col_t pax_col_ahsv_alt(uint8_t a, uint16_t h, uint8_t s, uint8_t v) {
    return PRIVATE_pax_col_hsv(a, h % 360 * 6 * 255 / 359, s, v);
}


// Converts ARGB into AHSV, ranges are 0-255.
void pax_undo_ahsv(pax_col_t in, uint8_t *a, uint8_t *h, uint8_t *s, uint8_t *v) {
    *a = in >> 24;
    uint16_t l_h;
    PRIVATE_pax_undo_col_hsv(in, &l_h, s, v);
    *h = (l_h + 3) / 6;
}

// Converts RGB into HSV, ranges are 0-255.
void pax_undo_hsv(pax_col_t in, uint8_t *h, uint8_t *s, uint8_t *v) {
    uint16_t l_h;
    PRIVATE_pax_undo_col_hsv(in, &l_h, s, v);
    *h = (l_h + 3) / 6;
}

// Converts ARGB into AHSV, ranges are 0-255, 0-359, 0-99, 0-99.
void pax_undo_ahsv_alt(pax_col_t in, uint8_t *a, uint16_t *h, uint8_t *s, uint8_t *v) {
    *a = in >> 24;
    uint16_t l_h;
    PRIVATE_pax_undo_col_hsv(in, &l_h, s, v);
    *h = (l_h + 3) * 359 / 255 / 6;
    *s = *s * 100 / 255;
    *v = *v * 100 / 255;
}

// Converts RGB into HSV, ranges are 0-359, 0-99, 0-99.
void pax_undo_hsv_alt(pax_col_t in, uint16_t *h, uint8_t *s, uint8_t *v) {
    uint16_t l_h;
    PRIVATE_pax_undo_col_hsv(in, &l_h, s, v);
    *h = (l_h + 3) * 359 / 255 / 6;
    *s = *s * 100 / 255;
    *v = *v * 100 / 255;
}


// Linearly interpolates between from and to, including alpha.
pax_col_t pax_col_lerp(uint8_t part, pax_col_t from, pax_col_t to) {
    return (pax_lerp(part, from >> 24, to >> 24) << 24) | (pax_lerp(part, from >> 16, to >> 16) << 16)
           | (pax_lerp(part, from >> 8, to >> 8) << 8) | pax_lerp(part, from, to);
}

// Merges the two colors, based on alpha.
pax_col_t pax_col_merge(pax_col_t base, pax_col_t top) {
    // It is not more optimal to add exceptions for full or zero alpha due to linearity.

    // Otherwise, do a full alpha blend.
    uint8_t part = top >> 24;
    // clang-format off
    top |= 0xff000000;
    return pax_lerp_mask(0x00ff00ff, part, base, top)
         | pax_lerp_mask(0xff00ff00, part, base, top);

    // return pax_lerp_off(24, part, base, 255)
    //      | pax_lerp_off(16, part, base, top)
    //      | pax_lerp_off( 8, part, base, top)
    //      | pax_lerp_off( 0, part, base, top);

    // return (pax_lerp(part, base >> 24, 255) << 24)
    //      | (pax_lerp(part, base >> 16, top >> 16) << 16)
    //      | (pax_lerp(part, base >> 8, top >> 8) << 8)
    //      |  pax_lerp(part, base, top);

    // clang-format on
}

// Tints the color, commonly used for textures.
pax_col_t pax_col_tint(pax_col_t col, pax_col_t tint) {
    // It is not more optimal to add exceptions for full or zero alpha due to linearity.

    // Otherwise, do a full tint.
    return (pax_lerp(tint >> 24, 0, col >> 24) << 24) | (pax_lerp(tint >> 16, 0, col >> 16) << 16)
           | (pax_lerp(tint >> 8, 0, col >> 8) << 8) | pax_lerp(tint, 0, col);
}



/* ======== DRAWING: PIXEL ======= */

// Set a pixel, merging with alpha.
void pax_merge_pixel(pax_buf_t *buf, pax_col_t color, int x, int y) {
    PAX_BUF_CHECK(buf);

#if PAX_COMPILE_ORIENTATION
    pax_vec2i tmp = pax_orient_det_vec2i(buf, (pax_vec2i){x, y});
    x             = tmp.x;
    y             = tmp.y;
#endif

    if (x < 0 || x >= buf->width || y < 0 || y >= buf->height) {
        // Out of bounds error.
        pax_last_error = PAX_ERR_BOUNDS;
        return;
    }

    int index = x + y * buf->width;
    if (PAX_IS_PALETTE(buf->type)) {
        // Palette colors don't have conversion.
        if (color & 0xff000000)
            buf->setter(buf, color, index);
    } else if (color >= 0xff000000) {
        // Opaque colors don't need alpha blending.
        buf->setter(buf, buf->col2buf(buf, color), index);
    } else if (color & 0xff000000) {
        // Non-transparent colors will be blended normally.
        pax_col_t base = buf->buf2col(buf, buf->getter(buf, index));
        buf->setter(buf, buf->col2buf(buf, pax_col_merge(base, color)), index);
    }
}

// Set a pixel.
void pax_set_pixel(pax_buf_t *buf, pax_col_t color, int x, int y) {
    PAX_BUF_CHECK(buf);

#if PAX_COMPILE_ORIENTATION
    pax_vec2i tmp = pax_orient_det_vec2i(buf, (pax_vec2i){x, y});
    x             = tmp.x;
    y             = tmp.y;
#endif

    if (x < 0 || x >= buf->width || y < 0 || y >= buf->height) {
        // Out of bounds error.
        pax_last_error = PAX_ERR_BOUNDS;
        return;
    }

    int index = x + y * buf->width;
    if (PAX_IS_PALETTE(buf->type)) {
        // Palette colors don't have conversion.
        buf->setter(buf, color, index);
    } else {
        // But all other colors do have a conversion.
        buf->setter(buf, buf->col2buf(buf, color), index);
    }
}

// Get a pixel.
pax_col_t pax_get_pixel(pax_buf_t const *buf, int x, int y) {
    PAX_BUF_CHECK(buf, 0);

#if PAX_COMPILE_ORIENTATION
    pax_vec2i tmp = pax_orient_det_vec2i(buf, (pax_vec2i){x, y});
    x             = tmp.x;
    y             = tmp.y;
#endif

    if (x < 0 || x >= buf->width || y < 0 || y >= buf->height) {
        // Out of bounds error.
        pax_last_error = PAX_ERR_BOUNDS;
        return 0;
    }
    return buf->buf2col(buf, buf->getter(buf, x + y * buf->width));
}

// Set a pixel without color conversion.
void pax_set_pixel_raw(pax_buf_t *buf, pax_col_t color, int x, int y) {
    PAX_BUF_CHECK(buf);

#if PAX_COMPILE_ORIENTATION
    pax_vec2i tmp = pax_orient_det_vec2i(buf, (pax_vec2i){x, y});
    x             = tmp.x;
    y             = tmp.y;
#endif

    if (x < 0 || x >= buf->width || y < 0 || y >= buf->height) {
        // Out of bounds error.
        pax_last_error = PAX_ERR_BOUNDS;
        return;
    }

    int index = x + y * buf->width;
    // Don't do any color conversion.
    buf->setter(buf, color, index);
}

// Get a pixel without color conversion.
pax_col_t pax_get_pixel_raw(pax_buf_t const *buf, int x, int y) {
    PAX_BUF_CHECK(buf, 0);

#if PAX_COMPILE_ORIENTATION
    pax_vec2i tmp = pax_orient_det_vec2i(buf, (pax_vec2i){x, y});
    x             = tmp.x;
    y             = tmp.y;
#endif

    if (x < 0 || x >= buf->width || y < 0 || y >= buf->height) {
        // Out of bounds error.
        pax_last_error = PAX_ERR_BOUNDS;
        return 0;
    }
    return buf->getter(buf, x + y * buf->width);
}



/* ========= DRAWING: 2D ========= */

// Draws an image at the image's normal size.
void pax_draw_image(pax_buf_t *buf, pax_buf_t const *image, float x, float y) {
    PAX_BUF_CHECK(buf);
    PAX_BUF_CHECK(image);
    pax_draw_image_sized(buf, image, x, y, image->width, image->height);
}

// Draw an image with a prespecified size.
void pax_draw_image_sized(pax_buf_t *buf, pax_buf_t const *image, float x, float y, float width, float height) {
    PAX_BUF_CHECK(buf);
    PAX_BUF_CHECK(image);
    if (PAX_IS_ALPHA(image->type)) {
        pax_shade_rect(buf, -1, &PAX_SHADER_TEXTURE(image), NULL, x, y, width, height);
    } else {
        pax_shade_rect(buf, -1, &PAX_SHADER_TEXTURE_OP(image), NULL, x, y, width, height);
    }
}

// Draws an image at the image's normal size.
// Assumes the image is completely opaque, any transparent parts are drawn opaque.
void pax_draw_image_op(pax_buf_t *buf, pax_buf_t const *image, float x, float y) {
    PAX_BUF_CHECK(buf);
    PAX_BUF_CHECK(image);
    pax_draw_image_sized_op(buf, image, x, y, image->width, image->height);
}

// Draw an image with a prespecified size.
// Assumes the image is completely opaque, any transparent parts are drawn opaque.
void pax_draw_image_sized_op(pax_buf_t *buf, pax_buf_t const *image, float x, float y, float width, float height) {
    PAX_BUF_CHECK(buf);
    PAX_BUF_CHECK(image);
    pax_shade_rect(buf, -1, &PAX_SHADER_TEXTURE_OP(image), NULL, x, y, width, height);
}

// Draw a rectangle with a shader.
// If uvs is NULL, a default will be used (0,0; 1,0; 1,1; 0,1).
void pax_shade_rect(
    pax_buf_t          *buf,
    pax_col_t           color,
    pax_shader_t const *shader,
    pax_quadf const    *uvs,
    float               x,
    float               y,
    float               width,
    float               height
) {
    if (!shader) {
        // If shader is NULL, simplify this.
        pax_draw_rect(buf, color, x, y, width, height);
        return;
    }

    PAX_BUF_CHECK(buf);

    if (!uvs) {
        // Apply default UVs.
        uvs = &dummy_quad_uvs;
    }

    if (matrix_2d_is_identity2(buf->stack_2d.value)) {
        // We don't need to use triangles here.
        matrix_2d_transform(buf->stack_2d.value, &x, &y);
        width  *= buf->stack_2d.value.a0;
        height *= buf->stack_2d.value.b1;

// Perform rotation.
#if PAX_COMPILE_ORIENTATION
        pax_rectf tmp = pax_orient_det_rectf(buf, (pax_rectf){x, y, width, height});
        x             = tmp.x;
        y             = tmp.y;
        width         = tmp.w;
        height        = tmp.h;

        pax_quadf uvs_rotated;
        if (buf->orientation & 1) {
            uvs_rotated = (pax_quadf){
                uvs->x0,
                uvs->y0,
                uvs->x3,
                uvs->y3,
                uvs->x2,
                uvs->y2,
                uvs->x1,
                uvs->y1,
            };
            uvs = &uvs_rotated;
        }
#endif

        pax_mark_dirty2(buf, x - 0.5, y - 0.5, width + 1, height + 1);
#if PAX_COMPILE_MCR
        if (pax_do_multicore) {
            // Assign worker task.
            pax_task_t task
                = {.buffer     = buf,
                   .type       = PAX_TASK_RECT,
                   .color      = color,
                   .use_shader = shader,
                   .quad_uvs   = *uvs,
                   .shape      = {x, y, width, height},
                   .shape_len  = 4};
            if (shader)
                task.shader = *shader;
            paxmcr_add_task(&task);
            // Draw our part.
            paxmcr_rect_shaded(
                false,
                buf,
                color,
                shader,
                x,
                y,
                width,
                height,
                uvs->x0,
                uvs->y0,
                uvs->x1,
                uvs->y1,
                uvs->x2,
                uvs->y2,
                uvs->x3,
                uvs->y3
            );
        } else
#endif
        {
            pax_rect_shaded(
                buf,
                color,
                shader,
                x,
                y,
                width,
                height,
                uvs->x0,
                uvs->y0,
                uvs->x1,
                uvs->y1,
                uvs->x2,
                uvs->y2,
                uvs->x3,
                uvs->y3
            );
        }
    } else {
        // Draw as a quad.
        matrix_2d_t mtx       = buf->stack_2d.value;
        pax_vec2f   vertex[4] = {
            matrix_2d_transform_alt(mtx, (pax_vec2f){x, y}),
            matrix_2d_transform_alt(mtx, (pax_vec2f){x + width, y}),
            matrix_2d_transform_alt(mtx, (pax_vec2f){x + width, y + height}),
            matrix_2d_transform_alt(mtx, (pax_vec2f){x, y + height}),
        };
#if PAX_COMPILE_MCR
        if (pax_do_multicore) {
            // Assign worker task.
            pax_task_t task
                = {.buffer     = buf,
                   .type       = PAX_TASK_QUAD,
                   .color      = color,
                   .use_shader = shader,
                   .quad_uvs   = *uvs,
                   .shape_len  = 8};
            memcpy(task.shape, &vertex, sizeof(vertex));
            if (shader)
                task.shader = *shader;
            paxmcr_add_task(&task);
            // Draw our part.
            paxmcr_quad_shaded(
                false,
                buf,
                color,
                shader,
                vertex[0].x,
                vertex[0].y,
                vertex[1].x,
                vertex[1].y,
                vertex[2].x,
                vertex[2].y,
                vertex[3].x,
                vertex[3].y,
                uvs->x0,
                uvs->y0,
                uvs->x1,
                uvs->y1,
                uvs->x2,
                uvs->y2,
                uvs->x3,
                uvs->y3
            );
        } else
#endif
        {
            pax_quad_shaded(
                buf,
                color,
                shader,
                vertex[0].x,
                vertex[0].y,
                vertex[1].x,
                vertex[1].y,
                vertex[2].x,
                vertex[2].y,
                vertex[3].x,
                vertex[3].y,
                uvs->x0,
                uvs->y0,
                uvs->x1,
                uvs->y1,
                uvs->x2,
                uvs->y2,
                uvs->x3,
                uvs->y3
            );
        }
    }
}

// Draw a line with a shader.
// Beta feature: UVs are not currently available.
void pax_shade_line(
    pax_buf_t          *buf,
    pax_col_t           color,
    pax_shader_t const *shader,
    pax_linef const    *uvs,
    float               x0,
    float               y0,
    float               x1,
    float               y1
) {
    if (!shader) {
        pax_draw_line(buf, color, x0, y0, x1, y1);
        return;
    }

    PAX_BUF_CHECK(buf);

    float u0, v0, u1, v1;

    if (uvs) {
        u0 = uvs->x0;
        v0 = uvs->y0;
        u1 = uvs->x1;
        v1 = uvs->y1;
    } else {
        u0 = 0;
        v0 = 0;
        u1 = 1;
        v1 = 0;
    }

    // Apply transforms.
    matrix_2d_transform(buf->stack_2d.value, &x0, &y0);
    matrix_2d_transform(buf->stack_2d.value, &x1, &y1);

    if (!isfinite(x0) || !isfinite(y0) || !isfinite(x1) || !isfinite(y1)) {
        // We can't draw to infinity.
        pax_last_error = PAX_ERR_INF;
        return;
    }

    // Rotate points.
    pax_vec1_t tmp = pax_orient_det_vec2f(buf, (pax_vec2f){x0, y0});
    x0             = tmp.x;
    y0             = tmp.y;
    tmp            = pax_orient_det_vec2f(buf, (pax_vec2f){x1, y1});
    x1             = tmp.x;
    y1             = tmp.y;

    // If any point is outside clip now, we don't draw a line.
    if (y0 < buf->clip.y || y1 > buf->clip.y + buf->clip.h - 1)
        return;

    pax_mark_dirty1(buf, x0, y0);
    pax_mark_dirty1(buf, x1, y1);
#if PAX_COMPILE_MCR
    // Because a line isn't drawn in alternating scanlines, we need to sync up with the worker.
    pax_join();
#endif
    pax_line_shaded(buf, color, shader, x0, y0, x1, y1, u0, v0, u1, v1);
}

// Draw a triangle with a shader.
// If uvs is NULL, a default will be used (0,0; 1,0; 0,1).
void pax_shade_tri(
    pax_buf_t          *buf,
    pax_col_t           color,
    pax_shader_t const *shader,
    pax_trif const     *uvs,
    float               x0,
    float               y0,
    float               x1,
    float               y1,
    float               x2,
    float               y2
) {
    if (!shader) {
        // If shader is NULL, simplify this.
        pax_draw_tri(buf, color, x0, y0, x1, y1, x2, y2);
        return;
    }

    PAX_BUF_CHECK(buf);
    matrix_2d_transform(buf->stack_2d.value, &x0, &y0);
    matrix_2d_transform(buf->stack_2d.value, &x1, &y1);
    matrix_2d_transform(buf->stack_2d.value, &x2, &y2);

    if (!isfinite(x0) || !isfinite(y0) || !isfinite(x1) || !isfinite(y1) || !isfinite(x2) || !isfinite(y2)) {
        // We can't draw to infinity.
        pax_last_error = PAX_ERR_INF;
        return;
    }

    // Rotate points.
    pax_vec1_t tmp = pax_orient_det_vec2f(buf, (pax_vec2f){x0, y0});
    x0             = tmp.x;
    y0             = tmp.y;
    tmp            = pax_orient_det_vec2f(buf, (pax_vec2f){x1, y1});
    x1             = tmp.x;
    y1             = tmp.y;
    tmp            = pax_orient_det_vec2f(buf, (pax_vec2f){x2, y2});
    x2             = tmp.x;
    y2             = tmp.y;

    if (!uvs) {
        // Apply default UVs.
        uvs = &dummy_tri_uvs;
    }

    if ((y2 == y0 && y1 == y0) || (x2 == x0 && x1 == x0)) {
        // We can't draw a flat triangle.
        return;
    }

    // Mark each corner of the triangle as dirty.
    pax_mark_dirty1(buf, x0 - 0.5, y0 - 0.5);
    pax_mark_dirty1(buf, x1 - 0.5, y1 - 0.5);
    pax_mark_dirty1(buf, x2 - 0.5, y2 - 0.5);
    pax_mark_dirty1(buf, x0 + 0.5, y0 + 0.5);
    pax_mark_dirty1(buf, x1 + 0.5, y1 + 0.5);
    pax_mark_dirty1(buf, x2 + 0.5, y2 + 0.5);

#if PAX_COMPILE_MCR
    if (pax_do_multicore) {
        // Assign worker task.
        pax_task_t task
            = {.buffer     = buf,
               .type       = PAX_TASK_TRI,
               .color      = color,
               .use_shader = shader,
               .tri_uvs    = *uvs,
               .shape      = {x0, y0, x1, y1, x2, y2},
               .shape_len  = 6};
        if (shader)
            task.shader = *shader;
        paxmcr_add_task(&task);
        // Draw our part.
        paxmcr_tri_shaded(
            false,
            buf,
            color,
            shader,
            x0,
            y0,
            x1,
            y1,
            x2,
            y2,
            uvs->x0,
            uvs->y0,
            uvs->x1,
            uvs->y1,
            uvs->x2,
            uvs->y2
        );
    } else
#endif
    {
        pax_tri_shaded(
            buf,
            color,
            shader,
            x0,
            y0,
            x1,
            y1,
            x2,
            y2,
            uvs->x0,
            uvs->y0,
            uvs->x1,
            uvs->y1,
            uvs->x2,
            uvs->y2
        );
    }
}

// Draw an arc with a shader, angles in radians.
// If uvs is NULL, a default will be used (0,0; 1,0; 1,1; 0,1).
void pax_shade_arc(
    pax_buf_t          *buf,
    pax_col_t           color,
    pax_shader_t const *shader,
    pax_quadf const    *uvs,
    float               x,
    float               y,
    float               r,
    float               a0,
    float               a1
) {
    if (!shader) {
        // If shader is NULL, simplify this.
        pax_draw_arc(buf, color, x, y, r, a0, a1);
        return;
    }

    PAX_BUF_CHECK(buf);

    if (!uvs) {
        // Assign default UVs.
        uvs = &dummy_quad_uvs;
    }

    // Simplify the angles slightly.
    float a2  = fmodf(a0, M_PI * 2);
    a1       += a2 - a0;
    a0        = a2;
    if (a1 < a0)
        PAX_SWAP(float, a0, a1);
    if (a1 - a0 > M_PI * 2) {
        a1 = M_PI * 2;
        a0 = 0;
    }

    // Pick an appropriate number of divisions.
    int n_div = pax_pick_arc_divs(&buf->stack_2d.value, r, a0, a1);

    // Get the sine and cosine of one division, used for rotation in the loop.
    float div_angle = (a1 - a0) / n_div;
    float c_sin     = sinf(div_angle);
    float c_cos     = cosf(div_angle);

    // Start with a unit vector according to a0.
    float x0 = cosf(a0);
    float y0 = sinf(a0);

    // Prepare some UVs to apply to the triangle.
    pax_trif tri_uvs;
    tri_uvs.x0 = (uvs->x0 + uvs->x1 + uvs->x2 + uvs->x3) * 0.25;
    tri_uvs.y0 = (uvs->y0 + uvs->y1 + uvs->y2 + uvs->y3) * 0.25;

    tri_uvs.x1 = pax_flerp4(x0, y0, uvs->x0, uvs->x1, uvs->x3, uvs->x2);
    tri_uvs.y1 = pax_flerp4(x0, y0, uvs->y0, uvs->y1, uvs->y3, uvs->y2);

    // Draw it as a series of triangles, rotating with what is essentially matrix multiplication.
    for (int i = 0; i < n_div; i++) {
        // Perform the rotation.
        float x1   = x0 * c_cos - y0 * c_sin;
        float y1   = x0 * c_sin + y0 * c_cos;
        // And UV interpolation.
        tri_uvs.x2 = pax_flerp4(x1, y1, uvs->x0, uvs->x1, uvs->x3, uvs->x2);
        tri_uvs.y2 = pax_flerp4(x1, y1, uvs->y0, uvs->y1, uvs->y3, uvs->y2);
        // We subtract y0 and y1 from y because our up is -y.
        pax_shade_tri(buf, color, shader, &tri_uvs, x, y, x + x0 * r, y - y0 * r, x + x1 * r, y - y1 * r);
        // Assign the newly rotated vectors.
        x0         = x1;
        y0         = y1;
        tri_uvs.x1 = tri_uvs.x2;
        tri_uvs.y1 = tri_uvs.y2;
    }
}

// Draw a circle with a shader.
// If uvs is NULL, a default will be used (0,0; 1,0; 1,1; 0,1).
void pax_shade_circle(
    pax_buf_t *buf, pax_col_t color, pax_shader_t const *shader, pax_quadf const *uvs, float x, float y, float r
) {
    if (!shader) {
        pax_draw_circle(buf, color, x, y, r);
    }

    PAX_BUF_CHECK(buf);

    // Use precalcualted circles for speed because the user can't tell anyway.
    pax_vec2f const *preset;
    pax_trif const  *uv_set;
    size_t           size = pax_pick_circle(&buf->stack_2d.value, r, &preset, &uv_set);

    // Use the builtin matrix stuff to our advantage.
    pax_push_2d(buf);
    pax_apply_2d(buf, matrix_2d_translate(x, y));
    pax_apply_2d(buf, matrix_2d_scale(r, r));
    if (uvs) {
        // UV interpolation required.
        pax_trif uv_res;
        uv_res.x0 = (uvs->x1 + uvs->x2) * 0.5;
        uv_res.y0 = (uvs->y1 + uvs->y2) * 0.5;
        uv_res.x1 = pax_flerp4(preset[1].x, -preset[1].y, uvs->x0, uvs->x1, uvs->x3, uvs->x2);
        uv_res.y1 = pax_flerp4(preset[1].x, -preset[1].y, uvs->y0, uvs->y1, uvs->y3, uvs->y2);
        for (size_t i = 0; i < size - 1; i++) {
            uv_res.x2 = pax_flerp4(preset[i + 1].x, -preset[i + 1].y, uvs->x0, uvs->x1, uvs->x3, uvs->x2);
            uv_res.y2 = pax_flerp4(preset[i + 1].x, -preset[i + 1].y, uvs->y0, uvs->y1, uvs->y3, uvs->y2);
            pax_shade_tri(
                buf,
                color,
                shader,
                &uv_res,
                preset[0].x,
                preset[0].y,
                preset[i].x,
                preset[i].y,
                preset[i + 1].x,
                preset[i + 1].y
            );
            uv_res.x1 = uv_res.x2;
            uv_res.y1 = uv_res.y2;
        }

    } else {
        // No UV interpolation needed.
        for (size_t i = 0; i < size - 1; i++) {
            pax_shade_tri(
                buf,
                color,
                shader,
                &uv_set[i],
                preset[0].x,
                preset[0].y,
                preset[i].x,
                preset[i].y,
                preset[i + 1].x,
                preset[i + 1].y
            );
        }
    }
    pax_pop_2d(buf);
}

// Draw a rectangle.
void pax_draw_rect(pax_buf_t *buf, pax_col_t color, float x, float y, float width, float height) {
    PAX_BUF_CHECK(buf);
    if (!pax_do_draw_col(buf, color))
        return;

    if (matrix_2d_is_identity2(buf->stack_2d.value)) {
        // We don't need to use triangles here.
        matrix_2d_transform(buf->stack_2d.value, &x, &y);
        width  *= buf->stack_2d.value.a0;
        height *= buf->stack_2d.value.b1;
        pax_simple_rect(buf, color, x, y, width, height);
    } else {
        // Draw as a quad.
        matrix_2d_t mtx       = buf->stack_2d.value;
        pax_vec2f   vertex[4] = {
            matrix_2d_transform_alt(mtx, (pax_vec2f){x, y}),
            matrix_2d_transform_alt(mtx, (pax_vec2f){x + width, y}),
            matrix_2d_transform_alt(mtx, (pax_vec2f){x + width, y + height}),
            matrix_2d_transform_alt(mtx, (pax_vec2f){x, y + height}),
        };
#if PAX_COMPILE_MCR
        if (pax_do_multicore) {
            // Assign worker task.
            pax_task_t task
                = {.buffer = buf, .type = PAX_TASK_QUAD, .color = color, .use_shader = NULL, .shape_len = 8};
            memcpy(task.shape, &vertex, sizeof(vertex));
            paxmcr_add_task(&task);
            // Draw our part.
            paxmcr_quad_unshaded(
                false,
                buf,
                color,
                vertex[0].x,
                vertex[0].y,
                vertex[1].x,
                vertex[1].y,
                vertex[2].x,
                vertex[2].y,
                vertex[3].x,
                vertex[3].y
            );
        } else
#endif
        {
            pax_quad_unshaded(
                buf,
                color,
                vertex[0].x,
                vertex[0].y,
                vertex[1].x,
                vertex[1].y,
                vertex[2].x,
                vertex[2].y,
                vertex[3].x,
                vertex[3].y
            );
        }
    }
}

// Draw a line.
void pax_draw_line(pax_buf_t *buf, pax_col_t color, float x0, float y0, float x1, float y1) {
    PAX_BUF_CHECK(buf);
    if (!pax_do_draw_col(buf, color))
        return;
    // Apply transforms.
    matrix_2d_transform(buf->stack_2d.value, &x0, &y0);
    matrix_2d_transform(buf->stack_2d.value, &x1, &y1);
    // Draw the line.
    pax_simple_line(buf, color, x0, y0, x1, y1);
}

// Draw a triangle.
void pax_draw_tri(pax_buf_t *buf, pax_col_t color, float x0, float y0, float x1, float y1, float x2, float y2) {
    PAX_BUF_CHECK(buf);
    if (!pax_do_draw_col(buf, color))
        return;
    // Apply the transforms.
    matrix_2d_transform(buf->stack_2d.value, &x0, &y0);
    matrix_2d_transform(buf->stack_2d.value, &x1, &y1);
    matrix_2d_transform(buf->stack_2d.value, &x2, &y2);
    // Draw the triangle.
    pax_simple_tri(buf, color, x0, y0, x1, y1, x2, y2);
}

// Draw na arc, angles in radians.
void pax_draw_arc(pax_buf_t *buf, pax_col_t color, float x, float y, float r, float a0, float a1) {
    PAX_BUF_CHECK(buf);
    if (!pax_do_draw_col(buf, color))
        return;

    // Simplify the angles slightly.
    float a2  = fmodf(a0, M_PI * 2);
    a1       += a2 - a0;
    a0        = a2;
    if (a1 < a0)
        PAX_SWAP(float, a0, a1);
    if (a1 - a0 > M_PI * 2) {
        a1 = M_PI * 2;
        a0 = 0;
    }

    // Pick an appropriate number of divisions.
    int n_div = pax_pick_arc_divs(&buf->stack_2d.value, r, a0, a1);

    // Get the sine and cosine of one division, used for rotation in the loop.
    float div_angle = (a1 - a0) / n_div;
    float c_sin     = sinf(div_angle);
    float c_cos     = cosf(div_angle);

    // Start with a unit vector according to a0.
    float x0 = cosf(a0);
    float y0 = sinf(a0);

    // Draw it as a series of triangles, rotating with what is essentially matrix multiplication.
    for (int i = 0; i < n_div; i++) {
        // Perform the rotation.
        float x1 = x0 * c_cos - y0 * c_sin;
        float y1 = x0 * c_sin + y0 * c_cos;
        // We subtract y0 and y1 from y because our up is -y.
        pax_draw_tri(buf, color, x, y, x + x0 * r, y - y0 * r, x + x1 * r, y - y1 * r);
        // Assign the newly rotated vectors.
        x0 = x1;
        y0 = y1;
    }
}

// Draw a circle.
void pax_draw_circle(pax_buf_t *buf, pax_col_t color, float x, float y, float r) {
    PAX_BUF_CHECK(buf);
    if (!pax_do_draw_col(buf, color))
        return;

    // Use precalcualted circles for speed because the user can't tell anyway.
    pax_vec2f const *preset;
    pax_trif const  *uv_set;
    size_t           size = pax_pick_circle(&buf->stack_2d.value, r, &preset, &uv_set);

    // Use the builtin matrix stuff to our advantage.
    pax_push_2d(buf);
    pax_apply_2d(buf, matrix_2d_translate(x, y));
    pax_apply_2d(buf, matrix_2d_scale(r, r));
    // Plot all the triangles in the ROM.
    for (size_t i = 0; i < size - 1; i++) {
        pax_draw_tri(buf, color, preset[0].x, preset[0].y, preset[i].x, preset[i].y, preset[i + 1].x, preset[i + 1].y);
    }
    pax_pop_2d(buf);
}



/* ======= DRAWING: SIMPLE ======= */

// Fill the background.
PAX_PERF_CRITICAL_ATTR void pax_background(pax_buf_t *buf, pax_col_t color) {
    PAX_BUF_CHECK(buf);

#if PAX_COMPILE_MCR
    pax_join();
#endif

    uint32_t value;
    if (PAX_IS_PALETTE(buf->type)) {
        if (color > buf->palette_size)
            value = 0;
        else
            value = color;
    } else {
        value = buf->col2buf(buf, color);
    }

    if (value == 0) {
        memset(buf->buf, 0, PAX_BUF_CALC_SIZE(buf->width, buf->height, buf->type));
    } else if (buf->bpp == 16) {
        if (buf->reverse_endianness) {
            value = pax_rev_endian_16(value);
        }
        // Fill 16bpp parts.
        for (size_t i = 0; i < buf->width * buf->height; i++) {
            buf->buf_16bpp[i] = value;
        }
    } else if (buf->bpp == 32) {
        if (buf->reverse_endianness) {
            value = pax_rev_endian_32(value);
        }
        // Fill 32bpp parts.
        for (size_t i = 0; i < buf->width * buf->height; i++) {
            buf->buf_32bpp[i] = value;
        }
    } else {
        // Fill <=8bpp parts.
        if (buf->bpp == 1)
            value = -value;
        else if (buf->bpp == 2)
            value = value * 0x55;
        else if (buf->bpp == 4)
            value = value * 0x11;
        size_t limit = (7 + buf->width * buf->height * buf->bpp) / 8;
        for (size_t i = 0; i < limit; i++) {
            buf->buf_8bpp[i] = value;
        }
    }

    pax_mark_dirty0(buf);
}

// Draw a rectangle, ignoring matrix transform.
void pax_simple_rect(pax_buf_t *buf, pax_col_t color, float x, float y, float width, float height) {
    PAX_BUF_CHECK(buf);
    if (!pax_do_draw_col(buf, color))
        return;

#if PAX_COMPILE_ORIENTATION
    // Do rotation.
    pax_rectf tmp = pax_orient_det_rectf(buf, (pax_rectf){x, y, width, height});
    x             = tmp.x;
    y             = tmp.y;
    width         = tmp.w;
    height        = tmp.h;
#endif

    // Mark dirty area.
    pax_mark_dirty2(buf, x - 0.5, y - 0.5, width + 1, height + 1);
#if PAX_COMPILE_MCR
    if (pax_do_multicore) {
        // Assign worker task.
        pax_task_t task
            = {.buffer     = buf,
               .type       = PAX_TASK_RECT,
               .color      = color,
               .use_shader = false,
               .shape      = {x, y, width, height},
               .shape_len  = 4};
        paxmcr_add_task(&task);
        // Draw our part.
        paxmcr_rect_unshaded(false, buf, color, x, y, width, height);
    } else
#endif
    {
        pax_rect_unshaded(buf, color, x, y, width, height);
    }
}

// Draw a line, ignoring matrix transform.
void pax_simple_line(pax_buf_t *buf, pax_col_t color, float x0, float y0, float x1, float y1) {
    PAX_BUF_CHECK(buf);
    if (!pax_do_draw_col(buf, color))
        return;

    if (!isfinite(x0) || !isfinite(y0) || !isfinite(x1) || !isfinite(y1)) {
        // We can't draw to infinity.
        pax_last_error = PAX_ERR_INF;
        return;
    }

#if PAX_COMPILE_ORIENTATION
    // Rotate points.
    pax_vec1_t tmp = pax_orient_det_vec2f(buf, (pax_vec2f){x0, y0});
    x0             = tmp.x;
    y0             = tmp.y;
    tmp            = pax_orient_det_vec2f(buf, (pax_vec2f){x1, y1});
    x1             = tmp.x;
    y1             = tmp.y;
#endif

    pax_mark_dirty1(buf, x0, y0);
    pax_mark_dirty1(buf, x1, y1);
#if PAX_COMPILE_MCR
    // Because a line isn't drawn in alternating scanlines, we need to sync up with the worker.
    pax_join();
#endif
    pax_line_unshaded(buf, color, x0, y0, x1, y1);
}

// Draw a triangle, ignoring matrix transform.
void pax_simple_tri(pax_buf_t *buf, pax_col_t color, float x0, float y0, float x1, float y1, float x2, float y2) {
    PAX_BUF_CHECK(buf);
    if (!pax_do_draw_col(buf, color))
        return;

    if (!isfinite(x0) || !isfinite(y0) || !isfinite(x1) || !isfinite(y1) || !isfinite(x2) || !isfinite(y2)) {
        // We can't draw to infinity.
        pax_last_error = PAX_ERR_INF;
        return;
    }

    if ((y2 == y0 && y1 == y0) || (x2 == x0 && x1 == x0)) {
        // We can't draw a flat triangle.
        return;
    }

#if PAX_COMPILE_ORIENTATION
    // Rotate points.
    pax_vec1_t tmp = pax_orient_det_vec2f(buf, (pax_vec2f){x0, y0});
    x0             = tmp.x;
    y0             = tmp.y;
    tmp            = pax_orient_det_vec2f(buf, (pax_vec2f){x1, y1});
    x1             = tmp.x;
    y1             = tmp.y;
    tmp            = pax_orient_det_vec2f(buf, (pax_vec2f){x2, y2});
    x2             = tmp.x;
    y2             = tmp.y;
#endif

    // Mark all points as dirty
    pax_mark_dirty1(buf, x0 - 0.5, y0 - 0.5);
    pax_mark_dirty1(buf, x1 - 0.5, y1 - 0.5);
    pax_mark_dirty1(buf, x2 - 0.5, y2 - 0.5);
    pax_mark_dirty1(buf, x0 + 0.5, y0 + 0.5);
    pax_mark_dirty1(buf, x1 + 0.5, y1 + 0.5);
    pax_mark_dirty1(buf, x2 + 0.5, y2 + 0.5);

#if PAX_COMPILE_MCR
    if (pax_do_multicore) {
        // Add worker task.
        pax_task_t task
            = {.buffer     = buf,
               .type       = PAX_TASK_TRI,
               .color      = color,
               .use_shader = false,
               .shape      = {x0, y0, x1, y1, x2, y2},
               .shape_len  = 6};
        paxmcr_add_task(&task);
        // Draw our part.
        paxmcr_tri_unshaded(false, buf, color, x0, y0, x1, y1, x2, y2);
    } else
#endif
    {
        pax_tri_unshaded(buf, color, x0, y0, x1, y1, x2, y2);
    }
}

// Draw a arc, ignoring matrix transform.
// Angles in radians.
void pax_simple_arc(pax_buf_t *buf, pax_col_t color, float x, float y, float r, float a0, float a1) {
    PAX_BUF_CHECK(buf);
    if (!pax_do_draw_col(buf, color))
        return;

    // Simplify the angles slightly.
    float a2  = fmodf(a0, M_PI * 2);
    a1       += a2 - a0;
    a0        = a2;
    if (a1 < a0)
        PAX_SWAP(float, a0, a1);
    if (a1 - a0 > M_PI * 2) {
        a1 = M_PI * 2;
        a0 = 0;
    }

    // Pick an appropriate number of divisions.
    int n_div;
    if (r > 30)
        n_div = (a1 - a0) / M_PI * 32 + 1;
    if (r > 20)
        n_div = (a1 - a0) / M_PI * 16 + 1;
    else
        n_div = (a1 - a0) / M_PI * 8 + 1;

    // Get the sine and cosine of one division, used for rotation in the loop.
    float div_angle = (a1 - a0) / n_div;
    float c_sin     = sinf(div_angle);
    float c_cos     = cosf(div_angle);

    // Start with a unit vector according to a0.
    float x0 = cosf(a0);
    float y0 = sinf(a0);

    // Draw it as a series of triangles, rotating with what is essentially matrix multiplication.
    for (int i = 0; i < n_div; i++) {
        // Perform the rotation.
        float x1 = x0 * c_cos - y0 * c_sin;
        float y1 = x0 * c_sin + y0 * c_cos;
        // We subtract y0 and y1 from y because our up is -y.
        pax_simple_tri(buf, color, x, y, x + x0 * r, y - y0 * r, x + x1 * r, y - y1 * r);
        // Assign them yes.
        x0 = x1;
        y0 = y1;
    }
}

// Draw a circle, ignoring matrix transform.
void pax_simple_circle(pax_buf_t *buf, pax_col_t color, float x, float y, float r) {
    pax_simple_arc(buf, color, x, y, r, 0, M_PI * 2);
}
