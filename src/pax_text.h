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

#ifndef PAX_TEXT_H
#define PAX_TEXT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "pax_gfx.h"
#include <stdio.h>

/* ====== UTF-8 UTILITIES ====== */

// Extracts an UTF-8 code from a null-terminated c-string.
// Returns the new string pointer.
// Sets the decoded UTF-8 using a pointer.
// If the string terminates early or contains invalid unicode, U+FFFD is returned.
char *pax_utf8_getch(const char *cstr, uint32_t *out);
// Returns how many UTF-8 characters a given c-string contains.
size_t pax_utf8_strlen(const char *cstr);

/* ======= DRAWING: TEXT ======= */

// Loads a font using a file descriptor.
// Allocates the entire font in one go, such that only free(pax_font_t*) is required.
pax_font_t *pax_load_font     (FILE *fd);
// Stores a font to a file descriptor.
// This is a memory intensive operation and might not succeed on embedded targets.
void        pax_store_font    (FILE *fd, const pax_font_t *font);

// Draw a string with the given font and return it's size.
// Text is center-aligned on every line.
// Size is before matrix transformation.
pax_vec2f  pax_center_text   (pax_buf_t *buf, pax_col_t color, const pax_font_t *font, float font_size, float x, float y, const char *text);
// Draw a string with the given font and return it's size.
// Size is before matrix transformation.
pax_vec2f  pax_draw_text     (pax_buf_t *buf, pax_col_t color, const pax_font_t *font, float font_size, float x, float y, const char *text);
// DEPRECATION NOTICE: This function is subject to be removed
// Draw a string with the given font and return it's size.
// Size is before matrix transformation.
// Font is scaled up without interpolation, overriding it's default.
__attribute__((deprecated))
pax_vec2f  pax_draw_text_noaa(pax_buf_t *buf, pax_col_t color, const pax_font_t *font, float font_size, float x, float y, const char *text);
// DEPRECATION NOTICE: This function is subject to be removed
// Draw a string with the given font and return it's size.
// Size is before matrix transformation.
// Font is scaled up with interpolation, overriding it's default.
__attribute__((deprecated))
pax_vec2f  pax_draw_text_aa  (pax_buf_t *buf, pax_col_t color, const pax_font_t *font, float font_size, float x, float y, const char *text);
// Calculate the size of the string with the given font.
// Size is before matrix transformation.
pax_vec2f  pax_text_size     (const pax_font_t *font, float font_size, const char *text);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // PAX_TEXT_H
