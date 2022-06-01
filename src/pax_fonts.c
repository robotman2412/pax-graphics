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

#include "pax_fonts.h"
#include <strings.h>

extern const pax_font_range_t pax_font_sky_ranges[];
extern const pax_font_range_t marker_ranges[];
extern const pax_font_range_t sairaregular_ranges[];
extern const pax_font_range_t sairacondensed_ranges[];

// Font ROMs.
extern const uint8_t font_bitmap_raw_7x9[];

// ¯\_(ツ)_/¯
//   0 1 2 3 4 5 6
// 0 . . . . . . .
// 1 . x . x . . .
// 2 . x . x . . .
// 3 . . . . . . x
// 4 . . . . . . x
// 5 . . . . . x .
// 6 . . . . x . .
// 7 . x x x . . .
// 8 . . . . . . .


const uint8_t funny_thingy[] = {
	0x00,
	0x0a,
	0x0a,
	0x40,
	0x40,
	0x20,
	0x10,
	0x0e,
};

const uint8_t unfunny_thingy[] = {
	0x00,
	0x3e,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
};

const pax_font_range_t font_7x9_ranges[] = {
	{ // Ascii range.
		.type  = PAX_FONT_TYPE_BITMAP_MONO,
		.start = 0x00000,
		.end   = 0x00080,
		.bitmap_mono = {
			.glyphs = font_bitmap_raw_7x9,
			.width  = 7,
			.height = 9,
			.bpp    = 1,
		},
	}, { // Test range.
		.type  = PAX_FONT_TYPE_BITMAP_MONO,
		.start = 0x030c4,
		.end   = 0x030c4,
		.bitmap_mono = {
			.glyphs = funny_thingy,
			.width  = 7,
			.height = 9,
			.bpp    = 1,
		},
	}, { // Macron range.
		.type  = PAX_FONT_TYPE_BITMAP_MONO,
		.start = 0x000af,
		.end   = 0x000af,
		.bitmap_mono = {
			.glyphs = unfunny_thingy,
			.width  = 7,
			.height = 9,
			.bpp    = 1,
		},
	}
};

const pax_font_t pax_fonts_index[] = {
	{ // Sky
		.name         = "Sky",
		.n_ranges     = 6,
		.ranges       = pax_font_sky_ranges,
		.default_size = 9,
		.recommend_aa = false,
	}, { // Sky mono
		.name         = "7x9",
		.n_ranges     = 3,
		.ranges       = font_7x9_ranges,
		.default_size = 9,
		.recommend_aa = false,
	}, { // Sky mono
		.name         = "sky mono",
		.n_ranges     = 3,
		.ranges       = font_7x9_ranges,
		.default_size = 9,
		.recommend_aa = false,
	}, { // PermanentMarker
		.name         = "permanentmarker",
		.n_ranges     = 23,
		.ranges       = marker_ranges,
		.default_size = 22,
		.recommend_aa = true,
	}, { // Saira condensed
		.name         = "saira condensed",
		.n_ranges     = 80,
		.ranges       = sairacondensed_ranges,
		.default_size = 45,
		.recommend_aa = true,
	}, { // Saira regular
		.name         = "saira regular",
		.n_ranges     = 23,
		.ranges       = sairaregular_ranges,
		.default_size = 18,
		.recommend_aa = true,
	}
};
const size_t pax_n_fonts = sizeof(pax_fonts_index) / sizeof(pax_font_t);

// Finds the built-in font with the given name.
const pax_font_t *pax_get_font(char *name) {
	for (size_t i = 0; i < PAX_N_FONTS; i++) {
		if (!strcasecmp(pax_fonts_index[i].name, name)) {
			return &pax_fonts_index[i];
		}
	}
	return PAX_FONT_DEFAULT;
}

