
// SPDX-License-Identifier: MIT

#include "pax_gui_internal.h"
#include "pax_gui_util.h"

static char const TAG[] = "pax-gui";



// Create a new button.
pgui_elem_t *pgui_new_button(char const *text, pgui_callback_t cb) {
    pgui_text_t *elem = malloc(sizeof(pgui_text_t));
    if (!elem)
        return NULL;
    memset(elem, 0, sizeof(pgui_text_t));
    elem->base.type     = &pgui_type_button;
    elem->base.callback = cb;
    elem->base.selected = -1;
    elem->text          = (char *)text;
    elem->text_len      = text ? strlen(text) : 0;
    elem->text_halign   = PAX_ALIGN_CENTER;
    elem->text_valign   = PAX_ALIGN_CENTER;
    return (pgui_elem_t *)elem;
}

// Calculate the minimum size of button elements.
void pgui_calc1_button(
    pax_vec2i gfx_size, pax_vec2i pos, pgui_elem_t *elem, pgui_theme_t const *theme, uint32_t flags
) {
    // Inherit calculation from text element.
    pgui_calc1_text(gfx_size, pos, elem, theme, flags);

    pgui_padding_t padding = *pgui_effective_padding(elem, theme);

    if (!(flags & PGUI_FLAG_FIX_WIDTH)) {
        // Clamp minimum width.
        for (size_t i = 0; i < elem->children_len; i++) {
            if (elem->children[i] && elem->size.x < elem->children[i]->size.x + padding.left + padding.right) {
                elem->size.x = elem->children[i]->size.x + padding.left + padding.right;
            }
        }
    }
    if (!(flags & PGUI_FLAG_FIX_HEIGHT)) {
        // Clamp minimum height.
        for (size_t i = 0; i < elem->children_len; i++) {
            if (elem->children[i] && elem->size.y < elem->children[i]->size.y + padding.top + padding.bottom) {
                elem->size.y = elem->children[i]->size.y + padding.top + padding.bottom;
            }
        }
    }
}

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
pgui_type_t const pgui_type_button = {
    .id    = PGUI_TYPE_ID_BUTTON,
    .name  = "button",
    .attr  = PGUI_ATTR_SELECTABLE | PGUI_ATTR_BUTTON | PGUI_ATTR_TEXTSTRUCT | PGUI_ATTR_CONTAINER,
    .draw  = pgui_draw_text,
    .calc1 = pgui_calc1_button,
    .calc2 = pgui_calc2_overlay,
    .event = pgui_event_button,
    .del   = pgui_del_text,
};
