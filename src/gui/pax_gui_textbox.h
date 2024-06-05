
// SPDX-License-Identifier: MIT

#ifndef PAX_GUI_TEXTBOX_H
#define PAX_GUI_TEXTBOX_H

#include "pax_gui_types.h"

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

// A single-line text input.
typedef struct pgui_textbox pgui_textbox_t;
// Textbox change callback.
typedef void (*pgui_textbox_cb_t)(pgui_textbox_t *textbox, void *cookie);


struct pgui_textbox {
    // Common GUI element data.
    pgui_base_t       base;
    // Scroll offset.
    float             scroll;
    // Cursor position.
    size_t            cursor;
    // Capacity in characters of text buffer.
    size_t            buf_cap;
    // Length in characters of text buffer.
    size_t            buf_len;
    // Text buffer.
    char             *buf;
    // Textbox change callback.
    pgui_textbox_cb_t callback;
    // Textbox change cookie.
    void             *cookie;
};



// Draw a textbox.
void pgui_draw_textbox(pax_buf_t *gfx, pax_vec2f pos, pgui_textbox_t *elem, pgui_theme_t const *theme, uint32_t flags);

// Send an event to a textbox.
pgui_resp_t pgui_event_textbox(pgui_textbox_t *elem, pgui_event_t event, uint32_t flags);

#ifdef __cplusplus
} // extern "C"
#endif //__cplusplus

#endif // PAX_GUI_TEXTBOX_H
