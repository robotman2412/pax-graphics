
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

// Texture shader for bitmap fonts on palette buffers.
pax_col_t pax_shader_font_bmp_pal(pax_col_t tint, pax_col_t existing, int x, int y, float u, float v, void *args);

// Texture shader for bitmap fonts.
pax_col_t pax_shader_font_bmp(pax_col_t tint, pax_col_t existing, int x, int y, float u, float v, void *args);

// Texture shader for bitmap fonts with linear interpolation.
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
