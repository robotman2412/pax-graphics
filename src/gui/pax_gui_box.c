
// SPDX-License-Identifier: MIT

#include "pax_gui_box.h"

#include "pax_internal.h"



// Draw a box.
void pgui_draw_box(pax_buf_t *gfx, pax_vec2f pos, pgui_box_t *elem, pgui_theme_t const *theme, uint32_t flags) {
    pgui_draw_base(gfx, pos, &elem->base, theme, flags);
}
