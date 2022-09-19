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

#ifndef PAX_TEXT_H
#define PAX_TEXT_H

#include "pax_gfx.h"
#include <stdio.h>

/* ======= DRAWING: TEXT ======= */

// Loads a font using a file descriptor.
// Allocates the entire font in one go, such that only free(pax_font_t*) is required.
pax_font_t *pax_load_font           (FILE *fd);
// Stores a font to a file descriptor.
// This is a memory intensive operation and might not succeed on embedded targets.
void        pax_store_font          (FILE *fd, const pax_font_t *font);

// Draw a string with the given font and return it's size.
// Text is center-aligned on every line.
// Size is before matrix transformation.
// Font is scaled up with method recommended by it (see pax_font_t::recommend_aa).
pax_vec1_t  pax_center_text         (pax_buf_t *buf, pax_col_t color, const pax_font_t *font, float font_size, float x, float y, const char *text);
// Draw a string with the given font and return it's size.
// Size is before matrix transformation.
// Font is scaled up with method recommended by it (see pax_font_t::recommend_aa).
pax_vec1_t  pax_draw_text           (pax_buf_t *buf, pax_col_t color, const pax_font_t *font, float font_size, float x, float y, const char *text);
// Draw a string with the given font and return it's size.
// Size is before matrix transformation.
// Font is scaled up without interpolation.
pax_vec1_t  pax_draw_text_noaa      (pax_buf_t *buf, pax_col_t color, const pax_font_t *font, float font_size, float x, float y, const char *text);
// Draw a string with the given font and return it's size.
// Size is before matrix transformation.
// Font is scaled up with interpolation.
pax_vec1_t  pax_draw_text_aa        (pax_buf_t *buf, pax_col_t color, const pax_font_t *font, float font_size, float x, float y, const char *text);
// Calculate the size of the string with the given font.
// Size is before matrix transformation.
pax_vec1_t  pax_text_size           (const pax_font_t *font, float font_size, const char *text);

#endif //PAX_TEXT_H
