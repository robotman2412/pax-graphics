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

#include "pax_shaders.h"

pax_col_t pax_shader_font_bitmap_uni(pax_col_t tint, int x, int y, float u, float v, void *_args) {
    pax_shader_font_bitmap_uni_args_t *args = _args;
    
	if (u < 0)      u = 0;
	else if (u > 1) u = 1;
	if (v < 0)      v = 0;
	else if (v > 1) v = 1;
	
    int glyph_x = u * args->font->glyphs_uni_w;
    int glyph_y = v * args->font->glyphs_uni_h;
    size_t glyph_y_mul = (args->font->glyphs_uni_w + 7) / 8;
    size_t glyph_len   = glyph_y_mul * args->font->glyphs_uni_h;
    size_t glyph_index = glyph_len * args->glyph + glyph_x / 8 + glyph_y_mul * glyph_y;
    
    return args->font->glyphs_uni[glyph_index] & (1 << (glyph_x & 7)) ? tint : 0;
}

pax_col_t pax_shader_texture(pax_col_t tint, int x, int y, float u, float v, void *args) {
	if (!args) {
		return (u < 0.5) ^ (v >= 0.5) ? 0xffff00ff : 0xff1f1f1f;
	}
	pax_buf_t *image = (pax_buf_t *) args;
	return pax_get_pixel(image, u*image->width, v*image->height);
}
