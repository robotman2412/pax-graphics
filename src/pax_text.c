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



/* ====== UTF-8 UTILITIES ====== */

// Extracts an UTF-8 code from a null-terminated c-string.
// Returns the new string pointer.
// Sets the decoded UTF-8 using a pointer.
// If the string terminates early or contains invalid unicode, U+FFFD is returned.
char *utf8_getch(char *cstr, wchar_t *out) {
	char len, mask;
	if (!*cstr) {
		*out = 0xfffd; // Something something invalid UTF8.
		return cstr;
	} else if (*cstr <= 0x7f) {
		*out = *cstr;
		return cstr + 1;
	} else if ((*cstr & 0xe0) == 0xc0) {
		len = 2;
		mask = 0x1f;
	} else if ((*cstr & 0xf0) == 0xe0) {
		len = 3;
		mask = 0x0f;
	} else if ((*cstr & 0xf8) == 0xf0) {
		len = 4;
		mask = 0x07;
	} else {
		*out = 0xfffd; // Something something invalid UTF8.
		return cstr;
	}
	
	*out = 0;
	for (int i = 0; i < len; i++) {
		*out <<= 6;
		*out |= *cstr & mask;
		mask = 0x3f;
		cstr ++;
		if (!*cstr) {
			*out = 0xfffd; // Something something invalid UTF8.
			return cstr;
		}
	}
	return cstr;
}

// Returns how many UTF-8 characters a given c-string contains.
size_t utf8_strlen(char *cstr) {
	char   *end   = cstr + strlen(cstr);
	wchar_t dummy = 0;
	size_t  len   = 0;
	while (cstr != end) {
		len ++;
		cstr = utf8_getch(cstr, &dummy);
	}
	return 0;
}

// Determine whether a character is visible according to text rendering.
// Space is not considered visible.
static inline bool pax_is_visible_char(wchar_t c) {
	return c > 0x1f && c != 0x7f;
}



/* ======= DRAWING: TEXT ======= */

// Draw a string with the given font.
// If font is NULL, the default font (7x9) will be used.
void pax_draw_text(pax_buf_t *buf, pax_col_t color, pax_font_t *font, float font_size, float _x, float _y, char *text) {
	PAX_BUF_CHECK("pax_draw_text");
	
	// Don't bother if it would turn out invisible.
	if (!text || !*text) return;
	if (color < 0x01000000) return;
	// Apply default font.
	if (!font) font = PAX_FONT_DEFAULT;
	
	// Calculate sizes.
	if (font_size == 0) font_size = font->bitmap_mono.height;
	float size_mul = font_size / font->bitmap_mono.height;
	float w = size_mul * font->bitmap_mono.width;
	float h = size_mul * font->bitmap_mono.height;
	
	size_t len = strlen(text);
	
	float x = _x, y = _y;
	
	// Set up shader.
	pax_shader_font_bitmap_mono_args_t args = {
		.font              = font
	};
	pax_shader_t shader = {
		.callback          = pax_shader_font_bitmap_mono,
		.callback_args     = &args,
		.alpha_promise_0   = true,
		.alpha_promise_255 = false
	};
	// And UVs.
	pax_quad_t uvs = {
		.x0 = 0,                       .y0 = 0,
		.x1 = font->bitmap_mono.width, .y1 = 0,
		.x2 = font->bitmap_mono.width, .y2 = font->bitmap_mono.height,
		.x3 = 0,                       .y3 = font->bitmap_mono.height,
	};
	
	for (size_t i = 0; i < len; i ++) {
		char c = text[i], next = text[i + 1];
		if (c == '\r' || c == '\n') {
			// Do newline characters.
			x = _x;
			y += h;
			if (c == '\r' && next == '\n') i++;
		} else {
			// Join the other thread because we're about to change the shader.
			pax_join();
			// Update the glyph.
			c = pax_is_visible_char(c) ? c : 1;
			args.glyph = c;
			// Precalculate the index so it isn't calculated on every pixel.
			args.glyph_y_mul = (font->bitmap_mono.width + 7) / 8;
			size_t glyph_len = args.glyph_y_mul * font->bitmap_mono.height;
			args.glyph_index = glyph_len * c;
			// And draw it.
			pax_shade_rect(buf, color, &shader, &uvs, x, y, w, h);
			
			x += w;
		}
	}
	pax_join();
	
	PAX_SUCCESS();
}

// Calculate the size of the string with the given font.
// Size is before matrix transformation.
// If font is NULL, the default font (7x9) will be used.
pax_vec1_t pax_text_size(pax_font_t *font, float font_size, char *text) {
	if (!font) font = PAX_FONT_DEFAULT;
	
	if (font_size == 0) font_size = font->default_size;
	float size_mul = font_size / font->default_size;
	float w = size_mul * font->bitmap_mono.width;
	float h = size_mul * font->bitmap_mono.height;
	
	float text_w = 0;
	float text_h = h;
	
	size_t len = strlen(text);
	
	float x = 0, y = 0;
	
	for (size_t i = 0; i < len; i ++) {
		char c = text[i], next = text[i + 1];
		if (c == '\r' || c == '\n') {
			x = 0;
			y += h;
			text_h = y + h;
			if (c == '\r' && next == '\n') i++;
		} else {
			x += w;
			if (x > text_w) text_w = x;
		}
	}
	
	return (pax_vec1_t) { .x = text_w, .y = text_h };
}
