
// SPDX-License-Identifier: MIT

#include "pax_gui.h"
#include "pax_gui_internal.h"
#include "pax_gui_util.h"
#include "pax_internal.h"

static char const TAG[] = "pax-gui";



// Create a new grid / table.
pgui_elem_t *pgui_new_grid(pax_vec2i num_cells) {
    if (num_cells.x < 1 || num_cells.y < 1) {
        return NULL;
    }
    pgui_grid_t *elem = malloc(sizeof(pgui_text_t));
    if (!elem)
        return NULL;
    memset(elem, 0, sizeof(pgui_grid_t));
    elem->base.type     = &pgui_type_grid;
    elem->base.selected = -1;
    elem->cells         = num_cells;
    elem->row_height    = calloc(num_cells.y, sizeof(int));
    if (!elem->row_height) {
        free(elem);
        return NULL;
    }
    elem->col_width = calloc(num_cells.x, sizeof(int));
    if (!elem->col_width) {
        free(elem->row_height);
        free(elem);
        return NULL;
    }
    return (pgui_elem_t *)elem;
}

// Calculate the minimum size of a grid.
void pgui_calc1_grid(pax_vec2i gfx_size, pax_vec2i pos, pgui_elem_t *elem, pgui_theme_t const *theme, uint32_t flags) {
    pgui_grid_t   *grid    = (pgui_grid_t *)elem;
    pgui_padding_t padding = *pgui_effective_padding(elem, theme);

    if ((flags & PGUI_FLAG_TOPLEVEL) && elem->selected < 0) {
        // Select lowest-indexed selectable child.
        for (size_t i = 0; i < elem->children_len; i++) {
            if (elem->children[i] && (elem->children[i]->type->attr & PGUI_ATTR_SELECTABLE)) {
                elem->selected            = i;
                elem->children[i]->flags |= PGUI_FLAG_HIGHLIGHT | PGUI_FLAG_DIRTY;
                elem->flags              |= PGUI_FLAG_DIRTY;
                elem->flags              &= ~PGUI_FLAG_HIGHLIGHT;
                break;
            }
        }
    }

    // Compute column sizes.
    elem->content_size.x = 0;
    for (int x = 0; x < grid->cells.x; x++) {
        int width = padding.left + padding.right;
        for (int y = 0; y < grid->cells.y; y++) {
            if (x + y * grid->cells.x >= elem->children_len)
                continue;
            pgui_elem_t *child = elem->children[y * grid->cells.x + x];
            if (child && child->size.x > width) {
                width = child->size.x;
            }
        }
        grid->col_width[x]    = width;
        elem->content_size.x += width + padding.left + padding.right;
    }

    // Compute row sizes.
    elem->content_size.y = 0;
    for (int y = 0; y < grid->cells.y; y++) {
        int height = padding.top + padding.bottom;
        for (int x = 0; x < grid->cells.x; x++) {
            if (x + y * grid->cells.x >= elem->children_len)
                continue;
            pgui_elem_t *child = elem->children[y * grid->cells.x + x];
            if (child && child->size.y > height) {
                height = child->size.y;
            }
        }
        grid->row_height[y]   = height;
        elem->content_size.y += height + padding.top + padding.bottom;
    }

    // Update element size.
    if (!(flags & PGUI_FLAG_FIX_WIDTH)) {
        elem->size.x = elem->content_size.x;
    }
    if (!(flags & PGUI_FLAG_FIX_HEIGHT)) {
        elem->size.y = elem->content_size.y;
    }
}

