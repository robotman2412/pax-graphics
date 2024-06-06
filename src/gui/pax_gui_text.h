
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

// Text element type.
extern pgui_type_t pgui_type_text_raw;
#define PGUI_TYPE_TEXT (&pgui_type_text_raw)

// Label element type.
extern pgui_type_t pgui_type_label_raw;
#define PGUI_TYPE_LABEL (&pgui_type_label_raw)

#ifdef __cplusplus
} // extern "C"
#endif //__cplusplus

#endif // PAX_GUI_TEXT_H
