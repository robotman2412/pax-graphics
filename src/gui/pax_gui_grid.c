
// SPDX-License-Identifier: MIT

#include "pax_gui.h"
#include "pax_gui_util.h"
#include "pax_internal.h"

static char const TAG[] = "pax-gui";



// Calculate the layout of a grid.
void pgui_calc_grid(pax_vec2i gfx_size, pax_vec2i pos, pgui_elem_t *elem, pgui_theme_t const *theme, uint32_t flags) {
    pgui_grid_t *grid    = (pgui_grid_t *)elem;
    int          padding = flags & PGUI_FLAG_NOPADDING ? 0 : theme->padding;

    // Validate grid.
    if (grid->cells.x < 1 || grid->cells.y < 1) {
        return;
    }
    if (elem->children_len != grid->cells.x * grid->cells.y) {
        return;
    }

    // Compute column sizes.
    elem->content_size.x = 0;
    for (int x = 0; x < grid->cells.x; x++) {
        int width = 2 * padding;
        for (int y = 0; y < grid->cells.y; y++) {
            pgui_elem_t *child = elem->children[y * grid->cells.x + x];
            if (child && child->size.x > width) {
                width = child->size.x;
            }
        }
        grid->col_width[x]    = width;
        elem->content_size.x += width + 2 * padding;
    }

    // Compute row sizes.
    elem->content_size.y = 0;
    for (int y = 0; y < grid->cells.y; y++) {
        int height = 2 * padding;
        for (int x = 0; x < grid->cells.x; x++) {
            pgui_elem_t *child = elem->children[y * grid->cells.x + x];
            if (child && child->size.y > height) {
                height = child->size.y;
            }
        }
        grid->row_height[y]   = height;
        elem->content_size.y += height + 2 * padding;
    }

    // Update element size.
    if (elem->flags & PGUI_FLAG_FILLCELL) {
        // TODO.
    } else {
        elem->size = elem->content_size;
    }

    // Compute child element positions.
    int y_offset = padding;
    for (int y = 0; y < grid->cells.y; y++) {
        int x_offset = padding;
        for (int x = 0; x < grid->cells.x; x++) {
            pgui_elem_t *child = elem->children[y * grid->cells.x + x];
            if (!child) {
                continue;
            }
            if (child->flags & PGUI_FLAG_FILLCELL) {
                child->pos.x  = x_offset;
                child->pos.y  = y_offset;
                child->size.x = grid->col_width[x];
                child->size.y = grid->row_height[y];
            } else {
                child->pos.x = x_offset + (grid->col_width[x] - child->size.x) / 2;
                child->pos.y = y_offset + (grid->row_height[y] - child->size.y) / 2;
            }
            x_offset += grid->col_width[x] + 2 * padding;
        }
        y_offset += grid->row_height[y] + 2 * padding;
    }

    if (elem->selected >= 0 && elem->selected < elem->children_len) {
        // Update scroll position.
        elem->scroll = pgui_adjust_scroll_2d(
            (pax_recti){
                elem->children[elem->selected]->pos.x,
                elem->children[elem->selected]->pos.y,
                elem->children[elem->selected]->size.x,
                elem->children[elem->selected]->size.y,
            },
            4 * padding,
            elem->size,
            elem->scroll,
            elem->content_size
        );
    }
}

// Draw a grid.
void pgui_draw_grid(pax_buf_t *gfx, pax_vec2i pos, pgui_elem_t *elem, pgui_theme_t const *theme, uint32_t flags) {
    pgui_grid_t *grid    = (pgui_grid_t *)elem;
    int          padding = flags & PGUI_FLAG_NOPADDING ? 0 : theme->padding;

    // Validate grid.
    if (grid->cells.x < 1 || grid->cells.y < 1) {
        PAX_LOGE(TAG, "Invalid grid size %dx%d", grid->cells.x, grid->cells.y);
        elem->flags |= PGUI_FLAG_HIDDEN;
        return;
    }
    if (elem->children_len != grid->cells.x * grid->cells.y) {
        PAX_LOGE(
            TAG,
            "Invalid number of children for %dx%d grid: %zu",
            grid->cells.x,
            grid->cells.y,
            elem->children_len
        );
        elem->flags |= PGUI_FLAG_HIDDEN;
        return;
    }

    // Draw cell separators.
    if (flags & PGUI_FLAG_NOSEPARATOR) {
        return;
    }
    pax_recti clip     = pax_get_clip(gfx);
    pax_recti bounds   = {elem->pos.x, elem->pos.y, elem->size.x, elem->size.y};
    int       x_offset = grid->col_width[0] - elem->scroll.x + 2 * padding;
    int       y_offset = grid->row_height[0] - elem->scroll.y + 2 * padding;
    bounds             = pgui_add_padding(bounds, padding);
    pax_set_clip(gfx, pax_recti_intersect(clip, bounds));
    for (int y = 1; y < grid->cells.y; y++) {
        pax_draw_line(gfx, theme->border_col, pos.x + 1, pos.y + y_offset, pos.x + elem->size.x - 1, pos.y + y_offset);
        y_offset += grid->row_height[y] + 2 * padding;
    }
    for (int x = 1; x < grid->cells.x; x++) {
        pax_draw_line(gfx, theme->border_col, pos.x + x_offset, pos.y + 1, pos.x + x_offset, pos.y + elem->size.y - 1);
        x_offset += grid->col_width[x] + 2 * padding;
    }
    pax_set_clip(gfx, clip);
}

