
// SPDX-License-Identifier: MIT

#include "pax_shaders.h"

#include "pax_internal.h"

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
    return pax_col_merge(existing, tint);
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
    return pax_col_merge(existing, tint);
}



// Texture shader without interpolation.
pax_col_t pax_shader_texture(pax_col_t tint, pax_col_t existing, int x, int y, float u, float v, void *args) {
    (void)x;
    (void)y;
    // Pointer cast to texture thingy.
    pax_buf_t const *image = (pax_buf_t const *)args;
    // Simply get a pixel.
    pax_col_t        color = pax_get_pixel(image, u * image->width, v * image->height);
    // And return it.
    if (tint != 0xffffffff) {
        color = pax_col_tint(color, tint);
    }
    if ((color | 0x00ffffff) == 0xffffffff) {
        return pax_col_merge(existing, color);
    } else {
        return color;
    }
}

// Texture shader with interpolation.
pax_col_t pax_shader_texture_aa(pax_col_t tint, pax_col_t existing, int x, int y, float u, float v, void *args) {
    (void)x;
    (void)y;
    // Pointer cast to texture thingy.
    pax_buf_t const *image = (pax_buf_t const *)args;

    // Remap UVs.
    u *= image->width;
    v *= image->height;
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
    pax_col_union_t col0 = {.col = pax_get_pixel(image, tex_x, tex_y)};
    pax_col_union_t col1 = {.col = pax_get_pixel(image, tex_x + 1, tex_y)};
    pax_col_union_t col2 = {.col = pax_get_pixel(image, tex_x + 1, tex_y + 1)};
    pax_col_union_t col3 = {.col = pax_get_pixel(image, tex_x, tex_y + 1)};

    // Compute interpolation coefficients.
    uint_fast32_t coeffx = dx * 256;
    uint_fast32_t coeffy = dy * 256;
    uint_fast32_t coeff0 = (256 - coeffx) * (256 - coeffy);
    uint_fast32_t coeff1 = coeffx * (256 - coeffy);
    uint_fast32_t coeff2 = coeffx * coeffy;
    uint_fast32_t coeff3 = (256 - coeffx) * coeffy;

    pax_col_union_t col_out;

    // Interpolate alpha.
    bool do_alpha = (col0.a & col1.a & col2.a & col3.a) != 255;
    if (do_alpha) {
        col_out.a = (col0.a * coeff0 + col1.a * coeff1 + col2.a * coeff2 + col3.a * coeff3) >> 16;
    } else {
        col_out.a = 255;
    }
    // Interpolate RGB.
    col_out.r       = (col0.r * coeff0 + col1.r * coeff1 + col2.r * coeff2 + col3.r * coeff3) >> 16;
    col_out.g       = (col0.g * coeff0 + col1.g * coeff1 + col2.g * coeff2 + col3.g * coeff3) >> 16;
    col_out.b       = (col0.b * coeff0 + col1.b * coeff1 + col2.b * coeff2 + col3.b * coeff3) >> 16;
    pax_col_t color = col_out.col;

    // And return it.
    if (tint != 0xffffffff) {
        color = pax_col_tint(color, tint);
    }
    if (do_alpha) {
        return pax_col_merge(existing, color);
    } else {
        return color;
    }
}
