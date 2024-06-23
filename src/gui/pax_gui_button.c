
// SPDX-License-Identifier: MIT

#include "pax_gui_util.h"

static char const TAG[] = "pax-gui";



// Behaviour for button elements.
pgui_resp_t pgui_event_button(
    pax_vec2i gfx_size, pax_vec2i pos, pgui_elem_t *elem, pgui_theme_t const *theme, uint32_t flags, pgui_event_t event
) {
    if (event.input != PGUI_INPUT_ACCEPT) {
        if (elem->flags & PGUI_FLAG_ACTIVE) {
            return event.type == PGUI_EVENT_TYPE_RELEASE ? PGUI_RESP_CAPTURED : PGUI_RESP_CAPTURED_ERR;
        }
        return PGUI_RESP_IGNORED;
    }
    if (flags & PGUI_FLAG_INACTIVE) {
        return PGUI_RESP_CAPTURED_ERR;
    }
    if (event.type == PGUI_EVENT_TYPE_PRESS) {
        elem->flags |= PGUI_FLAG_DIRTY;
        elem->flags |= PGUI_FLAG_ACTIVE;
    } else if (event.type == PGUI_EVENT_TYPE_RELEASE && flags & PGUI_FLAG_ACTIVE) {
        elem->flags |= PGUI_FLAG_DIRTY;
        elem->flags &= ~PGUI_FLAG_ACTIVE;
        if (elem->callback) {
            elem->callback(elem);
        }
    }
    return PGUI_RESP_CAPTURED;
}

// Button element type.
pgui_type_t pgui_type_button_raw = {
    .attr     = PGUI_ATTR_SELECTABLE | PGUI_ATTR_BUTTON,
    .min_size = {100, 30},
    .draw     = pgui_draw_text,
    .event    = pgui_event_button,
};
