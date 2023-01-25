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

static const char *TAG = "pax_text";

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
} pax_text_render_t;



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
	uint32_t    dummy = 0;
	size_t      len   = 0;
	while (cstr != end) {
		len ++;
		cstr = utf8_getch(cstr, &dummy);
	}
	return 0;
}



/* ======= DRAWING: TEXT ======= */

uint64_t text_promise_callback(pax_buf_t *buf, pax_col_t tint, void *args0) {
	pax_font_bmp_args_t *args = args0;
	return !(tint & 0xff000000) ? PAX_PROMISE_INVISIBLE :
			(args->bpp == 1 && !args->do_aa) ? PAX_PROMISE_CUTOUT : 0;
}

// Pixel-aligned optimisation of pax_shade_rect, used for text.
static void pixel_aligned_render(pax_buf_t *buf, pax_col_t color, const pax_shader_t *shader,
		const pax_quad_t *uvs, float x, float y, float width, float height) {
	// Offset and pixel-align co-ordinates.
	x = (int) (0.5 + x + buf->stack_2d.value.a2);
	y = (int) (0.5 + y + buf->stack_2d.value.b2);
	pax_mark_dirty2(buf, x, y, width, height);
	
	// TODO: Re-work as integer UV rendererererrererer device.
	#if PAX_COMPILE_MCR
	if (pax_do_multicore) {
		// Assign worker task.
		float shape[4] = {
			x, y, width, height
		};
		// Copies are made by paxmcr_add_task.
		pax_task_t task = {
			.buffer    = buf,
			.type      = PAX_TASK_RECT,
			.color     = color,
			.shader    = (pax_shader_t *) shader,
			.quad_uvs  = (pax_quad_t *) uvs,
			.shape     = (float *) shape,
			.shape_len = 4
		};
		paxmcr_add_task(&task);
		// Draw our part.
		paxmcr_rect_shaded(
			false,
			buf, color, shader, x, y, width, height,
			uvs->x0, uvs->y0, uvs->x1, uvs->y1,
			uvs->x2, uvs->y2, uvs->x3, uvs->y3
		);
	} else
	#endif
	{
		// Single core option.
		pax_rect_shaded(
			buf, color, shader, x, y, width, height,
			uvs->x0, uvs->y0, uvs->x1, uvs->y1,
			uvs->x2, uvs->y2, uvs->x3, uvs->y3
		);
	}
}

// Internal method for monospace bitmapped characters.
pax_vec2f text_bitmap_mono(pax_text_render_t *ctx, const pax_font_range_t *range, uint32_t glyph) {
	if (ctx->do_render && glyph != 0x20) {
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
			.do_aa       = ctx->do_aa,
		};
		args.mask        = (1 << args.bpp) - 1;
		
		size_t glyph_len = args.glyph_y_mul * range->bitmap_mono.height;
		args.glyph_index = glyph_len * (glyph - range->start);
		
		pax_shader_t shader = {
			.schema_version    =  1,
			.schema_complement = ~1,
			.renderer_id       = PAX_RENDERER_ID_SWR,
			.promise_callback  = text_promise_callback,
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
		if (matrix_2d_is_identity1(ctx->buf->stack_2d.value)) {
			// Pixel-aligned instead of float optimisation/fix.
			pixel_aligned_render(
				ctx->buf, ctx->color,
				&shader, &uvs,
				0,
				0,
				range->bitmap_mono.width,
				range->bitmap_mono.height
			);
		} else {
			pax_shade_rect(
				ctx->buf, ctx->color,
				&shader, &uvs,
				0,
				0,
				range->bitmap_mono.width,
				range->bitmap_mono.height
			);
		}
		pax_join();
	}
	
	// Size calculation is very simple.
	return (pax_vec2f) {
		.x = range->bitmap_mono.width,
		.y = range->bitmap_mono.height
	};
}

