
// SPDX-License-Identifier: MIT

#include "pax_gui_button.h"

#include "pax_internal.h"



// Draw a button.
void pgui_draw_button(pax_buf_t *gfx, pax_vec2f pos, pgui_button_t *elem, pgui_theme_t const *theme, uint32_t flags) {
    // Draw backdrop.
    pgui_draw_base(gfx, pos, &elem->base, theme, flags);
    pgui_draw_bounded_text(
        gfx,
        theme->fg_col,
        theme->font,
        theme->font_size,
        elem->text,
        (pax_rectf){
            pos.x + theme->input_padding,
            pos.y + theme->input_padding,
            elem->base.size.x - 2 * theme->input_padding,
            elem->base.size.y - 2 * theme->input_padding,
        },
        PAX_ALIGN_CENTER
    );
}

// Send an event to a button.
pgui_resp_t pgui_event_button(pgui_button_t *elem, pgui_event_t event, uint32_t flags) {
    if (event.input != PGUI_INPUT_ACCEPT) {
        if (elem->base.flags & PGUI_FLAG_ACTIVE) {
            return event.type == PGUI_EVENT_TYPE_RELEASE ? PGUI_RESP_CAPTURED : PGUI_RESP_CAPTURED_ERR;
        }
        return PGUI_RESP_IGNORED;
    }
    if (flags & PGUI_FLAG_INACTIVE) {
        return PGUI_RESP_CAPTURED_ERR;
    }
    if (event.type == PGUI_EVENT_TYPE_PRESS) {
        elem->base.flags |= PGUI_FLAG_DIRTY;
        elem->base.flags |= PGUI_FLAG_ACTIVE;
    } else if (event.type == PGUI_EVENT_TYPE_RELEASE && flags & PGUI_FLAG_ACTIVE) {
        elem->base.flags |= PGUI_FLAG_DIRTY;
        elem->base.flags &= ~PGUI_FLAG_ACTIVE;
        if (elem->callback) {
            elem->callback(elem, elem->cookie);
        }
    }
    return PGUI_RESP_CAPTURED;
}
