
// SPDX-License-Identifier: MIT

#include "pax_gui_grid.h"

#include "pax_internal.h"

static char const TAG[] = "pax-gui";



// Calculate the layout of a grid.
void pgui_calc_grid(pgui_grid_t *elem, pgui_theme_t const *theme) {
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

// Draw a grid.
void pgui_draw_grid(pax_buf_t *gfx, pax_vec2f pos, pgui_grid_t *elem, pgui_theme_t const *theme, uint32_t flags) {
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

    pax_vec2f padded_size = {
        elem->cell_size.x + 2 * theme->box_padding,
        elem->cell_size.y + 2 * theme->box_padding,
    };

    // Draw background.
    pgui_draw_base(gfx, pos, &elem->base, theme, flags);
    // Draw cell separators.
    if (flags & PGUI_FLAG_NOSEPARATOR) {
        return;
    }
    for (int y = 1; y < elem->cells.y; y++) {
        pax_draw_line(
            gfx,
            theme->border_col,
            pos.x + 1,
            pos.y + padded_size.y * y,
            pos.x + elem->base.size.x - 1,
            pos.y + padded_size.y * y
        );
    }
    for (int x = 1; x < elem->cells.x; x++) {
        pax_draw_line(
            gfx,
            theme->border_col,
            pos.x + padded_size.x * x,
            pos.y + 1,
            pos.x + padded_size.x * x,
            pos.y + elem->base.size.y - 1
        );
    }
}

// Navigation for grid elements.
static pgui_resp_t pgui_grid_nav(pgui_grid_t *elem, ptrdiff_t dx, ptrdiff_t dy) {
    // Original position.
    ptrdiff_t x0 = elem->box.selected % elem->cells.x;
    ptrdiff_t y0 = elem->box.selected / elem->cells.x;

    // Current position.
    ptrdiff_t x = (x0 + dx + elem->cells.x) % elem->cells.x;
    ptrdiff_t y = (y0 + dy + elem->cells.y) % elem->cells.y;
    while (x != x0 || y != y0) {
        ptrdiff_t i = x + y * elem->cells.x;
        if (elem->box.children[i] && PGUI_IS_SELECTABLE(elem->box.children[i]->type)) {
            if (elem->box.selected >= 0) {
                // Unmark previous selection.
                elem->box.children[elem->box.selected]->flags &= ~PGUI_FLAG_HIGHLIGHT;
                elem->box.children[elem->box.selected]->flags |= PGUI_FLAG_DIRTY;
            }
            // Mark new selection.
            elem->box.selected            = i;
            elem->box.children[i]->flags |= PGUI_FLAG_HIGHLIGHT | PGUI_FLAG_DIRTY;
            return PGUI_RESP_CAPTURED;
        }
        x = (x + dx + elem->cells.x) % elem->cells.x;
        y = (y + dy + elem->cells.y) % elem->cells.y;
    }

    return PGUI_RESP_CAPTURED_ERR;
}

// Send an event to a grid.
pgui_resp_t pgui_event_grid(pgui_grid_t *elem, pgui_event_t event, uint32_t flags) {
    if (elem->box.selected < 0) {
        if (event.input == PGUI_INPUT_ACCEPT && event.type == PGUI_EVENT_TYPE_RELEASE) {
            // Select lowest-indexed selectable child.
            for (size_t i = 0; i < elem->box.children_len; i++) {
                if (elem->box.children[i] && PGUI_IS_SELECTABLE(elem->box.children[i]->type)) {
                    elem->box.selected            = i;
                    elem->box.children[i]->flags |= PGUI_FLAG_HIGHLIGHT | PGUI_FLAG_DIRTY;
                    elem->base.flags             |= PGUI_FLAG_DIRTY;
                    elem->base.flags             &= ~PGUI_FLAG_HIGHLIGHT;
                    return PGUI_RESP_CAPTURED;
                }
            }
            return PGUI_RESP_CAPTURED_ERR;
        } else if (event.input == PGUI_INPUT_ACCEPT) {
            // Selecting happens on release, not press.
            return PGUI_RESP_CAPTURED;
        } else {
            // Other inputs while not selected are ignored.
            return PGUI_RESP_IGNORED;
        }

    } else {
        if (event.type == PGUI_EVENT_TYPE_RELEASE) {
            // No action on button release.
            return PGUI_RESP_CAPTURED;

        } else if (event.input == PGUI_INPUT_BACK && event.type == PGUI_EVENT_TYPE_PRESS) {
            // Un-select child; re-select self.
            elem->box.children[elem->box.selected]->flags &= ~PGUI_FLAG_HIGHLIGHT;
            elem->box.children[elem->box.selected]->flags |= PGUI_FLAG_DIRTY;
            elem->box.selected                             = -1;
            elem->base.flags                              |= PGUI_FLAG_HIGHLIGHT | PGUI_FLAG_DIRTY;

        } else if (event.input == PGUI_INPUT_UP) {
            // Navigate up.
            return pgui_grid_nav(elem, 0, -1);

        } else if (event.input == PGUI_INPUT_DOWN) {
            // Navigate down.
            return pgui_grid_nav(elem, 0, 1);

        } else if (event.input == PGUI_INPUT_LEFT) {
            // Navigate left.
            return pgui_grid_nav(elem, -1, 0);

        } else if (event.input == PGUI_INPUT_RIGHT) {
            // Navigate right.
            return pgui_grid_nav(elem, 1, 0);
        } else {
            // Other events ignored.
            return event.type == PGUI_EVENT_TYPE_RELEASE ? PGUI_RESP_CAPTURED : PGUI_RESP_CAPTURED_ERR;
        }
    }
}