// Internal method for variable pitch bitmapped characters.
pax_vec2f text_bitmap_var(pax_text_render_t *ctx, const pax_font_range_t *range, uint32_t glyph) {
	size_t index = (glyph - range->start);
	const pax_bmpv_t *dims = &range->bitmap_var.dims[index];
	if (ctx->do_render && glyph != 0x20) {
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
			.do_aa       = ctx->do_aa,
		};
		args.mask        = (1 << args.bpp) - 1;
		args.glyph_index = dims->index;
		
		pax_shader_t shader = {
			.schema_version    =  1,
			.schema_complement = ~1,
			.renderer_id       = PAX_RENDERER_ID_SWR,
			.promise_callback  = text_promise_callback,
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
			.x0 = 0,            .y0 = 0,
			.x1 = dims->draw_w, .y1 = 0,
			.x2 = dims->draw_w, .y2 = dims->draw_h,
			.x3 = 0,            .y3 = dims->draw_h,
		};
		
		// Start drawing, boy!
		if (dims->draw_w && dims->draw_h) {
			if (matrix_2d_is_identity1(ctx->buf->stack_2d.value)) {
				// Pixel-aligned instead of float optimisation/fix.
				pixel_aligned_render(
					ctx->buf, ctx->color,
					&shader, &uvs,
					dims->draw_x,
					dims->draw_y,
					dims->draw_w,
					dims->draw_h
				);
			} else {
				pax_shade_rect(
					ctx->buf, ctx->color,
					&shader, &uvs,
					dims->draw_x,
					dims->draw_y,
					dims->draw_w,
					dims->draw_h
				);
			}
			pax_join();
		}
	}
	
	// Size calculation is very simple.
	return (pax_vec2f) {
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
static pax_vec2f text_generic(pax_text_render_t *ctx, const char *text) {
	// Sanity checks.
	if (!text || !*text) {
		return (pax_vec2f) { .x=0, .y=0, };
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
			if (glyph == 0xa0) {
				// Non-breaking space is implicitly converted to space.
				glyph = 0x20;
			}
			
			// Try to find a range the glyph is in.
			if (!range || !text_range_includes(range, glyph)) {
				range = text_get_range(ctx->font, glyph);
			}
			
			pax_vec2f dims = { .x=0, .y=0 };
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
	return (pax_vec2f) {
		.x = size_mul * max_x,
		.y = size_mul * (y + ctx->font->default_size)
	};
}

// A single line of `pax_center_text`.
static pax_vec2f pax_center_line(pax_buf_t *buf, pax_col_t color, const pax_font_t *font, float font_size, float x, float y, const char *text) {
	pax_vec2f dims = pax_text_size(font, font_size, text);
	pax_draw_text(buf, color, font, font_size, x-dims.x/2.0f, y, text);
	return dims;
}

// Draw a string with the given font and return it's size.
// Text is center-aligned on every line.
// Size is before matrix transformation.
pax_vec2f pax_center_text(pax_buf_t *buf, pax_col_t color, const pax_font_t *font, float font_size, float x, float y, const char *text) {
	float width = 0, height = 0;
	
	// Allocate a buffer since we might go splitting up lines.
	char *buffer = malloc(strlen(text) + 1);
	
	// Continuously look for newlines.
	while (*text) {
		// Look for newline characters.
		char *cr = strchr(text, '\r');
		char *lf = strchr(text, '\n');
		// Determine which comes earlier.
		char *found = cr && (!lf || cr < lf) ? cr : lf;
		// Determine where to keep going afterwards.
		char *next  = cr && lf && (lf == cr + 1) ? lf+1 : found+1;
		
		if (!cr && !lf) {
			// Nothing special left to do.
			pax_center_line(buf, color, font, font_size, x, y+height, text);
			break;
			
		} else {
			// Copy string segment into buffer.
			memcpy(buffer, text, found-text);
			buffer[found-text] = 0;
			
			// Draw this line individually.
			pax_vec2f dims = pax_center_line(buf, color, font, font_size, x, y+height, buffer);
			if (dims.x > width) width = dims.x;
			height += dims.y;
			
			// Set text pointer to next line.
			text = next;
		}
	}
	
	// Clean up and return total size.
	free(buffer);
	return (pax_vec2f) {width, height};
}

// Draw a string with the given font and return it's size.
// Size is before matrix transformation.
pax_vec2f pax_draw_text(pax_buf_t *buf, pax_col_t color, const pax_font_t *font, float font_size, float x, float y, const char *text) {
	if (!font) PAX_ERROR1("pax_draw_text(font: NULL)", PAX_ERR_PARAM, (pax_vec2f){0});
	pax_text_render_t ctx = {
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
	pax_vec2f dims = text_generic(&ctx, text);
	pax_pop_2d  (buf);
	return dims;
}

// DEPRECATION NOTICE: This function is subject to be removed
// Draw a string with the given font and return it's size.
// Size is before matrix transformation.
// Font is scaled up with interpolation, overriding it's default.
// If font is NULL, the default font (7x9) will be used.
pax_vec2f pax_draw_text_aa(pax_buf_t *buf, pax_col_t color, const pax_font_t *font, float font_size, float x, float y, const char *text) {
	if (!font) PAX_ERROR1("pax_draw_text(font: NULL)", PAX_ERR_PARAM, (pax_vec2f){0});
	pax_text_render_t ctx = {
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
	pax_vec2f dims = text_generic(&ctx, text);
	pax_pop_2d  (buf);
	return dims;
}

// DEPRECATION NOTICE: This function is subject to be removed
// Draw a string with the given font and return it's size.
// Size is before matrix transformation.
// Font is scaled up without interpolation, overriding it's default.
pax_vec2f pax_draw_text_noaa(pax_buf_t *buf, pax_col_t color, const pax_font_t *font, float font_size, float x, float y, const char *text) {
	if (!font) PAX_ERROR1("pax_draw_text(font: NULL)", PAX_ERR_PARAM, (pax_vec2f){0});
	pax_text_render_t ctx = {
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
	pax_vec2f dims = text_generic(&ctx, text);
	pax_pop_2d  (buf);
	return dims;
}

// Calculate the size of the string with the given font.
// Size is before matrix transformation.
// If font is NULL, the default font (7x9) will be used.
pax_vec2f pax_text_size(const pax_font_t *font, float font_size, const char *text) {
	if (!font) PAX_ERROR1("pax_draw_text(font: NULL)", PAX_ERR_PARAM, (pax_vec2f){0});
	pax_text_render_t ctx = {
		.do_render = false,
		.font      = font,
		.font_size = font_size,
	};
	return text_generic(&ctx, text);
}



#if 1
// Calculates the size of the region's raw data.
static size_t pax_calc_range_size(const pax_font_range_t *range, bool include_structs) {
	size_t range_size = range->end - range->start + 1;
	size_t size = include_structs ? sizeof(pax_font_range_t) : 0;
	if (range->type == PAX_FONT_TYPE_BITMAP_MONO) {
		// Based on array CALCULATION.
		size_t bytes_per_line = (range->bitmap_mono.width * range->bitmap_mono.bpp + 7) / 8;
		size += range_size * range->bitmap_mono.height * bytes_per_line;
		return size;
		
	} else {
		// More complex; based on last index.
		pax_bmpv_t max_index = {.index = 0};
		
		// Find glyph with highest index.
		for (size_t i = 0; i < range_size; i++) {
			size_t index = range->bitmap_var.dims[i].index;
			if (index > max_index.index) max_index = range->bitmap_var.dims[i];
		}
		
		// Calculate length.
		size_t bytes_per_line = (max_index.draw_w * range->bitmap_var.bpp + 7) / 8;
		size += max_index.index + bytes_per_line * max_index.draw_h;
		
		if (include_structs) {
			// Calculate size of pax_bmpv_t included.
			size += sizeof(pax_bmpv_t) * range_size;
		}
		
		return size;
	}
}

// Calculates the size of the region's bitmap data.
static inline size_t pax_calc_range_bitmap_size(const pax_font_range_t *range) {
	return pax_calc_range_size(range, false);
}

// Calculates the size of the region's bitmap data and structs.
static inline size_t pax_calc_range_total_size(const pax_font_range_t *range) {
	return pax_calc_range_size(range, true);
}

// Reads a number from the file (little-endian).
static bool xreadnum(uint64_t *number, size_t bytes, FILE *fd) {
	uint64_t out  = 0;
	size_t read = 0;
	for (size_t i = 0; i < bytes; i++) {
		uint8_t tmp = 0;
		read += fread(&tmp, 1, 1, fd);
		out |= tmp << (i*8);
	}
	*number = out;
	return read == bytes;
}

// Writes a number to the file (little-endian).
static bool xwritenum(uint64_t number, size_t bytes, FILE *fd) {
	size_t written = 0;
	for (size_t i = 0; i < bytes; i++) {
		uint8_t tmp = number;
		written += fwrite(&tmp, 1, 1, fd);
		number >>= 8;
	}
	return written == bytes;
}

static inline bool xfwrite(const void *restrict __ptr, size_t __size, size_t __n, FILE *restrict __s) {
	return fwrite(__ptr, __size, __n, __s) == __n;
}

static inline bool xfread(void *restrict __ptr, size_t __size, size_t __n, FILE *restrict __s) {
	return fread(__ptr, __size, __n, __s) == __n;
}

// Args: size_t *number, size_t bytes, FILE *fd
#define xreadnum_assert(...) \
	do { \
		if (!xreadnum(__VA_ARGS__)) goto fd_error; \
	} while(0)

// Args: size_t number, size_t bytes, FILE *fd
#define xwritenum_assert(...) \
	do { \
		if (!xwritenum(__VA_ARGS__)) goto fd_error; \
	} while(0)

// Args: void *ptr, size_t size, size_t n, FILE *fd
#define fread_assert(...) \
	do { \
		if (!xfread(__VA_ARGS__)) goto fd_error; \
	} while(0)

// Args: void *ptr, size_t size, size_t n, FILE *fd
#define fwrite_assert(...) \
	do { \
		if (!xfwrite(__VA_ARGS__)) goto fd_error; \
	} while(0)
#endif

// Loads a font using a file descriptor.
// Allocates the entire font in one go, such that only free(pax_font_t*) is required.
pax_font_t *pax_load_font(FILE *fd) {
	if (!fd) {
		PAX_LOGE(TAG, "File pointer is NULL");
		pax_last_error = PAX_ERR_NODATA;
		return NULL;
	}
	pax_font_t *out = NULL;
	size_t      out_addr;
	uint64_t    tmpint;
	
	/* ==== DETERMINE COMPATIBILITY ==== */
	// Validate magic.
	char magic_temp[11] = {0,0,0,0,0,0,0,0,0,0,0};
	fread_assert(magic_temp, 1, 11, fd);
	if (strcmp(magic_temp, "pax_font_t")) {
		// Invalid magic.
		PAX_LOGE(TAG, "Invalid magic in font file");
		pax_last_error = PAX_ERR_CORRUPT;
		return NULL;
	}
	
	// Validate loader version.
	uint64_t font_version;
	xreadnum_assert(&font_version, sizeof(uint16_t), fd);
	if (font_version != PAX_FONT_LOADER_VERSION) {
		// Different font loader version; unsupported.
		PAX_LOGE(TAG, "Unsupported font version %hu (supported: %hu)", (uint16_t) font_version, PAX_FONT_LOADER_VERSION);
		return NULL;
	}
	
	/* ==== READ METADATA ==== */
	// Number of stored pax_bmpv_t.
	uint64_t n_bmpv;
	xreadnum_assert(&n_bmpv,   sizeof(uint64_t), fd);
	
	// Size of the combined bitmaps.
	uint64_t n_bitmap;
	xreadnum_assert(&n_bitmap, sizeof(uint64_t), fd);
	
	// Size of the font name.
	uint64_t n_name;
	xreadnum_assert(&n_name,   sizeof(uint64_t), fd);
	
	// Number of ranges in the font.
	uint64_t n_ranges;
	xreadnum_assert(&n_ranges, sizeof(uint64_t), fd);
	
	// Calculate required size.
	size_t required_size = sizeof(pax_font_t)
						+ n_ranges * sizeof(pax_font_range_t)
						+ n_bmpv * sizeof(pax_bmpv_t)
						+ n_bitmap
						+ n_name + 1;
	
	// Validate required size.
	if (required_size < PAX_FONT_LOADER_MINUMUM_SIZE) {
		// The size is suspiciously small.
		PAX_LOGE(TAG, "File corruption: Font size reported is too small (metadata; %zu < %zu)", required_size, PAX_FONT_LOADER_MINUMUM_SIZE);
		pax_last_error = PAX_ERR_UNSUPPORTED;
		return NULL;
	}
	
	// Allocate memory.
	out = malloc(required_size);
	out_addr = (size_t) out;
	if (!out) {
		PAX_LOGE(TAG, "Out of memory for loading font (%zu required)", required_size);
		pax_last_error = PAX_ERR_NOMEM;
		return NULL;
	}
	
	out->n_ranges = n_ranges;
	
	// Default point size.
	xreadnum_assert(&tmpint, sizeof(uint16_t), fd);
	out->default_size = tmpint;
	
	// Whether antialiassing is recommended.
	xreadnum_assert(&tmpint, sizeof(bool), fd);
	out->recommend_aa = !!tmpint;
	
	// Validate again whether the memory is enough.
	size_t minimum_size = sizeof(pax_font_t) + out->n_ranges * sizeof(pax_font_range_t) + 3;
	if (required_size < minimum_size) {
		PAX_LOGE(TAG, "File corruption: Font size reported is too small (range metadata; %zu < %zu)", required_size, minimum_size);
		pax_last_error = PAX_ERR_CORRUPT;
		free(out);
		return NULL;
	}
	
	/* ==== READ RANGES ==== */
	// Calculate addresses.
	size_t output_offset = sizeof(pax_font_t) + out->n_ranges * sizeof(pax_font_range_t);
	out->ranges = (void *) (out_addr + sizeof(pax_font_t));
	
	// Read range data.
	size_t bitmap_blob_size = 0;
	for (size_t i = 0; i < out->n_ranges; i++) {
		pax_font_range_t *range = (pax_font_range_t *) &out->ranges[i];
		
		// Range type.
		xreadnum_assert(&tmpint, sizeof(uint8_t), fd);
		range->type  = tmpint;
		
		// Range start glyph.
		xreadnum_assert(&tmpint, sizeof(uint32_t), fd);
		range->start = tmpint;
		
		// Range end glyph.
		xreadnum_assert(&tmpint, sizeof(uint32_t), fd);
		range->end   = tmpint;
		
		size_t range_size = range->end - range->start + 1;
		
		if (range->type == PAX_FONT_TYPE_BITMAP_MONO) {
			// Glyph width.
			xreadnum_assert(&tmpint, sizeof(uint8_t), fd);
			range->bitmap_mono.width  = tmpint;
			
			// Glyph height.
			xreadnum_assert(&tmpint, sizeof(uint8_t), fd);
			range->bitmap_mono.height = tmpint;
			
			// Glyph bits per pixel.
			xreadnum_assert(&tmpint, sizeof(uint8_t), fd);
			range->bitmap_mono.bpp    = tmpint;
			
		} else if (range->type == PAX_FONT_TYPE_BITMAP_VAR) {
			// Read later: Additional glyph dimensions.
			// Calculate the address.
			range->bitmap_var.dims   = (void *) (out_addr + output_offset);
			
			// Glyph height.
			xreadnum_assert(&tmpint, sizeof(uint8_t), fd);
			range->bitmap_var.height = tmpint;
			
			// Glyph bits per pixel.
			xreadnum_assert(&tmpint, sizeof(uint8_t), fd);
			range->bitmap_var.bpp    = tmpint;
			
			// Reassert size requirements.
			minimum_size  += range_size * sizeof(pax_bmpv_t);
			output_offset += range_size * sizeof(pax_bmpv_t);
			if (required_size < minimum_size) {
				PAX_LOGE(TAG, "File corruption: Font size reported is too small (bitmap metadata; %zu < %zu)", required_size, minimum_size);
				pax_last_error = PAX_ERR_CORRUPT;
				free(out);
				return NULL;
			}
			
			// Additional glyph dimensions.
			for (size_t x = 0; x < range_size; x++) {
				pax_bmpv_t *bmpv = (pax_bmpv_t *) &range->bitmap_var.dims[x];
				
				// Bitmap draw X offset.
				xreadnum_assert(&tmpint, sizeof(uint8_t), fd);
				bmpv->draw_x = tmpint;
				
				// Bitmap draw Y offset.
				xreadnum_assert(&tmpint, sizeof(uint8_t), fd);
				bmpv->draw_y = tmpint;
				
				// Bitmap drawn width.
				xreadnum_assert(&tmpint, sizeof(uint8_t), fd);
				bmpv->draw_w = tmpint;
				
				// Bitmap drawn height.
				xreadnum_assert(&tmpint, sizeof(uint8_t), fd);
				bmpv->draw_h = tmpint;
				
				// Bitmap measured width.
				xreadnum_assert(&tmpint, sizeof(uint8_t), fd);
				bmpv->measured_width = tmpint;
				
				// Bitmap index.
				xreadnum_assert(&tmpint, sizeof(uint64_t), fd);
				bmpv->index = tmpint;
			}
			
		} else {
			// Invalid type.
			PAX_LOGE(TAG, "File corruption: Font type invalid (%u in range %zu)", range->type, i);
			pax_last_error = PAX_ERR_CORRUPT;
			free(out);
			return NULL;
		}
		
		// Tally up the bitmap blob size for later use.
		bitmap_blob_size += pax_calc_range_bitmap_size(range);
		
	}
	
	/* ==== RAW BITMAP DATA ==== */
	fread_assert((void*) (out_addr + output_offset), 1, required_size - output_offset, fd);
	for (size_t i = 0; i < out->n_ranges; i++) {
		pax_font_range_t *range = (pax_font_range_t *) &out->ranges[i];
		
		if (range->type == PAX_FONT_TYPE_BITMAP_MONO) {
			// Calculate range glyphs address.
			range->bitmap_mono.glyphs = (void *) (out_addr + output_offset);
			output_offset            += pax_calc_range_bitmap_size(range);
			
		} else {
			// Calculate range glyphs address.
			range->bitmap_var.glyphs  = (void *) (out_addr + output_offset);
			output_offset            += pax_calc_range_bitmap_size(range);
		}
	}
	
	
	return out;
	
	
	fd_error:
	pax_last_error = PAX_ERR_NODATA;
	PAX_LOGE(TAG, "Error reading from file");
	if (out) free(out);
	return NULL;
}

// Stores a font to a file descriptor.
// This is a memory intensive operation and might not succeed on embedded targets.
void pax_store_font(FILE *fd, const pax_font_t *font) {
	if (!fd) {
		PAX_LOGE(TAG, "File pointer is NULL");
		pax_last_error = PAX_ERR_NODATA;
		return;
	}
	
	/* ==== MAGIC BYTES ==== */
	fwrite_assert("pax_font_t", 1, 11, fd);
	
	/* ==== PLATFORM METADATA ==== */
	// Font loader version.
	// Files written for another version are assumed incompatible.
	xwritenum_assert(PAX_FONT_LOADER_VERSION, sizeof(uint16_t), fd);
	
	/* ==== DETERMINE TOTAL SIZE ==== */
	// Calculate total bitmap size.
	size_t total_bitmap = 0;
	for (size_t i = 0; i < font->n_ranges; i++) {
		total_bitmap += pax_calc_range_bitmap_size(&font->ranges[i]);
	}
	// Calculate number of pax_bmpv_t stored.
	size_t total_bmpv = 0;
	for (size_t i = 0; i < font->n_ranges; i++) {
		const pax_font_range_t *range = &font->ranges[i];
		
		if (range->type == PAX_FONT_TYPE_BITMAP_VAR) {
			total_bmpv += range->end - range->start + 1;
		}
	}
	
	/* ==== FONT METADATA ==== */
	// Total number of pax_bmpv_t.
	xwritenum_assert(total_bmpv,         sizeof(uint64_t), fd);
	// Total size of the bitmap data.
	xwritenum_assert(total_bitmap,       sizeof(uint64_t), fd);
	// Length excluding null terminator of the name.
	xwritenum_assert(strlen(font->name), sizeof(uint64_t), fd);
	// Number of ranges in the font.
	xwritenum_assert(font->n_ranges,     sizeof(uint64_t), fd);
	// Default size of the font in pixels.
	xwritenum_assert(font->default_size, sizeof(uint16_t), fd);
	// Whether usage of antialiasing is recommended.
	xwritenum_assert(font->recommend_aa, 1,                fd);
	
	/* ==== RANGE DATA ==== */
	size_t raw_data_offs = 0;
	for (size_t i = 0; i < font->n_ranges; i++) {
		const pax_font_range_t *range = &font->ranges[i];
		size_t range_size = range->end - range->start + 1;
		
		// Range type.
		xwritenum_assert(range->type,  sizeof(uint8_t),  fd);
		// Range start.
		xwritenum_assert(range->start, sizeof(uint32_t), fd);
		// Range end.
		xwritenum_assert(range->end,   sizeof(uint32_t), fd);
		
		if (range->type == PAX_FONT_TYPE_BITMAP_MONO) {
			// Range width.
			xwritenum_assert(range->bitmap_mono.width,  sizeof(uint8_t),  fd);
			// Range height.
			xwritenum_assert(range->bitmap_mono.height, sizeof(uint8_t),  fd);
			// Range bit per pixel.
			xwritenum_assert(range->bitmap_mono.bpp,    sizeof(uint8_t),  fd);
		} else {
			// Range height.
			xwritenum_assert(range->bitmap_var.height,  sizeof(uint8_t),  fd);
			// Range bit per pixel.
			xwritenum_assert(range->bitmap_var.bpp,     sizeof(uint8_t),  fd);
			
			// Range bitmap dimensions.
			for (size_t x = 0; x < range_size; x++) {
				pax_bmpv_t bmpv = range->bitmap_var.dims[x];
				
				// Bitmap draw X offset.
				xwritenum_assert(bmpv.draw_x,         sizeof(uint8_t),  fd);
				// Bitmap draw Y offset.
				xwritenum_assert(bmpv.draw_y,         sizeof(uint8_t),  fd);
				// Bitmap drawn width.
				xwritenum_assert(bmpv.draw_w,         sizeof(uint8_t),  fd);
				// Bitmap drawn height.
				xwritenum_assert(bmpv.draw_h,         sizeof(uint8_t),  fd);
				// Bitmap measured width.
				xwritenum_assert(bmpv.measured_width, sizeof(uint8_t),  fd);
				// Bitmap data index.
				xwritenum_assert(bmpv.index,          sizeof(uint64_t), fd);
			}
		}
		
		raw_data_offs += pax_calc_range_bitmap_size(range);
	}
	
	/* ==== RAW DATA ==== */
	// Write bitmap data.
	for (size_t i = 0; i < font->n_ranges; i++) {
		const pax_font_range_t *range = &font->ranges[i];
		const void *data;
		// Determine size of raw data.
		size_t length = pax_calc_range_bitmap_size(range);
		
		// Grab raw data.
		if (range->type == PAX_FONT_TYPE_BITMAP_MONO) {
			data = range->bitmap_mono.glyphs;
		} else {
			data = range->bitmap_var.glyphs;
		}
		
		// Write to the data clump.
		fwrite_assert(data, 1, length, fd);
	}
	
	// Write font name.
	fwrite_assert(font->name, 1, strlen(font->name) + 1, fd);
	
	return;
	
	
	fd_error:
	pax_last_error = PAX_ERR_UNKNOWN;
	PAX_LOGE(TAG, "Error writing to file");
}

