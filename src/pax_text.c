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

#include "pax_internal.h"
#include "pax_gfx.h"
#include "pax_shaders.h"
#include "string.h"



/* ===== PRIVATE TYPEDEFS ====== */
typedef struct {
	/* ---- Generic context ---- */
	// Whether or not to render text.
	// If false, only calculates size.
	bool        do_render;
	// The buffer to render to, if any.
	pax_buf_t  *buf;
	// The color to draw with.
	pax_col_t   color;
	// The on-screen position of the text.
	float       x, y;
	// The font to use.
	const pax_font_t *font;
	// The font size to use.
	float             font_size;
	
	/* ---- Rendering context ---- */
	// The glyph offset, if any.
	const uint8_t    *glyph_offs;
	// The glyph bits per pixel, if any.
	uint8_t           glyph_bpp;
	// The rendered size of the glyph.
	uint8_t           render_width, render_height;
	// The counted size of the glyph.
	uint8_t           counted_width, counted_height;
	// The rendering offset of the glyph.
	uint8_t           dx, dy;
	// The vertical offset in bits per pixel.
	uint8_t           vertical_offs;
	// Whether to do anti-aliasing and/or interpolation.
	bool              do_aa;
} pax_text_ctx_t;



/* ====== UTF-8 UTILITIES ====== */

// Extracts an UTF-8 code from a null-terminated c-string.
// Returns the new string pointer.
// Sets the decoded UTF-8 using a pointer.
// If the string terminates early or contains invalid unicode, U+FFFD is returned.
char *utf8_getch(const char *cstr, uint32_t *out) {
	char len, mask;
	if (!*cstr) {
		// Null pointer.
		*out = 0xfffd; // Something something invalid UTF8.
		return (char *) cstr;
	} else if (*cstr <= 0x7f) {
		// ASCII point.
		*out = *cstr;
		return (char *) cstr + 1;
	} else if ((*cstr & 0xe0) == 0xc0) {
		// Two byte point.
		len = 2;
		mask = 0x1f;
	} else if ((*cstr & 0xf0) == 0xe0) {
		// Three byte point.
		len = 3;
		mask = 0x0f;
	} else if ((*cstr & 0xf8) == 0xf0) {
		// Four byte point.
		len = 4;
		mask = 0x07;
	} else {
		// There are no points over four bytes long.
		*out = 0xfffd; // Something something invalid UTF8.
		return (char *) cstr;
	}
	
	*out = 0;
	for (int i = 0; i < len; i++) {
		if (!*cstr) {
			*out = 0xfffd; // Something something invalid UTF8.
			return (char *) cstr;
		}
		*out <<= 6;
		*out |= *cstr & mask;
		mask = 0x3f;
		cstr ++;
	}
	return (char *) cstr;
}

// Returns how many UTF-8 characters a given c-string contains.
size_t utf8_strlen(const char *cstr) {
	const char *end   = cstr + strlen(cstr);
	uint32_t     dummy = 0;
	size_t      len   = 0;
	while (cstr != end) {
		len ++;
		cstr = utf8_getch(cstr, &dummy);
	}
	return 0;
}



/* ======= DRAWING: TEXT ======= */

// Internal method for monospace bitmapped characters.
pax_vec1_t text_bitmap_mono(pax_text_ctx_t *ctx, const pax_font_range_t *range, uint32_t glyph) {
	if (ctx->do_render) {
		// Set up shader.
		pax_font_bmp_args_t args = {
			.font        = ctx->font,
			.range       = range,
			.glyph       = glyph,
			.glyph_y_mul = (range->bitmap_mono.width * range->bitmap_mono.bpp + 7) / 8,
			.glyph_w     = range->bitmap_mono.width,
			.glyph_h     = range->bitmap_mono.height,
			.bpp         = range->bitmap_mono.bpp,
			.ppb         = 8 / range->bitmap_mono.bpp,
		};
		args.mask        = (1 << args.bpp) - 1;
		
		size_t glyph_len = args.glyph_y_mul * range->bitmap_mono.height;
		args.glyph_index = glyph_len * (glyph - range->start);
		
		pax_shader_t shader = {
			.callback_args     = &args,
			.alpha_promise_0   = true,
			.alpha_promise_255 = false,
		};
		if (range->bitmap_mono.bpp > 1) {
			// Multi-bit per pixel impl.
			shader.callback = pax_shader_font_bmp_hi;
		} else {
			// Single bit per pixel impl.
			shader.callback = ctx->do_aa ? pax_shader_font_bmp_aa : pax_shader_font_bmp;
		}
		
		// And UVs.
		pax_quad_t uvs = {
			.x0 = 0,                        .y0 = 0,
			.x1 = range->bitmap_mono.width, .y1 = 0,
			.x2 = range->bitmap_mono.width, .y2 = range->bitmap_mono.height,
			.x3 = 0,                        .y3 = range->bitmap_mono.height,
		};
		
		// Start drawing, boy!
		pax_shade_rect(
			ctx->buf, ctx->color,
			&shader, &uvs,
			0, 0,
			range->bitmap_mono.width,
			range->bitmap_mono.height
		);
		pax_join();
	}
	
	// Size calculation is very simple.
	return (pax_vec1_t) {
		.x = range->bitmap_mono.width,
		.y = range->bitmap_mono.height
	};
}

