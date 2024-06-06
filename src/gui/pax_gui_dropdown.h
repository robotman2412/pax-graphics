
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

// Dropdown element type.
extern pgui_type_t pgui_type_dropdown_raw;
#define PGUI_TYPE_DROPDOWN (&pgui_type_dropdown_raw)

#ifdef __cplusplus
} // extern "C"
#endif //__cplusplus

#endif // PAX_GUI_DROPDOWN_H
