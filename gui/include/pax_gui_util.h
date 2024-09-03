
// SPDX-License-Identifier: MIT

#ifndef PAX_GUI_UTIL_H
#define PAX_GUI_UTIL_H

#include "pax_gui.h"

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

// Logic for CTRL+RIGHT in editable text.
size_t pgui_text_ctrl_right(char const *cstr, size_t cstr_len, size_t cursor, bool merge_space);
// Logic for CTRL+LEFT in editable text.
size_t pgui_text_ctrl_left(char const *cstr, size_t cstr_len, size_t cursor, bool merge_space);



// Visuals for (editable) text-based elements.
void pgui_drawutil_textbox(
    pax_buf_t *gfx, pax_vec2i pos, pgui_elem_t *elem, pgui_theme_t const *theme, uint32_t flags, bool editable
);



// Draw the base of a box or input element.
void pgui_drawutil_base(
    pax_buf_t *gfx, pax_vec2i pos, pax_vec2i size, pgui_elem_t *elem, pgui_theme_t const *theme, uint32_t flags
);

// Draw the border of a box or input element.
void pgui_drawutil_border(
    pax_buf_t *gfx, pax_vec2i pos, pax_vec2i size, pgui_elem_t *elem, pgui_theme_t const *theme, uint32_t flags
);

// PAX GUI text measuring helper.
// Returns the bounds of the cursor.
pax_recti pgui_drawutil_getcursor(
    pax_font_t const *font,
    float             font_size,
    bool              shrink_to_fit,
    char const       *text,
    size_t            text_len,
    ptrdiff_t         cursorpos,
    pax_recti         bounds,
    pax_align_t       halign,
    pax_align_t       valign
);

// PAX GUI text drawing helper.
void pgui_drawutil_text(
    pax_buf_t        *gfx,
    pax_col_t         color,
    pax_font_t const *font,
    float             font_size,
    bool              shrink_to_fit,
    char const       *text,
    size_t            text_len,
    ptrdiff_t         cursorpos,
    pax_recti         bounds,
    pax_align_t       halign,
    pax_align_t       valign
);

// Draw a scrollbar.
void pgui_drawutil_scrollbar(
    pax_buf_t          *gfx,
    pax_vec2i           pos,
    pax_vec2i           size,
    pgui_theme_t const *theme,
    int                 scroll,
    int                 window,
    int                 total,
    bool                horizontal
);



// Adjust a scrollbar to show as much of the desired area as possible.
int pgui_adjust_scroll(
    int focus_offset, int focus_size, int viewport_margin, int viewport_window, int scroll, int content_size
);

// Adjust a 2D scrollbar to show as much of the desired area as possible.
pax_vec2i pgui_adjust_scroll_2d(
    pax_recti focussed_object, int viewport_margin, pax_vec2i viewport_size, pax_vec2i scroll, pax_vec2i content_size
);

// Add internal padding to a rectangle.
static inline pax_recti pgui_add_padding(pax_recti orig, int amount) {
    return (pax_recti){
        orig.x + amount,
        orig.y + amount,
        orig.w - 2 * amount,
        orig.h - 2 * amount,
    };
}

// Add internal padding to a rectangle.
static inline pax_recti pgui_add_padding4(pax_recti orig, pgui_padding_t amount) {
    return (pax_recti){
        orig.x + amount.left,
        orig.y + amount.top,
        orig.w - amount.left - amount.right,
        orig.h - amount.top - amount.bottom,
    };
}

#ifdef __cplusplus
} // extern "C"
#endif //__cplusplus

#endif // PAX_GUI_UTIL_H