// Internal method for variable pitch bitmapped characters.
pax_vec1_t text_bitmap_var(pax_text_ctx_t *ctx, const pax_font_range_t *range, uint32_t glyph) {
	size_t index = (glyph - range->start);
	const pax_bmpv_t *dims = &range->bitmap_var.dims[index];
	if (ctx->do_render) {
		// Set up shader.
		pax_font_bmp_args_t args = {
			.font        = ctx->font,
			.range       = range,
			.glyph       = glyph,
			.glyph_y_mul = (dims->draw_w * range->bitmap_var.bpp + 7) / 8,
			.glyph_w     = dims->draw_w,
			.glyph_h     = dims->draw_h,
			.bpp         = range->bitmap_var.bpp,
			.ppb         = 8 / range->bitmap_var.bpp,
		};
		args.mask        = (1 << args.bpp) - 1;
		args.glyph_index = dims->index;
		
		pax_shader_t shader = {
			.callback_args     = &args,
			.alpha_promise_0   = true,
			.alpha_promise_255 = false,
		};
		if (range->bitmap_var.bpp > 1) {
			// Multi-bit per pixel impl.
			shader.callback = ctx->do_aa ? pax_shader_font_bmp_hi_aa : pax_shader_font_bmp_hi;
		} else {
			// Single bit per pixel impl.
			shader.callback = ctx->do_aa ? pax_shader_font_bmp_aa : pax_shader_font_bmp;
		}
		
		// And UVs.
		pax_quad_t uvs = {
			.x0 = 0,                  .y0 = 0,
			.x1 = dims->draw_w-0.00, .y1 = 0,
			.x2 = dims->draw_w-0.00, .y2 = dims->draw_h-0.00,
			.x3 = 0,                  .y3 = dims->draw_h-0.00,
		};
		
		// Start drawing, boy!
		if (dims->draw_w && dims->draw_h) {
			pax_shade_rect(
				ctx->buf, ctx->color,
				&shader, &uvs,
				dims->draw_x,
				dims->draw_y,
				dims->draw_w,
				dims->draw_h
			);
			pax_join();
		}
	}
	
	// Size calculation is very simple.
	return (pax_vec1_t) {
		.x = dims->measured_width,
		.y = range->bitmap_var.height
	};
}

// Determines whether a character lies in a given range.
static inline bool text_range_includes(const pax_font_range_t *range, uint32_t c) {
	return c >= range->start && c <= range->end;
}

// Internal method for determining the font range to use.
// Returns NULL if not in any range.
static const pax_font_range_t *text_get_range(const pax_font_t *font, uint32_t c) {
	// Iterate over ranges.
	for (size_t i = 0; i < font->n_ranges; i++) {
		// Look for the first including the point.
		if (text_range_includes(&font->ranges[i], c)) {
			return &font->ranges[i];
		}
	}
	return NULL;
}

