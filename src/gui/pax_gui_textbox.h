
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

// Textbox element type.
extern pgui_type_t pgui_type_textbox_raw;
#define PGUI_TYPE_TEXTBOX (&pgui_type_textbox_raw)

#ifdef __cplusplus
} // extern "C"
#endif //__cplusplus

#endif // PAX_GUI_TEXTBOX_H
