
// SPDX-License-Identifier: MIT

#ifndef PAX_GUI_BUTTON_H
#define PAX_GUI_BUTTON_H

#include "pax_gui_types.h"

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

#ifdef __cplusplus
} // extern "C"
#endif //__cplusplus

// A button that can be pressed, optionally calling a function.
typedef struct pgui_button pgui_button_t;
// Button press callback.
typedef void (*pgui_button_cb_t)(pgui_button_t *button, void *cookie);

struct pgui_button {
    // Common GUI element data.
    pgui_base_t      base;
    // Button text.
    char const      *text;
    // Button press callback.
    pgui_button_cb_t callback;
    // Button press cookie.
    void            *cookie;
};

// Draw a button.
void pgui_draw_button(pax_buf_t *gfx, pax_vec2f pos, pgui_button_t *elem, pgui_theme_t const *theme, uint32_t flags);

// Send an event to a button.
pgui_resp_t pgui_event_button(pgui_button_t *elem, pgui_event_t event, uint32_t flags);

#endif // PAX_GUI_BUTTON_H