// Calculate the internal layout of a grid.
void pgui_calc2_grid(pax_vec2i gfx_size, pax_vec2i pos, pgui_elem_t *elem, pgui_theme_t const *theme, uint32_t flags) {
    pgui_grid_t   *grid    = (pgui_grid_t *)elem;
    pgui_padding_t padding = *pgui_effective_padding(elem, theme);

    // // Update column sizes.
    // if (elem->size.x > elem->content_size.x) {
    //     int extra = elem->size.x - elem->content_size.x;
    //     for (int i = 0; i < grid->cells.x; i++) {
    //         grid->col_width += extra / grid->cells.x;
    //         grid->col_width += extra % grid->cells.x >= i;
    //     }
    //     elem->content_size.x = elem->size.x;
    //     elem->scroll.x       = 0;
    // }

    // // Update row sizes.
    // if (elem->size.y > elem->content_size.y) {
    //     int extra = elem->size.y - elem->content_size.y;
    //     for (int i = 0; i < grid->cells.y; i++) {
    //         grid->row_height += extra / grid->cells.y;
    //         grid->row_height += extra % grid->cells.y >= i;
    //     }
    //     elem->content_size.y = elem->size.y;
    //     elem->scroll.y       = 0;
    // }

    // Compute child element positions.
    int y_offset = padding.top;
    for (int y = 0; y < grid->cells.y; y++) {
        int x_offset = padding.left;
        for (int x = 0; x < grid->cells.x; x++) {
            if (x + y * grid->cells.x >= elem->children_len)
                continue;
            pgui_elem_t *child = elem->children[y * grid->cells.x + x];
            if (!child)
                continue;

            if (child->flags & PGUI_FLAG_FIX_WIDTH) {
                child->pos.x = x_offset + (grid->col_width[x] - child->size.x) / 2;
            } else {
                child->pos.x  = x_offset;
                child->size.x = grid->col_width[x];
            }
            if (child->flags & PGUI_FLAG_FIX_HEIGHT) {
                child->pos.y = y_offset + (grid->row_height[y] - child->size.y) / 2;
            } else {
                child->pos.y  = y_offset;
                child->size.y = grid->row_height[y];
            }
            x_offset += grid->col_width[x] + padding.left + padding.right;
        }
        y_offset += grid->row_height[y] + padding.top + padding.bottom;
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
            padding.left + padding.right + padding.top + padding.bottom,
            elem->size,
            elem->scroll,
            elem->content_size
        );
    }
}

// Draw a grid.
void pgui_draw_grid(pax_buf_t *gfx, pax_vec2i pos, pgui_elem_t *elem, pgui_theme_t const *theme, uint32_t flags) {
    pgui_grid_t   *grid    = (pgui_grid_t *)elem;
    pgui_padding_t padding = *pgui_effective_padding(elem, theme);

    // Draw cell separators.
    if (flags & PGUI_FLAG_NOSEPARATOR) {
        return;
    }
    pax_recti clip     = pax_get_clip(gfx);
    pax_recti bounds   = {elem->pos.x, elem->pos.y, elem->size.x, elem->size.y};
    int       x_offset = grid->col_width[0] - elem->scroll.x + padding.left + padding.right;
    int       y_offset = grid->row_height[0] - elem->scroll.y + padding.top + padding.bottom;
    bounds             = pgui_add_padding4(bounds, padding);
    pax_set_clip(gfx, pax_recti_intersect(clip, bounds));
    for (int y = 1; y < grid->cells.y; y++) {
        pax_draw_line(
            gfx,
            theme->palette[elem->variant].border_col,
            pos.x + 1,
            pos.y + y_offset,
            pos.x + elem->size.x - 1,
            pos.y + y_offset
        );
        y_offset += grid->row_height[y] + padding.top + padding.bottom;
    }
    for (int x = 1; x < grid->cells.x; x++) {
        pax_draw_line(
            gfx,
            theme->palette[elem->variant].border_col,
            pos.x + x_offset,
            pos.y + 1,
            pos.x + x_offset,
            pos.y + elem->size.y - 1
        );
        x_offset += grid->col_width[x] + padding.left + padding.right;
    }
    pax_set_clip(gfx, clip);
}

// Next/previous for grid elements.
static pgui_resp_t pgui_grid_next(pgui_elem_t *elem, pgui_theme_t const *theme, uint32_t flags, bool next) {
    pgui_padding_t padding = *pgui_effective_padding(elem, theme);

    // Original index.
    ptrdiff_t i0 = elem->selected;

    // Current index.
    ptrdiff_t i = elem->selected;
    if (next) {
        i = (i + 1) % elem->children_len;
    } else {
        i = (i + elem->children_len - 1) % elem->children_len;
    }

    while (i != i0) {
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
                padding.left + padding.right + padding.top + padding.bottom,
                elem->size,
                elem->scroll,
                elem->content_size
            );
            elem->flags |= PGUI_FLAG_DIRTY;
            return PGUI_RESP_CAPTURED;
        }
        if (next) {
            i = (i + 1) % elem->children_len;
        } else {
            i = (i + elem->children_len - 1) % elem->children_len;
        }
    }

    return PGUI_RESP_CAPTURED_ERR;
}

