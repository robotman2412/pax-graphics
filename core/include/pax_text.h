
// SPDX-License-Identifier: MIT

#ifndef PAX_TEXT_H
#define PAX_TEXT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "pax_gfx.h"

#include <string.h>

/* ====== UTF-8 UTILITIES ====== */

// Extracts an UTF-8 code from a string.
// Returns how many bytes were read.
// Sets the decoded UTF-8 using a pointer.
// If the string terminates early or contains invalid unicode, U+FFFD is returned.
size_t pax_utf8_getch_l(char const *cstr, size_t len, uint32_t *out);
// Returns how many UTF-8 characters a given string contains.
size_t pax_utf8_strlen_l(char const *cstr, size_t len);
// Seek to the next UTF-8 character in a string.
size_t pax_utf8_seeknext_l(char const *cstr, size_t len, size_t cursor);
// Seek to the previous UTF-8 character in a string.
size_t pax_utf8_seekprev_l(char const *cstr, size_t len, size_t cursor);

// Extracts an UTF-8 code from a null-terminated c-string.
// Returns the new string pointer.
// Sets the decoded UTF-8 using a pointer.
// If the string terminates early or contains invalid unicode, U+FFFD is returned.
static inline char *pax_utf8_getch(char const *cstr, uint32_t *out) {
    return (char *)cstr + pax_utf8_getch_l(cstr, cstr ? strlen(cstr) : 0, out);
}
// Returns how many UTF-8 characters a given c-string contains.
static inline size_t pax_utf8_strlen(char const *cstr) {
    return pax_utf8_strlen_l(cstr, cstr ? strlen(cstr) : 0);
}
// Seek to the next UTF-8 character in a string.
static inline size_t pax_utf8_seeknext(char const *cstr, size_t cursor) {
    return pax_utf8_seeknext_l(cstr, cstr ? strlen(cstr) : 0, cursor);
}
// Seek to the previous UTF-8 character in a string.
static inline size_t pax_utf8_seekprev(char const *cstr, size_t cursor) {
    return pax_utf8_seekprev_l(cstr, cstr ? strlen(cstr) : 0, cursor);
}

/* ======= DRAWING: TEXT ======= */

// Loads a font using a file descriptor.
// Allocates the entire font in one go, such that only free(pax_font_t*) is required.
pax_font_t *pax_load_font(FILE *fd);
// Stores a font to a file descriptor.
void        pax_store_font(FILE *fd, pax_font_t const *font);

// Draw a string with given font, size, alignment and optional cursor index.
// Returns the text size and relative cursor position in a pax_2vec2f.
pax_2vec2f pax_draw_text_adv(
    pax_buf_t        *buf,
    pax_col_t         color,
    pax_font_t const *font,
    float             font_size,
    float             x,
    float             y,
    char const       *text,
    size_t            len,
    pax_align_t       halign,
    pax_align_t       valign,
    ptrdiff_t         cursorpos
);

// Measure the size of a string with given font, size, alignment and optional cursor index.
// Returns the text size and relative cursor position in a pax_2vec2f.
pax_2vec2f pax_text_size_adv(
    pax_font_t const *font,
    float             font_size,
    char const       *text,
    size_t            len,
    pax_align_t       halign,
    pax_align_t       valign,
    ptrdiff_t         cursorpos
);

// Draw a string with the given font and return it's size.
// Size is before matrix transformation.
static inline pax_vec2f pax_draw_text(
    pax_buf_t *buf, pax_col_t color, pax_font_t const *font, float font_size, float x, float y, char const *text
) {
    pax_2vec2f dims = pax_draw_text_adv(
        buf,
        color,
        font,
        font_size,
        x,
        y,
        text,
        text ? strlen(text) : 0,
        PAX_ALIGN_BEGIN,
        PAX_ALIGN_BEGIN,
        -1
    );
    return (pax_vec2f){dims.x0, dims.y0};
}
// Draw a string with the given font and return it's size.
// Text is center-aligned on every line.
// Size is before matrix transformation.
static inline pax_vec2f pax_center_text(
    pax_buf_t *buf, pax_col_t color, pax_font_t const *font, float font_size, float x, float y, char const *text
) {
    pax_2vec2f dims = pax_draw_text_adv(
        buf,
        color,
        font,
        font_size,
        x,
        y,
        text,
        text ? strlen(text) : 0,
        PAX_ALIGN_CENTER,
        PAX_ALIGN_BEGIN,
        -1
    );
    return (pax_vec2f){dims.x0, dims.y0};
}
// Draw a string with the given font and return it's size.
// Text is right-aligned on every line.
// Size is before matrix transformation.
static inline pax_vec2f pax_right_text(
    pax_buf_t *buf, pax_col_t color, pax_font_t const *font, float font_size, float x, float y, char const *text
) {
    pax_2vec2f dims = pax_draw_text_adv(
        buf,
        color,
        font,
        font_size,
        x,
        y,
        text,
        text ? strlen(text) : 0,
        PAX_ALIGN_END,
        PAX_ALIGN_BEGIN,
        -1
    );
    return (pax_vec2f){dims.x0, dims.y0};
}
// Calculate the size of the string with the given font.
// Size is before matrix transformation.
static inline pax_vec2f pax_text_size(pax_font_t const *font, float font_size, char const *text) {
    pax_2vec2f dims
        = pax_text_size_adv(font, font_size, text, text ? strlen(text) : 0, PAX_ALIGN_BEGIN, PAX_ALIGN_BEGIN, -1);
    return (pax_vec2f){dims.x0, dims.y0};
}

#ifdef __cplusplus
} // extern "C"
#endif

#endif // PAX_TEXT_H
