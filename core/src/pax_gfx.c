
// SPDX-License-Identifier: MIT

#include "pax_internal.h"

#include <malloc.h>
#include <string.h>



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
    int n_desc = sizeof(desc) / sizeof(char *);
    if (error > 0 || -error > n_desc)
        return unknown;
    else
        return desc[-error];
}



/* ======= DRAWING HELPERS ======= */

// A wrapper callback to support V0 shader callbacks.
static pax_col_t
    pax_shader_wrapper_for_v0(pax_col_t tint, pax_col_t existing, int x, int y, float u, float v, void *args0) {
    pax_shader_t        *args = args0;
    pax_shader_func_v0_t v0   = args->callback;
    return pax_col_merge(existing, v0(tint, x, y, u, v, args->callback_args));
}

// Gets the correct callback function for the shader.
pax_shader_ctx_t pax_get_shader_ctx(pax_buf_t *buf, pax_col_t color, pax_shader_t const *shader) {
    (void)buf;
    (void)color;

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

// Initialize a buffer where the `pax_buf_t` struct is user-managed.
// If `mem` is `NULL`, a new area is allocated.
// WARNING: Only use this function if you know what you're doing!
bool pax_buf_init(pax_buf_t *buf, void *mem, int width, int height, pax_buf_type_t type) {
    bool use_alloc = !mem;
    if (use_alloc) {
        // Allocate the right amount of bytes.
        mem = malloc(pax_buf_calc_size_dynamic(width, height, type));
        if (!mem) {
            pax_set_err(PAX_ERR_NOMEM);
            free(buf);
            return false;
        }
    }
    *buf = (pax_buf_t){
        // Buffer size information.
        .type               = type,
        .buf                = mem,
        .width              = width,
        .height             = height,
        .type_info          = pax_buf_type_info(type),
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
    pax_set_ok();
    return true;
}

// De-initialize a buffer initialized by `pax_buf_init`.
// WARNING: Only use this function if you know what you're doing!
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
}

// Set the palette for buffers with palette types.
// Creates an internal copy of the palette.
void pax_buf_set_palette(pax_buf_t *buf, pax_col_t const *palette, size_t palette_len) {
    PAX_BUF_CHECK(buf);
    if (buf->type_info.fmt_type != PAX_BUF_SUBTYPE_PALETTE) {
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
    if (buf->type_info.fmt_type != PAX_BUF_SUBTYPE_PALETTE) {
        PAX_ERROR(PAX_ERR_UNSUPPORTED);
    }
    if (buf->do_free_pal) {
        free((pax_col_t *)buf->palette);
        buf->do_free_pal = false;
    }
    buf->palette      = palette;
    buf->palette_size = palette_len;
}

// Get the palette for buffers with palette types.
pax_col_t const *pax_buf_get_palette(pax_buf_t *buf, size_t *palette_len) {
    PAX_BUF_CHECK(buf, NULL);
    if (buf->type_info.fmt_type != PAX_BUF_SUBTYPE_PALETTE) {
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

// Retrieve the width of the buffer without applying orientation.
int pax_buf_get_width_raw(pax_buf_t const *buf) {
    PAX_BUF_CHECK(buf, 0);
    return buf->width;
}

// Retrieve the height of the buffer without applying orientation.
int pax_buf_get_height_raw(pax_buf_t const *buf) {
    PAX_BUF_CHECK(buf, 0);
    return buf->height;
}

// Retrieve dimensions of the buffer without applying orientation.
pax_vec2i pax_buf_get_dims_raw(pax_buf_t const *buf) {
    PAX_BUF_CHECK(buf, (pax_vec2i){0, 0});
    return (pax_vec2i){buf->width, buf->height};
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
    return pax_buf_calc_size_dynamic(buf->width, buf->height, buf->type);
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
    PAX_BUF_CHECK(buf, (pax_recti){0, 0, 0, 0});
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
    PAX_BUF_CHECK(buf, (pax_recti){0, 0, 0, 0});
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


// Color error function.
static uint32_t col_error(pax_col_t _a, pax_col_t _b) {
    pax_col_union_t a = {.col = _a};
    pax_col_union_t b = {.col = _b};
    return abs(a.a - b.a) * 4 + abs(a.r - b.r) + abs(a.g - b.g) + abs(a.b - b.b);
}

// Finds the closes color in a palette.
size_t pax_closest_in_palette(pax_col_t const *palette, size_t palette_size, pax_col_t color) {
    size_t   closest_index = 0;
    uint32_t closest_err   = UINT32_MAX;
    for (size_t i = 0; i < palette_size; i++) {
        uint32_t err = col_error(palette[i], color);
        if (err < closest_err) {
            closest_index = i;
            closest_err   = err;
        }
    }
    return closest_index;
}
