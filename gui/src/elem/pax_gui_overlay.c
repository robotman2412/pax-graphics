
// SPDX-License-Identifier: MIT

#include "pax_gui_internal.h"
#include "pax_gui_util.h"



// Create a new overlay.
pgui_elem_t *pgui_new_overlay() {
    pgui_elem_t *elem = calloc(1, sizeof(pgui_elem_t));
    if (!elem)
        return NULL;
    elem->type     = &pgui_type_overlay;
    elem->flags    = PGUI_FLAG_NOBACKGROUND | PGUI_FLAG_NOBORDER | PGUI_FLAG_NOPADDING;
    elem->selected = -1;
    return elem;
}

// Calculate the minimum size of overlay elements.
void pgui_calc1_overlay(
    pax_vec2i gfx_size, pax_vec2i pos, pgui_elem_t *elem, pgui_theme_t const *theme, uint32_t flags
) {
    int padding = flags & PGUI_FLAG_NOPADDING ? 0 : 2 * theme->padding;

    if (!(flags & PGUI_FLAG_FIX_WIDTH)) {
        elem->size.x = 0;
        // Clamp minimum width.
        for (size_t i = 0; i < elem->children_len; i++) {
            if (elem->children[i] && elem->size.x < elem->children[i]->size.x + padding) {
                elem->size.x = elem->children[i]->size.x + padding;
            }
        }
    }
    if (!(flags & PGUI_FLAG_FIX_HEIGHT)) {
        elem->size.y = 0;
        // Clamp minimum height.
        for (size_t i = 0; i < elem->children_len; i++) {
            if (elem->children[i] && elem->size.y < elem->children[i]->size.y + padding) {
                elem->size.y = elem->children[i]->size.y + padding;
            }
        }
    }
}

// Calculate the internal layout of overlay elements.
void pgui_calc2_overlay(
    pax_vec2i gfx_size, pax_vec2i pos, pgui_elem_t *elem, pgui_theme_t const *theme, uint32_t flags
) {
    int padding = flags & PGUI_FLAG_NOPADDING ? 0 : theme->padding;

    for (size_t i = 0; i < elem->children_len; i++) {
        if (!elem->children[i])
            continue;
        pgui_elem_t *child = elem->children[i];

        if (child->flags & PGUI_FLAG_FIX_WIDTH) {
            child->pos.x = (elem->size.x - child->size.x) / 2;
        } else {
            child->pos.x  = padding;
            child->size.x = elem->size.x - 2 * padding;
        }
        if (child->flags & PGUI_FLAG_FIX_HEIGHT) {
            child->pos.y = (elem->size.y - child->size.y) / 2;
        } else {
            child->pos.y  = padding;
            child->size.y = elem->size.y - 2 * padding;
        }
    }
}

// Overlay element type.
pgui_type_t const pgui_type_overlay = {
    .id    = PGUI_TYPE_ID_OVERLAY,
    .name  = "overlay",
    .attr  = 0,
    .calc1 = pgui_calc1_overlay,
    .calc2 = pgui_calc2_overlay,
};