// Navigation for grid elements.
static pgui_resp_t
    pgui_grid_nav(pgui_elem_t *elem, pgui_theme_t const *theme, uint32_t flags, ptrdiff_t dx, ptrdiff_t dy) {
    pgui_grid_t *grid    = (pgui_grid_t *)elem;
    int          padding = flags & PGUI_FLAG_NOPADDING ? 0 : theme->padding;

    // Original position.
    ptrdiff_t x0 = elem->selected % grid->cells.x;
    ptrdiff_t y0 = elem->selected / grid->cells.x;

    // Current position.
    ptrdiff_t x = (x0 + dx + grid->cells.x) % grid->cells.x;
    ptrdiff_t y = (y0 + dy + grid->cells.y) % grid->cells.y;
    while (x != x0 || y != y0) {
        ptrdiff_t i = x + y * grid->cells.x;
        if (elem->children[i] && (elem->children[i]->type->attr & PGUI_ATTR_SELECTABLE)) {
            if (elem->selected >= 0) {
                // Unmark previous selection.
                elem->children[elem->selected]->flags &= ~PGUI_FLAG_HIGHLIGHT;
                elem->children[elem->selected]->flags |= PGUI_FLAG_DIRTY;
            }

            // Mark new selection.
            elem->selected            = i;
            elem->children[i]->flags |= PGUI_FLAG_HIGHLIGHT | PGUI_FLAG_DIRTY;

            // Update scroll position.
            elem->scroll = pgui_adjust_scroll_2d(
                (pax_recti){
                    elem->children[i]->pos.x,
                    elem->children[i]->pos.y,
                    elem->children[i]->size.x,
                    elem->children[i]->size.y,
                },
                4 * padding,
                elem->size,
                elem->scroll,
                elem->content_size
            );
            return PGUI_RESP_CAPTURED;
        }
        x = (x + dx + grid->cells.x) % grid->cells.x;
        y = (y + dy + grid->cells.y) % grid->cells.y;
    }

    return PGUI_RESP_CAPTURED_ERR;
}

// Send an event to a grid.
pgui_resp_t pgui_event_grid(
    pax_vec2i gfx_size, pax_vec2i pos, pgui_elem_t *elem, pgui_theme_t const *theme, uint32_t flags, pgui_event_t event
) {
    pgui_grid_t *grid = (pgui_grid_t *)elem;

    // Validate grid.
    if (grid->cells.x < 1 || grid->cells.y < 1) {
        return PGUI_RESP_IGNORED;
    }
    if (elem->children_len != grid->cells.x * grid->cells.y) {
        return PGUI_RESP_IGNORED;
    }

    if (elem->selected < 0 || elem->selected >= elem->children_len) {
        if (event.input == PGUI_INPUT_ACCEPT && event.type == PGUI_EVENT_TYPE_RELEASE) {
            // Select lowest-indexed selectable child.
            for (size_t i = 0; i < elem->children_len; i++) {
                if (elem->children[i] && (elem->children[i]->type->attr & PGUI_ATTR_SELECTABLE)) {
                    elem->selected            = i;
                    elem->children[i]->flags |= PGUI_FLAG_HIGHLIGHT | PGUI_FLAG_DIRTY;
                    elem->flags              |= PGUI_FLAG_DIRTY;
                    elem->flags              &= ~PGUI_FLAG_HIGHLIGHT;
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
            elem->children[elem->selected]->flags &= ~PGUI_FLAG_HIGHLIGHT;
            elem->children[elem->selected]->flags |= PGUI_FLAG_DIRTY;
            elem->selected                         = -1;
            elem->flags                           |= PGUI_FLAG_HIGHLIGHT | PGUI_FLAG_DIRTY;
            return PGUI_RESP_CAPTURED;

        } else if (event.input == PGUI_INPUT_UP) {
            // Navigate up.
            return pgui_grid_nav(elem, theme, flags, 0, -1);

        } else if (event.input == PGUI_INPUT_DOWN) {
            // Navigate down.
            return pgui_grid_nav(elem, theme, flags, 0, 1);

        } else if (event.input == PGUI_INPUT_LEFT) {
            // Navigate left.
            return pgui_grid_nav(elem, theme, flags, -1, 0);

        } else if (event.input == PGUI_INPUT_RIGHT) {
            // Navigate right.
            return pgui_grid_nav(elem, theme, flags, 1, 0);
        } else {
            // Other events ignored.
            return event.type == PGUI_EVENT_TYPE_RELEASE ? PGUI_RESP_CAPTURED : PGUI_RESP_CAPTURED_ERR;
        }
    }
}

// Box element type.
pgui_type_t pgui_type_grid_raw = {
    .attr  = PGUI_ATTR_SELECTABLE,
    .draw  = pgui_draw_grid,
    .calc  = pgui_calc_grid,
    .event = pgui_event_grid,
};
