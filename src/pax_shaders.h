/*
	MIT License

	Copyright (c) 2022 Julian Scheffers

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
	SOFTWARE.
*/

#ifndef PAX_SHADERS_H
#define PAX_SHADERS_H

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

#include "pax_types.h"
#include "pax_fonts.h"
#include "pax_gfx.h"

/* ============ FONTS ============ */

// Data relevant to drawing bitmap fonts.
typedef struct pax_font_bmp_args {
	// The font to be drawn.
	const pax_font_t       *font;
	// The range of the font to be drawn.
	const pax_font_range_t *range;
	// Whether or not to do antialiasing, used by the promise callback.
	bool                    do_aa;
	// The glyph to be drawn.
	uint32_t                glyph;
	// The first byte index of the glyph to be drawn.
	size_t                  glyph_index;
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
} pax_font_bmp_args_t;

// Texture shader for multi-bpp bitmap fonts.
pax_col_t pax_shader_font_bmp_hi(pax_col_t tint, int x, int y, float u, float v, void *args);

// Texture shader for multi-bpp bitmap fonts with linear interpolation.
pax_col_t pax_shader_font_bmp_hi_aa(pax_col_t tint, int x, int y, float u, float v, void *args);

// Texture shader for 1bpp bitmap fonts.
pax_col_t pax_shader_font_bmp(pax_col_t tint, int x, int y, float u, float v, void *args);

// Texture shader for 1bpp bitmap fonts with linear interpolation.
pax_col_t pax_shader_font_bmp_aa(pax_col_t tint, int x, int y, float u, float v, void *args);

/* ========== TEXTURES =========== */

// Create a shader_t of the given texture.
// Texture format is pax_but_t*.
#define PAX_SHADER_TEXTURE(texture) (pax_shader_t) { .schema_version = 0, .schema_complement = ~0, .renderer_id = PAX_RENDERER_ID_SWR, .promise_callback = NULL, .callback = pax_shader_texture_aa, .callback_args = texture, .alpha_promise_0=true, .alpha_promise_255=false }
// Create a shader_t of the given texture, assumes the texture is opaque.
// Texture format is pax_but_t*.
#define PAX_SHADER_TEXTURE(texture) (pax_shader_t) { .schema_version = 0, .schema_complement = ~0, .renderer_id = PAX_RENDERER_ID_SWR, .promise_callback = NULL, .callback = pax_shader_texture_aa, .callback_args = texture, .alpha_promise_0=true, .alpha_promise_255=true }
// Texture shader. No interpolation.
pax_col_t pax_shader_texture(pax_col_t tint, int x, int y, float u, float v, void *args);
// Texture shader with interpolation.
pax_col_t pax_shader_texture_aa(pax_col_t tint, int x, int y, float u, float v, void *args);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //PAX_SHADERS_H
