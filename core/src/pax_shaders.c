
// SPDX-License-Identifier: MIT

#include "pax_shaders.h"

#include "pax_gfx.h"
#include "pax_internal.h"
#include "pax_types.h"

#include <stdint.h>

#if defined __GNUC__ && !defined __clang__
    #pragma GCC optimize 3
#endif

#if CONFIG_PAX_DO_BICUBIC
    // Cubic interpolation: y = -2x³ + 3x²
    #define pax_interp_value(a) (-2 * a * a * a + 3 * a * a)
#else
    // Linear interpolation: y = x
    #define pax_interp_value(a) (a)
#endif



// Sample a pixel from a bitmap font glyph.
static inline __attribute__((always_inline)) uint8_t sample_glyph(int x, int y, pax_text_rsdata_t const *rsdata) {
    // Clamp to bounds.
    if (x < 0)
        x = 0;
    else if (x >= rsdata->w)
        x = rsdata->w - 1;
    if (y < 0)
        y = 0;
    else if (y >= rsdata->h)
        y = rsdata->h - 1;
    uint8_t value = rsdata->bitmap[rsdata->row_stride * y + x * rsdata->bpp / 8];

    value  = value >> (x * rsdata->bpp % 8);
    value &= (1 << rsdata->bpp) - 1;
    value  = value * 255 / ((1 << rsdata->bpp) - 1);

    return value;
}

// Texture shader for bitmap fonts on palette type buffers.
pax_col_t pax_shader_font_bmp_pal(pax_col_t tint, pax_col_t existing, int x, int y, float u, float v, void *args0) {
    pax_text_rsdata_t const *args = args0;
    (void)x;
    (void)y;

    // Get texture coords.
    int glyph_x = u;
    int glyph_y = v;

    return sample_glyph(glyph_x, glyph_y, args) >= 128 ? tint : existing;
}

// Texture shader for bitmap fonts.
pax_col_t pax_shader_font_bmp(pax_col_t tint, pax_col_t existing, int x, int y, float u, float v, void *args0) {
    pax_text_rsdata_t const *args = args0;
    (void)x;
    (void)y;

    // Get texture coords.
    int glyph_x = u;
    int glyph_y = v;

    // Extract the pixel data.
    uint8_t value = sample_glyph(glyph_x, glyph_y, args);

    // Alpha-blend with the existing color.
    tint = (tint & 0x00ffffff) | (pax_lerp(value, 0, tint >> 24) << 24);
    return pax_col_merge_inlined(existing, tint);
}

// Texture shader for bitmap fonts with linear interpolation.
pax_col_t pax_shader_font_bmp_aa(pax_col_t tint, pax_col_t existing, int x, int y, float u, float v, void *args0) {
    pax_text_rsdata_t const *args = args0;
    (void)x;
    (void)y;

    // Correct UVs for the offset caused by filtering.
    u                -= 0.5;
    v                -= 0.5;
    // Get texture coords, round down instead of round to 0.
    int      glyph_x  = floorf(u);
    int      glyph_y  = floorf(v);
    // Get subpixel coords.
    uint16_t dx       = pax_interp_value(u - glyph_x) * 255;
    uint16_t dy       = pax_interp_value(v - glyph_y) * 255;
    dx               += dx >> 7;
    dy               += dy >> 7;

    uint8_t c0 = 0, c1 = 0, c2 = 0, c3 = 0;

    // Top left bit.
    if (glyph_x >= 0 && glyph_y >= 0) {
        c0 = sample_glyph(glyph_x, glyph_y, args);
    }

    // Top right bit.
    if (glyph_x < args->w - 1 && glyph_y >= 0) {
        c1 = sample_glyph(glyph_x + 1, glyph_y, args);
    }

    // Bottom left bit.
    if (glyph_x >= 0 && glyph_y < args->h - 1) {
        c2 = sample_glyph(glyph_x, glyph_y + 1, args);
    }

    // Bottom right bit.
    if (glyph_x < args->w - 1 && glyph_y < args->h - 1) {
        c3 = sample_glyph(glyph_x + 1, glyph_y + 1, args);
    }

    // First stage interpolation.
    uint8_t c4 = c0 + (c1 - c0) * dx / 256;
    uint8_t c5 = c2 + (c3 - c2) * dx / 256;

    // Second stage interpolation.
    uint8_t value = c4 + (c5 - c4) * dy / 256;

    // Alpha-blend with the existing color.
    tint = (tint & 0x00ffffff) | (pax_lerp(value, 0, tint >> 24) << 24);
    return pax_col_merge_inlined(existing, tint);
}



