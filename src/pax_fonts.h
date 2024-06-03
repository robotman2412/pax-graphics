
// SPDX-License-Identifier: MIT

#ifndef PAX_FONTS_H
#define PAX_FONTS_H

#include "pax_types.h"

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

/* ============ INDEX ============ */

extern pax_font_t const pax_font_sky_raw;
extern pax_font_t const pax_font_sky_mono_raw;
extern pax_font_t const pax_font_marker_raw;
extern pax_font_t const pax_font_saira_condensed_raw;
extern pax_font_t const pax_font_saira_regular_raw;

#define pax_font_sky             (&pax_font_sky_raw)
#define pax_font_sky_mono        (&pax_font_sky_mono_raw)
#define pax_font_marker          (&pax_font_marker_raw)
#define pax_font_saira_condensed (&pax_font_saira_condensed_raw)
#define pax_font_saira_regular   (&pax_font_saira_regular_raw)

// The number of built-in fonts.
#define PAX_N_FONTS      pax_n_fonts
// The default font ("sky", variable pitch).
#define PAX_FONT_DEFAULT pax_font_sky
// A comprehensive index of built-in fonts.
extern pax_font_t const *pax_fonts_index[];
extern size_t const      pax_n_fonts;

/* ========== FUNCTIONS ========== */

// Finds the built-in font with the given name.
pax_font_t const *pax_get_font(char const *name);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif // PAX_FONTS_H
