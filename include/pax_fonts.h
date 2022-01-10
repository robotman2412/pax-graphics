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

#ifndef PAX_FONTS_H
#define PAX_FONTS_H

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

#include "pax_types.h"

/* ============ TYPES ============ */

struct pax_font;

typedef struct pax_font pax_font_t;

struct pax_font {
	uint8_t  type;
	char    *name;
	uint8_t *glyphs_uni;
	uint8_t  glyphs_uni_w;
	uint8_t  glyphs_uni_h;
};

#define PAX_BITMAP_UNI 0

// Bitmap font definition: uniform width characters.
// Bits are to be packed into bytes per row, and split rows vertically.
// Only ascii and exactly ascii-sized fonts are supported.
#define PAX_FONT_BITMAP_UNI(strname, arrname, w, h) (pax_font_t) {\
	.name = strname,\
	.type = PAX_BITMAP_UNI,\
	.glyphs_uni = arrname,\
	.glyphs_uni_w = w,\
	.glyphs_uni_h = h\
}

/* ============ INDEX ============ */

extern uint8_t font_bitmap_raw_7x9[];

#define PAX_N_FONTS 1
#define PAX_FONT_DEFAULT (&pax_fonts_index[0])
extern pax_font_t pax_fonts_index[PAX_N_FONTS];

/* ========== FUNCTIONS ========== */

// Finds the built-in font with the given name.
pax_font_t *pax_get_font(char *name);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //PAX_FONTS_H
