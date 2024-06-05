
// SPDX-License-Identifier: MIT

#ifndef PAX_GUI_H
#define PAX_GUI_H

#include "pax_gui_box.h"
#include "pax_gui_button.h"
#include "pax_gui_dropdown.h"
#include "pax_gui_grid.h"
#include "pax_gui_text.h"
#include "pax_gui_textbox.h"

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

// Recalculate the position of a GUI element and its children.
void        pgui_calc_layout(pgui_base_t *elem, pgui_theme_t const *theme);
// Draw a GUI element and its children.
void        pgui_draw(pax_buf_t *gfx, pgui_base_t *elem, pgui_theme_t const *theme);
// Re-draw dirty parts of the GUI and mark the elements clean.
void        pgui_redraw(pax_buf_t *gfx, pgui_base_t *elem, pgui_theme_t const *theme);
// Handle a button event.
// Returns if and how the event was handled.
pgui_resp_t pgui_event(pgui_base_t *elem, pgui_event_t event);

#ifdef __cplusplus
} // extern "C"
#endif //__cplusplus

#endif // PAX_GUI_H
