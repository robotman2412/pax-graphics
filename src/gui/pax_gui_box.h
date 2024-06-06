
// SPDX-License-Identifier: MIT

#ifndef PAX_GUI_BOX_H
#define PAX_GUI_BOX_H

#include "pax_gui_types.h"

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

// Contains other GUI elements, which may include other boxes.
// This is the abstract container and does not have any logic for selecting children.
// However, selected children will still receive input events.
typedef struct pgui_box pgui_box_t;

struct pgui_box {
    // Common GUI element data.
    pgui_base_t   base;
    // Index of selected child, if any.
    ptrdiff_t     selected;
    // Number of children.
    size_t        children_len;
    // Pointers to child elements.
    pgui_base_t **children;
};

// Box element type.
extern pgui_type_t pgui_type_box_raw;
#define PGUI_TYPE_BOX (&pgui_type_box_raw)

#ifdef __cplusplus
} // extern "C"
#endif //__cplusplus

#endif // PAX_GUI_BOX_H