// Navigation for grid elements.
static pgui_resp_t
    pgui_grid_nav(pgui_elem_t *elem, pgui_theme_t const *theme, uint32_t flags, ptrdiff_t dx, ptrdiff_t dy) {
    pgui_grid_t   *grid    = (pgui_grid_t *)elem;
    pgui_padding_t padding = *pgui_effective_padding(elem, theme);

    // Original position.
    ptrdiff_t x0 = elem->selected % grid->cells.x;
    ptrdiff_t y0 = elem->selected / grid->cells.x;

    // Current position.
    ptrdiff_t x = (x0 + dx + grid->cells.x) % grid->cells.x;
    ptrdiff_t y = (y0 + dy + grid->cells.y) % grid->cells.y;
    while (x != x0 || y != y0) {
        ptrdiff_t i = x + y * grid->cells.x;
        if (x + y * grid->cells.x < elem->children_len && elem->children[i]
            && (elem->children[i]->type->attr & PGUI_ATTR_SELECTABLE)) {
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
                padding.left + padding.right + padding.top + padding.bottom,
                elem->size,
                elem->scroll,
                elem->content_size
            );
            elem->flags |= PGUI_FLAG_DIRTY;
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
            if (!(flags & PGUI_FLAG_TOPLEVEL)) {
                // Un-select child; re-select self.
                elem->children[elem->selected]->flags &= ~PGUI_FLAG_HIGHLIGHT;
                elem->children[elem->selected]->flags |= PGUI_FLAG_DIRTY;
                elem->selected                         = -1;
                elem->flags                           |= PGUI_FLAG_HIGHLIGHT | PGUI_FLAG_DIRTY;
                return PGUI_RESP_CAPTURED;
            } else {
                return PGUI_RESP_IGNORED;
            }

        } else if (event.input == PGUI_INPUT_NEXT) {
            // Navigate next.
            return pgui_grid_next(elem, theme, flags, true);

        } else if (event.input == PGUI_INPUT_PREV) {
            // Navigate previous.
            return pgui_grid_next(elem, theme, flags, false);

        } else if (event.input == PGUI_INPUT_UP) {
            // Navigate up.
            if (grid->cells.y == 1) {
                return PGUI_RESP_IGNORED;
            }
            return pgui_grid_nav(elem, theme, flags, 0, -1);

        } else if (event.input == PGUI_INPUT_DOWN) {
            // Navigate down.
            if (grid->cells.y == 1) {
                return PGUI_RESP_IGNORED;
            }
            return pgui_grid_nav(elem, theme, flags, 0, 1);

        } else if (event.input == PGUI_INPUT_LEFT) {
            // Navigate left.
            if (grid->cells.x == 1) {
                return PGUI_RESP_IGNORED;
            }
            return pgui_grid_nav(elem, theme, flags, -1, 0);

        } else if (event.input == PGUI_INPUT_RIGHT) {
            // Navigate right.
            if (grid->cells.x == 1) {
                return PGUI_RESP_IGNORED;
            }
            return pgui_grid_nav(elem, theme, flags, 1, 0);

        } else {
            // Other events ignored.
            return event.type == PGUI_EVENT_TYPE_RELEASE ? PGUI_RESP_CAPTURED : PGUI_RESP_CAPTURED_ERR;
        }
    }
}

// Child list changed callback for grid-based elements.
void pgui_child_grid(pgui_elem_t *elem) {
    // Disable padding on all labels.
    for (size_t i = 0; i < elem->children_len; i++) {
        if (!elem->children[i] || !(elem->children[i]->flags & PGUI_FLAG_NOBACKGROUND)
            || !(elem->children[i]->flags & PGUI_FLAG_NOBORDER))
            continue;
        elem->children[i]->flags |= PGUI_FLAG_NOPADDING;
    }
}

// Additional delete function for grid-based elements.
void pgui_del_grid(pgui_elem_t *elem) {
    pgui_grid_t *grid = (pgui_grid_t *)elem;
    free(grid->col_width);
    free(grid->row_height);
}

// Box element type.
pgui_type_t const pgui_type_grid = {
    .id    = PGUI_TYPE_ID_GRID,
    .name  = "grid",
    .attr  = PGUI_ATTR_SELECTABLE | PGUI_ATTR_CONTAINER,
    .draw  = pgui_draw_grid,
    .calc1 = pgui_calc1_grid,
    .calc2 = pgui_calc2_grid,
    .event = pgui_event_grid,
    .child = pgui_child_grid,
    .del   = pgui_del_grid,
};
