
// SPDX-License-Identifier: MIT

#ifndef PAX_GUI_TEXT_H
#define PAX_GUI_TEXT_H

#include "pax_gui_types.h"

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

// A paragraph of text with adjustable alignment.
typedef struct pgui_text pgui_text_t;
// A vertically centered, shrink-to-fit label with adjustable alignment.
typedef struct pgui_text pgui_label_t;

struct pgui_text {
    // Common GUI element data.
    pgui_base_t      base;
    // Text to show.
    char const      *text;
    // Text alignment.
    pax_text_align_t align;
};

// Draw a text paragraph.
void pgui_draw_text(pax_buf_t *gfx, pax_vec2f pos, pgui_text_t *elem, pgui_theme_t const *theme, uint32_t flags);

// Draw a text label.
void pgui_draw_label(pax_buf_t *gfx, pax_vec2f pos, pgui_label_t *elem, pgui_theme_t const *theme, uint32_t flags);

#ifdef __cplusplus
} // extern "C"
#endif //__cplusplus

#endif // PAX_GUI_TEXT_H
