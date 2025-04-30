
// SPDX-License-Identifier: MIT

#include "pax_fonts.h"

#include "pax_internal.h"

#include <string.h>

extern pax_font_range_t const pax_font_sky_ranges[];
extern pax_font_range_t const permanentmarker_ranges[];
extern pax_font_range_t const sairaregular_ranges[];
extern pax_font_range_t const sairacondensed_ranges[];

// Font ROMs.
extern uint8_t const font_bitmap_raw_7x9[];

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


static uint8_t const funny_thingy[] = {
    0x00,
    0x0a,
    0x0a,
    0x40,
    0x40,
    0x20,
    0x10,
    0x0e,
};

static uint8_t const unfunny_thingy[] = {
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

static pax_font_range_t const font_7x9_ranges[] = {
    {
        // Ascii range.
        .type  = PAX_FONT_TYPE_BITMAP_MONO,
        .start = 0x00000,
        .end   = 0x00080,
        .bitmap_mono =
            {
                .glyphs = font_bitmap_raw_7x9,
                .width  = 7,
                .height = 9,
                .bpp    = 1,
            },
    },
    {
        // Test range.
        .type  = PAX_FONT_TYPE_BITMAP_MONO,
        .start = 0x030c4,
        .end   = 0x030c4,
        .bitmap_mono =
            {
                .glyphs = funny_thingy,
                .width  = 7,
                .height = 9,
                .bpp    = 1,
            },
    },
    {
        // Macron range.
        .type  = PAX_FONT_TYPE_BITMAP_MONO,
        .start = 0x000af,
        .end   = 0x000af,
        .bitmap_mono =
            {
                .glyphs = unfunny_thingy,
                .width  = 7,
                .height = 9,
                .bpp    = 1,
            },
    }
};

pax_font_t const *pax_fonts_index[] = {
#if CONFIG_PAX_COMPILE_FONT_INDEX
    &pax_font_sky_raw,
    &pax_font_sky_mono_raw,
    &pax_font_marker_raw,
    &pax_font_saira_condensed_raw,
    &pax_font_saira_regular_raw,
#endif
};
size_t const pax_n_fonts = sizeof(pax_fonts_index) / sizeof(pax_font_t const *);

pax_font_t const pax_font_sky_raw = {
    // Sky
    .name         = "Sky",
    .n_ranges     = 6,
    .ranges       = pax_font_sky_ranges,
    .default_size = 9,
    .recommend_aa = false,
};
pax_font_t const pax_font_sky_mono_raw = {
    // Sky mono
    .name         = "Sky Mono",
    .n_ranges     = 3,
    .ranges       = font_7x9_ranges,
    .default_size = 9,
    .recommend_aa = false,
};
pax_font_t const pax_font_marker_raw = {
    // PermanentMarker
    .name         = "Permanent Marker",
    .n_ranges     = 3,
    .ranges       = permanentmarker_ranges,
    .default_size = 22,
    .recommend_aa = true,
};
pax_font_t const pax_font_saira_condensed_raw = {
    // Saira condensed
    .name         = "Saira Condensed",
    .n_ranges     = 3,
    .ranges       = sairacondensed_ranges,
    .default_size = 45,
    .recommend_aa = true,
};
pax_font_t const pax_font_saira_regular_raw = {
    // Saira regular
    .name         = "Saira Regular",
    .n_ranges     = 27,
    .ranges       = sairaregular_ranges,
    .default_size = 18,
    .recommend_aa = true,
};

#if CONFIG_PAX_COMPILE_FONT_INDEX
static char lower(char in) {
    if (in >= 'A' && in <= 'Z') {
        return in + 'a' - 'A';
    } else {
        return in;
    }
}

static bool casecmp(char const *restrict a, char const *restrict b) {
    while (*a || *b) {
        if (lower(*a) != lower(*b))
            return false;
        a++;
        b++;
    }
    return true;
}

// Finds the built-in font with the given name.
pax_font_t const *pax_get_font(char const *name) {
    for (size_t i = 0; i < PAX_N_FONTS; i++) {
        if (!casecmp(pax_fonts_index[i]->name, name)) {
            return pax_fonts_index[i];
        }
    }
    return NULL;
}
#else
pax_font_t const *pax_get_font(char const *name) {
    (void)name;
    // Not compiled in, so ignore this.
    PAX_ERROR(PAX_ERR_UNSUPPORTED, NULL);
}
#endif
