
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

// Button element type.
extern pgui_type_t pgui_type_button_raw;
#define PGUI_TYPE_BUTTON (&pgui_type_button_raw)

#endif // PAX_GUI_BUTTON_H