// Internal method for rendering text and calculating text size.
static pax_vec1_t text_generic(pax_text_ctx_t *ctx, const char *text) {
	// Sanity checks.
	if (!text || !*text) {
		return (pax_vec1_t) { .x=0, .y=0, };
	}
	// Render checks.
	if (ctx->color < 0x01000000) {
		ctx->do_render = false;
	}
	
	// Set up the fancy size crap.
	float size_mul = ctx->font_size / ctx->font->default_size;
	if (ctx->do_render)
		pax_push_2d(ctx->buf);
	if (ctx->do_render)
		pax_apply_2d(ctx->buf, matrix_2d_scale(size_mul, size_mul));
	
	float x = 0, y = 0;
	float max_x = 0;
	const pax_font_range_t *range = NULL;
	const char *limit = text + strlen(text);
	
	// Simply loop over all characters.
	while (text < limit) {
		// Get a character.
		uint32_t glyph = 0;
		text = utf8_getch(text, &glyph);
		
		// Is it a newline?
		if (glyph == '\r') {
			// Consume a '\n' if the next character is one.
			char *peek = utf8_getch(text, &glyph);
			if (glyph == '\n') text = peek;
			goto newline;
		} else if (glyph == '\n') {
			// Insert a newline.
			newline:
			if (x > max_x) max_x = x;
			if (ctx->do_render)
				pax_apply_2d(ctx->buf, matrix_2d_translate(-x, ctx->font->default_size));
			x  = 0;
			y += ctx->font->default_size;
		} else {
			// Try to find a range the glyph is in.
			if (!range || !text_range_includes(range, glyph)) {
				range = text_get_range(ctx->font, glyph);
			}
			
			pax_vec1_t dims = { .x=0, .y=0 };
			if (range) {
				// Handle the character.
				switch (range->type) {
					case PAX_FONT_TYPE_BITMAP_MONO:
						dims = text_bitmap_mono(ctx, range, glyph);
						break;
					case PAX_FONT_TYPE_BITMAP_VAR:
						dims = text_bitmap_var(ctx, range, glyph);
						break;
				}
			} else {
				// Ignore it for now.
			}
			x += dims.x;
			if (ctx->do_render)
				pax_apply_2d(ctx->buf, matrix_2d_translate(dims.x, 0));
		}
	}
	
	if (ctx->do_render)
		pax_pop_2d(ctx->buf);
	
	if (x > max_x) max_x = x;
	return (pax_vec1_t) {
		.x = size_mul * max_x,
		.y = size_mul * (y + ctx->font->default_size)
	};
}

// Draw a string with the given font and return it's size.
// Text is center-aligned.
// Size is before matrix transformation.
// If font is NULL, the default font (sky) will be used.
// Font is scaled up with method recommended by it (see pax_font_t::recommend_aa).
pax_vec1_t pax_center_text(pax_buf_t *buf, pax_col_t color, const pax_font_t *font, float font_size, float x, float y, const char *text) {
	pax_vec1_t dims = pax_text_size(font, font_size, text);
	pax_draw_text(buf, color, font, font_size, x - dims.x/2, y, text);
	return dims;
}

// Draw a string with the given font.
// If font is NULL, the default font (7x9) will be used.
pax_vec1_t pax_draw_text(pax_buf_t *buf, pax_col_t color, const pax_font_t *font, float font_size, float x, float y, const char *text) {
	if (!font) font = PAX_FONT_DEFAULT;
	pax_text_ctx_t ctx = {
		.do_render = true,
		.buf       = buf,
		.color     = color,
		.x         = x,
		.y         = y,
		.font      = font,
		.font_size = font_size,
		.do_aa     = font->recommend_aa,
	};
	pax_push_2d (buf);
	pax_apply_2d(buf, matrix_2d_translate(x, y));
	pax_vec1_t dims = text_generic(&ctx, text);
	pax_pop_2d  (buf);
	return dims;
}

// Draw a string with the given font.
// If font is NULL, the default font (7x9) will be used.
pax_vec1_t pax_draw_text_aa(pax_buf_t *buf, pax_col_t color, const pax_font_t *font, float font_size, float x, float y, const char *text) {
	if (!font) font = PAX_FONT_DEFAULT;
	pax_text_ctx_t ctx = {
		.do_render = true,
		.buf       = buf,
		.color     = color,
		.x         = x,
		.y         = y,
		.font      = font,
		.font_size = font_size,
		.do_aa     = true,
	};
	pax_push_2d (buf);
	pax_apply_2d(buf, matrix_2d_translate(x, y));
	pax_vec1_t dims = text_generic(&ctx, text);
	pax_pop_2d  (buf);
	return dims;
}

// Draw a string with the given font.
// If font is NULL, the default font (7x9) will be used.
pax_vec1_t pax_draw_text_noaa(pax_buf_t *buf, pax_col_t color, const pax_font_t *font, float font_size, float x, float y, const char *text) {
	if (!font) font = PAX_FONT_DEFAULT;
	pax_text_ctx_t ctx = {
		.do_render = true,
		.buf       = buf,
		.color     = color,
		.x         = x,
		.y         = y,
		.font      = font,
		.font_size = font_size,
		.do_aa     = false,
	};
	pax_push_2d (buf);
	pax_apply_2d(buf, matrix_2d_translate(x, y));
	pax_vec1_t dims = text_generic(&ctx, text);
	pax_pop_2d  (buf);
	return dims;
}

// Calculate the size of the string with the given font.
// Size is before matrix transformation.
// If font is NULL, the default font (7x9) will be used.
pax_vec1_t pax_text_size(const pax_font_t *font, float font_size, const char *text) {
	if (!font) font = PAX_FONT_DEFAULT;
	pax_text_ctx_t ctx = {
		.do_render = false,
		.font      = font,
		.font_size = font_size,
	};
	return text_generic(&ctx, text);
}

