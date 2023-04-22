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

#ifndef PAX_FONTS_H
#define PAX_FONTS_H

#include "pax_types.h"

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

/* ============ INDEX ============ */

extern const pax_font_t PRIVATE_pax_font_sky;
extern const pax_font_t PRIVATE_pax_font_sky_mono;
extern const pax_font_t PRIVATE_pax_font_marker;
extern const pax_font_t PRIVATE_pax_font_saira_condensed;
extern const pax_font_t PRIVATE_pax_font_saira_regular;

#define pax_font_sky             (&PRIVATE_pax_font_sky)
#define pax_font_sky_mono        (&PRIVATE_pax_font_sky_mono)
#define pax_font_marker          (&PRIVATE_pax_font_marker)
#define pax_font_saira_condensed (&PRIVATE_pax_font_saira_condensed)
#define pax_font_saira_regular   (&PRIVATE_pax_font_saira_regular)

// The number of built-in fonts.
#define PAX_N_FONTS pax_n_fonts
// The default font ("sky", variable pitch).
#define PAX_FONT_DEFAULT pax_font_sky
// A comprehensive index of built-in fonts.
extern const pax_font_t *pax_fonts_index[];
extern const size_t      pax_n_fonts;

/* ========== FUNCTIONS ========== */

// Finds the built-in font with the given name.
const pax_font_t *pax_get_font(const char *name);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //PAX_FONTS_H
