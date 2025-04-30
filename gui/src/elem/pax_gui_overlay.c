
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

// Calculate the internal layout of overlay elements.
void pgui_calc2_overlay(
    pax_vec2i gfx_size, pax_vec2i pos, pgui_elem_t *elem, pgui_theme_t const *theme, uint32_t flags
) {
    (void)gfx_size;
    (void)pos;
    (void)flags;
    pgui_padding_t padding = *pgui_effective_padding(elem, theme);

    for (size_t i = 0; i < elem->children_len; i++) {
        if (!elem->children[i])
            continue;
        pgui_elem_t *child = elem->children[i];

        if (child->flags & PGUI_FLAG_FIX_WIDTH) {
            child->pos.x = (elem->size.x - child->size.x) / 2;
        } else {
            child->pos.x  = padding.left;
            child->size.x = elem->size.x - padding.left - padding.right;
        }
        if (child->flags & PGUI_FLAG_FIX_HEIGHT) {
            child->pos.y = (elem->size.y - child->size.y) / 2;
        } else {
            child->pos.y  = padding.left;
            child->size.y = elem->size.y - padding.top + padding.bottom;
        }
    }
}

// Overlay element type.
pgui_type_t const pgui_type_overlay = {
    .id          = PGUI_TYPE_ID_OVERLAY,
    .base_struct = PGUI_STRUCT_BASE,
    .name        = "overlay",
    .attr        = PGUI_ATTR_CONTAINER,
    .calc1       = pgui_calc1_dropdown,
    .calc2       = pgui_calc2_overlay,
};
