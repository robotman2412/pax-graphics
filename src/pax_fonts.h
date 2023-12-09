
// SPDX-License-Identifier: MIT

#ifndef PAX_FONTS_H
#define PAX_FONTS_H

#include "pax_types.h"

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

/* ============ INDEX ============ */

extern pax_font_t const PRIVATE_pax_font_sky;
extern pax_font_t const PRIVATE_pax_font_sky_mono;
extern pax_font_t const PRIVATE_pax_font_marker;
extern pax_font_t const PRIVATE_pax_font_saira_condensed;
extern pax_font_t const PRIVATE_pax_font_saira_regular;

#define pax_font_sky             (&PRIVATE_pax_font_sky)
#define pax_font_sky_mono        (&PRIVATE_pax_font_sky_mono)
#define pax_font_marker          (&PRIVATE_pax_font_marker)
#define pax_font_saira_condensed (&PRIVATE_pax_font_saira_condensed)
#define pax_font_saira_regular   (&PRIVATE_pax_font_saira_regular)

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
