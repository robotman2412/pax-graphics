/*
	MIT License

	Copyright (c) 2021-2023 Julian Scheffers

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
#include "pax_internal.h"

static inline float pax_interp_cubic(float a) {
	// Cubic interpolation: y = -2x³ + 3x²
	return -2*a*a*a + 3*a*a;
}

#if PAX_DO_BICUBIC
#define pax_interp_value pax_interp_cubic
#else
// Linear interpolation: y = x
#define pax_interp_value(a) (a)
#endif

// Texture shader for multi-bpp bitmap fonts on palette type buffers.
pax_col_t pax_shader_font_bmp_hi_pal(pax_col_t base, pax_col_t existing, int x, int y, float u, float v, void *args0) {
	pax_font_bmp_args_t *args = args0;
	
	// Extract texture coords.
	int glyph_x = u;
	int glyph_y = v;
	if (glyph_x >= args->glyph_w) glyph_x = args->glyph_w - 1;
	if (glyph_y >= args->glyph_h) glyph_y = args->glyph_h - 1;
	
	// Find byte index of pixel.
	size_t glyph_index =
		glyph_x / args->ppb
		+ args->glyph_y_mul * glyph_y;
	
	// Extract pixel data.
	uint16_t val =
		// Get bitmap at byte index.
		(args->bitmap[glyph_index]
		// Align the bits.
			>> args->bpp * (glyph_x & (args->mask)))
		// Mask out the ones we want.
		& args->mask;
	
	// Compute alpha.
	uint8_t alpha = val * 255 / (args->mask);
	
	// Pick the output value.
	return alpha <= 127 ? existing : base;
}

// Texture shader for multi-bpp bitmap fonts.
pax_col_t pax_shader_font_bmp_hi(pax_col_t base, pax_col_t existing, int x, int y, float u, float v, void *args0) {
	pax_font_bmp_args_t *args = args0;
	
	// Extract texture coords.
	int glyph_x = u;
	int glyph_y = v;
	if (glyph_x >= args->glyph_w) glyph_x = args->glyph_w - 1;
	if (glyph_y >= args->glyph_h) glyph_y = args->glyph_h - 1;
	
	// Find byte index of pixel.
	size_t glyph_index =
		glyph_x / args->ppb
		+ args->glyph_y_mul * glyph_y;
	
	// Extract pixel data.
	uint16_t val =
		// Get bitmap at byte index.
		(args->bitmap[glyph_index]
		// Align the bits.
			>> args->bpp * (glyph_x & (args->mask)))
		// Mask out the ones we want.
		& args->mask;
	
	// Compute alpha.
	uint8_t alpha = val * 255 / (args->mask);
	
	// Interpolate the output color.
	uint8_t coeff8 = pax_lerp(alpha, 0, base >> 24);
	return pax_col_lerp(coeff8, existing | 0xff000000, base);
}

// Texture shader for multi-bpp bitmap fonts with linear interpolation.
pax_col_t pax_shader_font_bmp_hi_aa(pax_col_t base, pax_col_t existing, int x, int y, float u, float v, void *args0) {
	pax_font_bmp_args_t *args = args0;
	
	// Correct UVs for the offset caused by filtering.
	u -= 0.5;
	v -= 0.5;
	// Get texture coords, round down instead of round to 0.
	int glyph_x = (float) (u + 1);
	int glyph_y = (float) (v + 1);
	glyph_x --;
	glyph_y --;
	// Get subpixel coords.
	float dx = pax_interp_value(u - glyph_x);
	float dy = pax_interp_value(v - glyph_y);
	
	// Coords out of bounds fix.
	if (glyph_x >= args->glyph_w) glyph_x = args->glyph_w - 1;
	if (glyph_y >= args->glyph_h) glyph_y = args->glyph_h - 1;
	
	// Find byte indicies for four neighbouring pixels.
	size_t glyph_index_0 = glyph_x / args->ppb + args->glyph_y_mul * glyph_y;
	glyph_y ++;
	size_t glyph_index_2 = glyph_x / args->ppb + args->glyph_y_mul * glyph_y;
	glyph_x ++;
	size_t glyph_index_3 = glyph_x / args->ppb + args->glyph_y_mul * glyph_y;
	glyph_y --;
	size_t glyph_index_1 = glyph_x / args->ppb + args->glyph_y_mul * glyph_y;
	glyph_x --;
	
	const uint8_t *arr = args->bitmap;
	uint8_t glyph_bit_0 = 0;
	uint8_t glyph_bit_1 = 0;
	uint8_t glyph_bit_2 = 0;
	uint8_t glyph_bit_3 = 0;
	
	// Top left bit.
	if (glyph_x >= 0 && glyph_y >= 0) {
		glyph_bit_0 = (arr[glyph_index_0] >> args->bpp * (glyph_x & (args->mask))) & args->mask;
	}
	
	// Top right bit.
	if (glyph_x < args->glyph_w - 1 && glyph_y >= 0) {
		glyph_bit_1 = (arr[glyph_index_1] >> args->bpp * ((glyph_x + 1) & (args->mask))) & args->mask;
	}
	
	// Bottom left bit.
	if (glyph_x >= 0 && glyph_y < args->glyph_h - 1) {
		glyph_bit_2 = (arr[glyph_index_2] >> args->bpp * (glyph_x & (args->mask))) & args->mask;
	}
	
	// Bottom right bit.
	if (glyph_x < args->glyph_w - 1 && glyph_y < args->glyph_h - 1) {
		glyph_bit_3 = (arr[glyph_index_3] >> args->bpp * ((glyph_x + 1) & (args->mask))) & args->mask;
	}
	
	// Turn them into floats.
	float c0 = glyph_bit_0 / (float) args->mask;
	float c1 = glyph_bit_1 / (float) args->mask;
	float c2 = glyph_bit_2 / (float) args->mask;
	float c3 = glyph_bit_3 / (float) args->mask;
	
	// First stage interpolation.
	float c4 = c0 + (c1 - c0) * dx;
	float c5 = c2 + (c3 - c2) * dx;
	
	// Second stage interpolation.
	float coeff = c4 + (c5 - c4) * dy;
	
	// Interpolate the output color.
	uint8_t coeff8 = coeff * (base >> 24);
	return pax_col_lerp(coeff8, existing | 0xff000000, base);
}


// Texture shader for 1bpp bitmap fonts on palette type buffers.
pax_col_t pax_shader_font_bmp_pal(pax_col_t tint, pax_col_t existing, int x, int y, float u, float v, void *args0) {
	pax_font_bmp_args_t *args = args0;
	
	// Get texture coords.
	int glyph_x = u;
	int glyph_y = v;
	// Coords out of bounds fix.
	if (glyph_x >= args->glyph_w) glyph_x = args->glyph_w - 1;
	if (glyph_y >= args->glyph_h) glyph_y = args->glyph_h - 1;
	// Get byte index of pixel.
	size_t glyph_index = glyph_x / 8 + args->glyph_y_mul * glyph_y;
	
	// Extract the pixel data.
	bool pixdat = args->bitmap[glyph_index] & (1 << (glyph_x & 7));
	return pixdat ? tint : existing;
}

// Texture shader for 1bpp bitmap fonts.
pax_col_t pax_shader_font_bmp(pax_col_t tint, pax_col_t existing, int x, int y, float u, float v, void *args0) {
	pax_font_bmp_args_t *args = args0;
	
	// Get texture coords.
	int glyph_x = u;
	int glyph_y = v;
	// Coords out of bounds fix.
	if (glyph_x >= args->glyph_w) glyph_x = args->glyph_w - 1;
	if (glyph_y >= args->glyph_h) glyph_y = args->glyph_h - 1;
	// Get byte index of pixel.
	size_t glyph_index = glyph_x / 8 + args->glyph_y_mul * glyph_y;
	
	// Extract the pixel data.
	bool pixdat = args->bitmap[glyph_index] & (1 << (glyph_x & 7));
	return pixdat ? pax_col_merge(existing, tint) : existing;
}

// Texture shader for 1bpp bitmap fonts with linear interpolation.
pax_col_t pax_shader_font_bmp_aa(pax_col_t base, pax_col_t existing, int x, int y, float u, float v, void *args0) {
	pax_font_bmp_args_t *args = args0;
	
	// Correct UVs for the offset caused by filtering.
	u -= 0.5;
	v -= 0.5;
	// Get texture coords, round down instead of round to 0.
	int glyph_x = (float) (u + 1);
	int glyph_y = (float) (v + 1);
	glyph_x --;
	glyph_y --;
	// Get subpixel coords.
	float dx = pax_interp_value(u - glyph_x);
	float dy = pax_interp_value(v - glyph_y);
	
	// Coords out of bounds fix.
	if (glyph_x >= args->glyph_w) glyph_x = args->glyph_w - 1;
	if (glyph_y >= args->glyph_h) glyph_y = args->glyph_h - 1;
	
	// Find byte indicies for four neighbouring pixels.
	size_t glyph_index_0 = glyph_x / 8 + args->glyph_y_mul * glyph_y;
	glyph_y ++;
	size_t glyph_index_2 = glyph_x / 8 + args->glyph_y_mul * glyph_y;
	glyph_x ++;
	size_t glyph_index_3 = glyph_x / 8 + args->glyph_y_mul * glyph_y;
	glyph_y --;
	size_t glyph_index_1 = glyph_x / 8 + args->glyph_y_mul * glyph_y;
	glyph_x --;
	
	const uint8_t *arr = args->bitmap;
	bool glyph_bit_0 = 0;
	bool glyph_bit_1 = 0;
	bool glyph_bit_2 = 0;
	bool glyph_bit_3 = 0;
	
	// Top left bit.
	if (glyph_x >= 0 && glyph_y >= 0) {
		glyph_bit_0 = arr[glyph_index_0] & (1 << (glyph_x & 7));
	}
	
	// Top right bit.
	if (glyph_x < args->glyph_w - 1 && glyph_y >= 0) {
		glyph_bit_1 = arr[glyph_index_1] & (1 << ((glyph_x+1) & 7));
	}
	
	// Bottom left bit.
	if (glyph_x >= 0 && glyph_y < args->glyph_h - 1) {
		glyph_bit_2 = arr[glyph_index_2] & (1 << (glyph_x & 7));
	}
	
	// Bottom right bit.
	if (glyph_x < args->glyph_w - 1 && glyph_y < args->glyph_h - 1) {
		glyph_bit_3 = arr[glyph_index_3] & (1 << ((glyph_x+1) & 7));
	}
	
	// Simple conversion to float.
	float c0 = glyph_bit_0;
	float c1 = glyph_bit_1;
	float c2 = glyph_bit_2;
	float c3 = glyph_bit_3;
	
	// First stage interpolation.
	float c4 = c0 + (c1 - c0) * dx;
	float c5 = c2 + (c3 - c2) * dx;
	
	// Second stage interpolation.
	float coeff = c4 + (c5 - c4) * dy;
	
	// Interpolate the output color.
	uint8_t coeff8 = coeff * (base >> 24);
	return pax_col_lerp(coeff8, existing | 0xff000000, base);
}



// Texture shader. No interpolation.
pax_col_t pax_shader_texture(pax_col_t tint, int x, int y, float u, float v, void *args) {
	if (!args) {
		// Make a default texture.
		return (u < 0.5) ^ (v >= 0.5) ? 0xffff00ff : 0xff1f1f1f;
	}
	
	// Pointer cast to texture thingy.
	const pax_buf_t *image = (const pax_buf_t *) args;
	// Simply get a pixel.
	pax_col_t  color = pax_get_pixel(image, u*image->width, v*image->height);
	// And return it.
	return pax_col_tint(color, tint);
}

// Texture shader. No interpolation.
pax_col_t pax_shader_texture_aa(pax_col_t tint, int x, int y, float u, float v, void *args) {
	if (!args) {
		// Make a default texture.
		return (u < 0.5) ^ (v >= 0.5) ? 0xffff00ff : 0xff1f1f1f;
	}
	
	// Pointer cast to texture thingy.
	const pax_buf_t *image = (const pax_buf_t *) args;
	
	// Remap UVs.
	u *= image->width;
	v *= image->height;
	// Correct UVs for the offset caused by filtering.
	u -= 0.5;
	v -= 0.5;
	
	// Get texture coords, round down instead of round to 0.
	int tex_x = (float) (u + 1);
	int tex_y = (float) (v + 1);
	tex_x --;
	tex_y --;
	// Get subpixel coords.
	float dx = pax_interp_value(u - tex_x);
	float dy = pax_interp_value(v - tex_y);
	
	// Get four pixels.
	pax_col_t  col0 = pax_get_pixel(image, tex_x,   tex_y);
	pax_col_t  col1 = pax_get_pixel(image, tex_x+1, tex_y);
	pax_col_t  col2 = pax_get_pixel(image, tex_x+1, tex_y+1);
	pax_col_t  col3 = pax_get_pixel(image, tex_x,   tex_y+1);
	
	// First stage interpolation.
	pax_col_t col_a = pax_col_lerp(dx*255, col0, col1);
	pax_col_t col_b = pax_col_lerp(dx*255, col3, col2);
	
	// Second stage interpolation.
	pax_col_t col   = pax_col_lerp(dy*255, col_a, col_b);
	
	// And return it.
	return pax_col_tint(col, tint);
}
