
// SPDX-License-Identifier: MIT

#include "pax_gui.h"
#include "pax_internal.h"

static char const TAG[] = "pax-gui";



// Calculate the layout of a grid.
static void pgui_calc_grid(pgui_grid_t *elem, pgui_theme_t const *theme) {
    // Validate grid.
    if (elem->cells.x < 1 || elem->cells.y < 1) {
        PAX_LOGE(TAG, "Invalid grid size %dx%d", elem->cells.x, elem->cells.y);
        elem->base.flags |= PGUI_FLAG_HIDDEN;
        return;
    }
    if (elem->box.children_len != elem->cells.x * elem->cells.y) {
        PAX_LOGE(
            TAG,
            "Invalid number of children for %dx%d grid: %zu",
            elem->cells.x,
            elem->cells.y,
            elem->box.children_len
        );
        elem->base.flags |= PGUI_FLAG_HIDDEN;
        return;
    }


    // Compute size.
    pax_vec2f padded_size;
    if (elem->base.flags & PGUI_FLAG_FILLCELL) {
        // Resize cells to fit bounds.
        padded_size = (pax_vec2f){
            elem->base.size.x / elem->cells.x,
            elem->base.size.y / elem->cells.y,
        };
        elem->cell_size = (pax_vec2f){
            padded_size.x - 2 * theme->box_padding,
            padded_size.y - 2 * theme->box_padding,
        };

    } else {
        // Resize bounds to fit cells.
        padded_size = (pax_vec2f){
            elem->cell_size.x + 2 * theme->box_padding,
            elem->cell_size.y + 2 * theme->box_padding,
        };
        elem->base.size = (pax_vec2f){
            padded_size.x * elem->cells.x,
            padded_size.y * elem->cells.y,
        };
    }

    // Compute child element positions.
    for (int y = 0; y < elem->cells.y; y++) {
        for (int x = 0; x < elem->cells.x; x++) {
            pgui_base_t *child = elem->box.children[y * elem->cells.x + x];
            if (!child) {
                continue;
            }
            if (child->flags & PGUI_FLAG_FILLCELL) {
                child->pos.x  = x * padded_size.x;
                child->pos.y  = y * padded_size.y;
                child->size.x = elem->cell_size.x;
                child->size.y = elem->cell_size.y;
            } else {
                child->pos.x = x * padded_size.x + (elem->cell_size.x - child->size.x) * 0.5;
                child->pos.y = y * padded_size.y + (elem->cell_size.y - child->size.y) * 0.5;
            }
        }
    }
}

// Recalculate the position of a GUI element and its children.
void pgui_calc_layout(pgui_base_t *elem, pgui_theme_t const *theme) {
    if (!theme) {
        theme = &pgui_theme_default;
    }

    switch (elem->type) {
        default: PAX_LOGE(TAG, "Cannot calculate layout of unknown element type %d", elem->type); return;
        case PGUI_TYPE_BOX: break;
        case PGUI_TYPE_GRID: pgui_calc_grid((pgui_grid_t *)elem, theme); break;
        case PGUI_TYPE_BUTTON: break;
        case PGUI_TYPE_DROPDOWN: break;
        case PGUI_TYPE_TEXT: break;
        case PGUI_TYPE_LABEL: break;
    }

    if (PGUI_IS_BOX(elem->type)) {
        // Update flags.
        pgui_box_t *box = (pgui_box_t *)elem;
        for (size_t i = 0; i < box->children_len; i++) {
            if (box->children[i]) {
                pgui_calc_layout(box->children[i], theme);
            }
        }
    }
}