static inline __attribute__((always_inline)) int tx_calc_px_index(pax_buf_t const *buf, int x, int y) {
#if CONFIG_PAX_COMPILE_ORIENTATION
    pax_vec2i tmp = pax_orient_det_vec2i(buf, (pax_vec2i){x, y});
    x             = tmp.x;
    y             = tmp.y;
#endif

    x %= buf->width;
    y %= buf->height;
    return x + y * buf->width;
}

// Texture shader without interpolation.
pax_col_t pax_shader_texture(pax_col_t tint, pax_col_t existing, int x, int y, float u, float v, void *args) {
    (void)x;
    (void)y;
    // Pointer cast to texture thingy.
    pax_buf_t const *image = (pax_buf_t const *)args;

    // Remap UVs.
#if CONFIG_PAX_COMPILE_ORIENTATION
    if (image->orientation & 1) {
        v *= image->width;
        u *= image->height;
    } else
#endif
    {
        u *= image->width;
        v *= image->height;
    }

    // Simply get a pixel.
    int       index = tx_calc_px_index(image, u, v);
    pax_col_t color = image->buf2col(image, image->getter(image, index));
    // And return it.
    if ((color >> 24) != 255) {
        return pax_col_merge_inlined(existing, color);
    } else {
        return color;
    }
}

// A linear interpolation based only on ints.
// Coeff: 0 through 256 inclusive.
static inline __attribute__((always_inline)) uint32_t
    txaa_lerp_mask(uint32_t mask, uint32_t coeff, uint32_t from, uint32_t to) {
    from &= mask;
    to   &= mask;
    return mask & (from + ((((uint64_t)to - from) * coeff) >> 8));
}

// Texture shader with interpolation.
pax_col_t pax_shader_texture_aa(pax_col_t tint, pax_col_t existing, int x, int y, float u, float v, void *args) {
    (void)x;
    (void)y;
    // Pointer cast to texture thingy.
    pax_buf_t const *image = (pax_buf_t const *)args;

    // Remap UVs.
#if CONFIG_PAX_COMPILE_ORIENTATION
    if (image->orientation & 1) {
        v *= image->width;
        u *= image->height;
    } else
#endif
    {
        u *= image->width;
        v *= image->height;
    }
    // Correct UVs for the offset caused by filtering.
    u -= 0.5;
    v -= 0.5;

    // Get texture coords, round down instead of round to 0.
    int   tex_x = floorf(u);
    int   tex_y = floorf(v);
    // Get subpixel coords.
    float dx    = pax_interp_value(u - tex_x);
    float dy    = pax_interp_value(v - tex_y);

    // Get four pixels.
    int                idx0 = tx_calc_px_index(image, tex_x, tex_y);
    int                idx1 = tx_calc_px_index(image, tex_x + 1, tex_y);
    int                idx2 = tx_calc_px_index(image, tex_x + 1, tex_y + 1);
    int                idx3 = tx_calc_px_index(image, tex_x, tex_y + 1);
    pax_index_getter_t get  = image->getter;
    pax_col_conv_t     conv = image->buf2col;
    uint32_t           raw0 = get(image, idx0);
    uint32_t           raw1 = get(image, idx1);
    uint32_t           raw2 = get(image, idx2);
    uint32_t           raw3 = get(image, idx3);
    pax_col_t          col0 = conv(image, raw0);
    pax_col_t          col1 = conv(image, raw1);
    pax_col_t          col2 = conv(image, raw2);
    pax_col_t          col3 = conv(image, raw3);

    // Compute interpolation coefficients.
    uint32_t coeffx  = dx * 255;
    uint32_t coeffy  = dy * 255;
    coeffx          &= 255;
    coeffy          &= 255;
    coeffx          += coeffx >> 7;
    coeffy          += coeffy >> 7;

    pax_col_t col01_a = txaa_lerp_mask(0xff00ff00, coeffx, col0, col1);
    pax_col_t col32_a = txaa_lerp_mask(0xff00ff00, coeffx, col3, col2);
    pax_col_t color_a = txaa_lerp_mask(0xff00ff00, coeffy, col01_a, col32_a);
    pax_col_t col01_b = txaa_lerp_mask(0x00ff00ff, coeffx, col0, col1);
    pax_col_t col32_b = txaa_lerp_mask(0x00ff00ff, coeffx, col3, col2);
    pax_col_t color_b = txaa_lerp_mask(0x00ff00ff, coeffy, col01_b, col32_b);
    pax_col_t color   = color_a | color_b;

    // And return it.
    if ((color >> 24) != 255) {
        return pax_col_merge_inlined(existing, color);
    } else {
        return color;
    }
}
