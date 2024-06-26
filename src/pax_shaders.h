
// SPDX-License-Identifier: MIT

#ifndef PAX_SHADERS_H
#define PAX_SHADERS_H

#include "pax_fonts.h"
#include "pax_gfx.h"
#include "pax_types.h"

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

/* ============ FONTS ============ */

// Data relevant to drawing bitmap fonts.
typedef struct pax_font_bmp_args {
    // The font to be drawn.
    pax_font_t const       *font;
    // The range of the font to be drawn.
    pax_font_range_t const *range;
    // Whether or not to do antialiasing, used by the promise callback.
    bool                    do_aa;
    // The glyph to be drawn.
    uint32_t                glyph;
    // The glyph data pointer.
    uint8_t const          *bitmap;
    // The bytes per line of the glyph.
    size_t                  glyph_y_mul;
    // The width of the glyph's drawn region.
    uint8_t                 glyph_w;
    // The height of the glyph's drawn region.
    uint8_t                 glyph_h;
    // The glyph's Bits Per Pixel.
    uint8_t                 bpp;
    // The glyph's Pixels Per Byte.
    uint8_t                 ppb;
    // The bitmask representing the max value that can be stored in bpp bits.
    uint8_t                 mask;
    // The bitmask used for truncating sub-byte indices.
    uint8_t                 index_mask;
} pax_font_bmp_args_t;

// Texture shader for multi-bpp bitmap fonts on palette buffers.
pax_col_t pax_shader_font_bmp_hi_pal(pax_col_t tint, pax_col_t existing, int x, int y, float u, float v, void *args);

// Texture shader for multi-bpp bitmap fonts.
pax_col_t pax_shader_font_bmp_hi(pax_col_t tint, pax_col_t existing, int x, int y, float u, float v, void *args);

// Texture shader for multi-bpp bitmap fonts with linear interpolation.
pax_col_t pax_shader_font_bmp_hi_aa(pax_col_t tint, pax_col_t existing, int x, int y, float u, float v, void *args);

// Texture shader for 1bpp bitmap fonts on palette buffers.
pax_col_t pax_shader_font_bmp_pal(pax_col_t tint, pax_col_t existing, int x, int y, float u, float v, void *args);

// Texture shader for 1bpp bitmap fonts.
pax_col_t pax_shader_font_bmp(pax_col_t tint, pax_col_t existing, int x, int y, float u, float v, void *args);

// Texture shader for 1bpp bitmap fonts with linear interpolation.
pax_col_t pax_shader_font_bmp_aa(pax_col_t tint, pax_col_t existing, int x, int y, float u, float v, void *args);

/* ========== TEXTURES =========== */

// Create a shader_t of the given texture.
// Texture format is pax_but_t*.
#define PAX_SHADER_TEXTURE(texture)                                                                                    \
    (pax_shader_t) {                                                                                                   \
        .schema_version = 1, .schema_complement = (uint8_t)~1, .renderer_id = PAX_RENDERER_ID_SWR,                     \
        .promise_callback = NULL, .callback = (void *)pax_shader_texture_aa, .callback_args = (void *)(texture),       \
        .alpha_promise_0 = true, .alpha_promise_255 = false                                                            \
    }
// Create a shader_t of the given texture, assumes the texture is opaque.
// Texture format is pax_but_t*.
#define PAX_SHADER_TEXTURE_OP(texture)                                                                                 \
    (pax_shader_t) {                                                                                                   \
        .schema_version = 1, .schema_complement = (uint8_t)~1, .renderer_id = PAX_RENDERER_ID_SWR,                     \
        .promise_callback = NULL, .callback = (void *)pax_shader_texture_aa, .callback_args = (void *)(texture),       \
        .alpha_promise_0 = true, .alpha_promise_255 = true                                                             \
    }
// Texture shader without interpolation.
pax_col_t pax_shader_texture(pax_col_t tint, pax_col_t existing, int x, int y, float u, float v, void *args);
// Texture shader with interpolation.
pax_col_t pax_shader_texture_aa(pax_col_t tint, pax_col_t existing, int x, int y, float u, float v, void *args);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif // PAX_SHADERS_H
