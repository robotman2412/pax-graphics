
// SPDX-License-Identifier: MIT

#ifndef PAX_GUI_DROPDOWN_H
#define PAX_GUI_DROPDOWN_H

#include "pax_gui_types.h"

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

// A drop-down menu for selecting from a fixed set of options.
typedef struct pgui_dropdown pgui_dropdown_t;
// Dropdown select callback.
typedef void (*pgui_dropdown_cb_t)(pgui_dropdown_t *dropdown, void *cookie);

struct pgui_dropdown {
    // Common GUI element data.
    pgui_base_t        base;
    // Scroll offset.
    float              scroll;
    // Selected option.
    size_t             selected;
    // Option hovered in the selection menu.
    size_t             to_select;
    // Number of options.
    size_t             options_len;
    // Option text.
    char const       **options;
    // Dropdown select callback.
    pgui_dropdown_cb_t callback;
    // Dropdown select cookie.
    void              *cookie;
};



// Draw a dropdown.
void pgui_draw_dropdown(
    pax_buf_t *gfx, pax_vec2f pos, pgui_dropdown_t *elem, pgui_theme_t const *theme, uint32_t flags
);

// Send an event to a dropdown.
pgui_resp_t pgui_event_dropdown(pgui_dropdown_t *elem, pgui_event_t event, uint32_t flags);

#ifdef __cplusplus
} // extern "C"
#endif //__cplusplus

#endif // PAX_GUI_DROPDOWN_H
