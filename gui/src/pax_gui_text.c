
// SPDX-License-Identifier: MIT

#include "pax_gui_util.h"



// Visuals for text-based elements.
void pgui_draw_text(pax_buf_t *gfx, pax_vec2i pos, pgui_elem_t *elem, pgui_theme_t const *theme, uint32_t flags) {
    pgui_drawutil_textbox(gfx, pos, elem, theme, flags, false);
}

// Text element type.
pgui_type_t pgui_type_text_raw = {
    .min_size = {100, 30},
    .draw     = pgui_draw_text,
};
