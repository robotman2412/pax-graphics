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

typedef enum {
	PAX_FONT_BITMAP_MONO,
	PAX_FONT_BITMAP_VAR,
} pax_font_type_t;

struct pax_bmpv;
struct pax_font;
struct pax_font_range;

typedef struct pax_bmpv       pax_bmpv_t;
typedef struct pax_font       pax_font_t;
typedef struct pax_font_range pax_font_range_t;

struct pax_bmpv {
	// The position of the drawn portion.
	uint8_t draw_x, draw_y;
	// The size of the drawn portion.
	uint8_t draw_w, draw_h;
	// The measured width of the glyph.
	uint8_t measured_width;
	// The index in the glyphs data for this glyph.
	size_t index;
};

struct pax_font {
	// The searchable name of the font.
	char             *name;
	// The number of ranges included in the font.
	size_t            n_ranges;
	// The ranges included in the font.
	pax_font_range_t *ranges;
	// Default point size.
	uint16_t          default_size;
};

struct pax_font_range {
	// The type of font range.
	pax_font_type_t  type;
	// First character in range.
	wchar_t          start;
	// Last character in range.
	wchar_t          end;
	union {
		// Monospace, bitmapped fonts.
		struct {
			uint8_t *glyphs;
			uint8_t  width;
			uint8_t  height;
			// uint8_t  bpp;
		} bitmap_mono;
		// Variable pitch, bitmapped fonts.
		struct {
			uint8_t    *glyphs;
			pax_bmpv_t *dims;
			uint8_t     height;
			// uint8_t     bpp;
		} bitmap_var;
	};
};

/* ============ INDEX ============ */

#define PAX_N_FONTS pax_n_fonts
#define PAX_FONT_DEFAULT (&pax_fonts_index[0])
extern const pax_font_t pax_fonts_index[];
extern const size_t     pax_n_fonts;

/* ========== FUNCTIONS ========== */

// Finds the built-in font with the given name.
pax_font_t *pax_get_font(char *name);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //PAX_FONTS_H
