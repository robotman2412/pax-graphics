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

// Distinguishes between ways to draw fonts.
typedef enum {
	// For monospace bitmapped fonts.
	PAX_FONT_TYPE_BITMAP_MONO,
	// For variable pitch bitmapped fonts.
	PAX_FONT_TYPE_BITMAP_VAR,
} pax_font_type_t;

struct pax_bmpv;
struct pax_font;
struct pax_font_range;

typedef struct pax_bmpv       pax_bmpv_t;
typedef struct pax_font       pax_font_t;
typedef struct pax_font_range pax_font_range_t;

// Information relevant to each character of a variable pitch font.
struct pax_bmpv {
	// The position of the drawn portion.
	int8_t draw_x, draw_y;
	// The size of the drawn portion.
	uint8_t draw_w, draw_h;
	// The measured width of the glyph.
	uint8_t measured_width;
	// The index in the glyphs data for this glyph.
	size_t index;
};

// Information relevant for the entirety of each font.
struct pax_font {
	// The searchable name of the font.
	const char             *name;
	// The number of ranges included in the font.
	const size_t            n_ranges;
	// The ranges included in the font.
	const pax_font_range_t *ranges;
	// Default point size.
	uint16_t                default_size;
	// Whether or not it is recommended to use antialiasing.
	// Applies to pax_draw_text, but not it's variants.
	bool                    recommend_aa;
};

// Describes a range of glyphs in a font.
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
			// The raw glyph bytes.
			const uint8_t *glyphs;
			// The width of all glyphs.
			const uint8_t  width;
			// The height of all glyphs.
			const uint8_t  height;
			// The Bits Per Pixel of all glyphs.
			const uint8_t  bpp;
		} bitmap_mono;
		// Variable pitch, bitmapped fonts.
		struct {
			// The raw glyph bytes.
			const uint8_t    *glyphs;
			// Additional dimensions defined per glyph.
			const pax_bmpv_t *dims;
			// The height of all glyphs.
			const uint8_t     height;
			// The Bits Per Pixel of all glyphs.
			const uint8_t     bpp;
		} bitmap_var;
	};
};

/* ============ INDEX ============ */

// The number of built-in fonts.
#define PAX_N_FONTS pax_n_fonts
// The default font ("sky", variable pitch).
#define PAX_FONT_DEFAULT (&pax_fonts_index[0])
// A comprehensive index of built-in fonts.
extern const pax_font_t pax_fonts_index[];
extern const size_t     pax_n_fonts;

/* ========== FUNCTIONS ========== */

// Finds the built-in font with the given name.
const pax_font_t *pax_get_font(char *name);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //PAX_FONTS_H
