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

typedef struct pax_bmp_mono_args {
	pax_font_t       *font;
	pax_font_range_t *range;
	char              glyph;
	size_t            glyph_index;
	size_t            glyph_y_mul;
} pax_bmp_mono_args_t;

// Texture shader for monospace bitmap fonts.
pax_col_t pax_shader_font_bitmap_mono(pax_col_t tint, int x, int y, float u, float v, void *args);

/* ========== TEXTURES =========== */

// Create a shader_t of the given texture.
// Texture format is pax_but_t*.
#define PAX_SHADER_TEXTURE(texture) (pax_shader_t) { .callback = pax_shader_texture, .callback_args = texture, .alpha_promise_0=true, .alpha_promise_255=false }
// Texture shader. No interpolation.
pax_col_t pax_shader_texture(pax_col_t tint, int x, int y, float u, float v, void *args);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //PAX_SHADERS_H
