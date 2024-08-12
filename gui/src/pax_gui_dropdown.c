
// SPDX-License-Identifier: MIT

#include "pax_gui_util.h"

static char const TAG[] = "pax-gui";



// Child clipping rectangle for dropdowns.
void pgui_clip_dropdown(pax_buf_t *gfx, pax_vec2i pos, pgui_elem_t *elem, pgui_theme_t const *theme, uint32_t flags) {
    pgui_dropdown_t *dropdown = (pgui_dropdown_t *)elem;
    if (pos.x != dropdown->last_pos.x || pos.y != dropdown->last_pos.y) {
        pgui_calc_dropdown(pax_buf_get_dims(gfx), pos, elem, theme, flags);
    }
    if (flags & PGUI_FLAG_ACTIVE) {
        pax_set_clip(gfx, dropdown->child_pos);
    }
}

// GUI element draw call.
void pgui_draw_dropdown(pax_buf_t *gfx, pax_vec2i pos, pgui_elem_t *elem, pgui_theme_t const *theme, uint32_t flags) {
    pgui_dropdown_t *dropdown = (pgui_dropdown_t *)elem;
    pax_recti        clip     = pax_get_clip(gfx);
    if (pos.x != dropdown->last_pos.x || pos.y != dropdown->last_pos.y) {
        pgui_calc_dropdown(pax_buf_get_dims(gfx), pos, elem, theme, flags);
    }

    // Draw box around the location of the children.
    if (flags & PGUI_FLAG_ACTIVE) {
        pax_noclip(gfx);
        pax_draw_round_rect(
            gfx,
            theme->button_col,
            dropdown->child_pos.x,
            dropdown->child_pos.y,
            dropdown->child_pos.w,
            dropdown->child_pos.h,
            theme->rounding
        );
        pax_clip(
            gfx,
            dropdown->child_pos.x,
            dropdown->child_pos.y + elem->selected * elem->size.y - elem->scroll.y,
            dropdown->child_pos.w,
            elem->size.y
        );
        pax_draw_round_rect(
            gfx,
            theme->pressed_col,
            dropdown->child_pos.x,
            dropdown->child_pos.y,
            dropdown->child_pos.w,
            dropdown->child_pos.h,
            theme->rounding
        );
        pax_noclip(gfx);
        pgui_drawutil_border(
            gfx,
            (pax_vec2i){dropdown->child_pos.x, dropdown->child_pos.y},
            (pax_vec2i){dropdown->child_pos.w, dropdown->child_pos.h},
            elem,
            theme,
            flags & ~PGUI_FLAG_NOBORDER
        );
    }

    if (!theme->dropdown_covering_menu || !(flags & PGUI_FLAG_ACTIVE)) {
        pgui_elem_t *child = elem->children[dropdown->selected];

        // Draw current selection on the dropdown.
        pax_recti bounds = {pos.x + theme->padding, pos.y + theme->padding, elem->size.x, elem->size.y};
        pax_set_clip(gfx, pax_recti_intersect(clip, bounds));
        uint32_t  child_flags = (flags & PGUI_FLAGS_INHERITABLE) | child->flags;
        pax_vec2i child_pos   = {pos.x + theme->padding, pos.y + theme->padding};

        // Explicitly draw child.
        pgui_drawutil_base(gfx, child_pos, child->size, child, theme, child_flags);
        child->type->draw(gfx, child_pos, child, theme, child_flags);
        pgui_drawutil_border(gfx, child_pos, child->size, child, theme, child_flags);
    }

    // Draw the segmenting line.
    pax_noclip(gfx);
    if (theme->dropdown_segmented) {
        pax_draw_line(
            gfx,
            theme->border_col,
            pos.x + elem->size.x - elem->size.y,
            pos.y + 1,
            pos.x + elem->size.x - elem->size.y,
            pos.y + elem->size.y - 1
        );
    }

    // Draw arrow.
    pax_push_2d(gfx);
    pax_apply_2d(gfx, matrix_2d_translate(pos.x + elem->size.x, pos.y));
    pax_apply_2d(gfx, matrix_2d_scale(elem->size.y, elem->size.y));
    pax_apply_2d(gfx, matrix_2d_translate(-0.5f, 0.5f));
    if (flags & PGUI_FLAG_ACTIVE) {
        pax_apply_2d(gfx, matrix_2d_scale(1, -1));
    }
    if (theme->dropdown_solid_arrow) {
        pax_draw_tri(gfx, theme->fg_col, -0.129903811f, -0.075, 0.129903811f, -0.075, 0, 0.15f);
    } else {
        pax_draw_line(gfx, theme->fg_col, -0.2f, -0.1f, 0, 0.1f);
        pax_draw_line(gfx, theme->fg_col, +0.2f, -0.1f, 0, 0.1f);
    }
    pax_pop_2d(gfx);

    // Restore clip rectangle.
    pax_set_clip(gfx, clip);
}

// GUI element layout calculation call.
void pgui_calc_dropdown(
    pax_vec2i gfx_size, pax_vec2i pos, pgui_elem_t *elem, pgui_theme_t const *theme, uint32_t flags
) {
    pgui_dropdown_t *dropdown = (pgui_dropdown_t *)elem;
    dropdown->last_pos        = pos;

    if (flags & PGUI_FLAG_ACTIVE) {
        // Show all children.
        for (size_t i = 0; i < elem->children_len; i++) {
            elem->children[i]->flags &= ~PGUI_FLAG_HIDDEN;
        }
    } else {
        // Hide all children.
        for (size_t i = 0; i < elem->children_len; i++) {
            elem->children[i]->flags |= PGUI_FLAG_HIDDEN;
        }
    }

    // Calculate element size.
    elem->size           = theme->min_input_size;
    elem->content_size.y = 0;
    for (size_t i = 0; i < elem->children_len; i++) {
        elem->content_size.y += 2 * theme->padding + elem->children[i]->size.y;
        if (elem->size.x < elem->children[i]->size.x + 2 * theme->padding) {
            elem->size.x = elem->children[i]->size.x + 2 * theme->padding;
        }
        if (elem->size.y < elem->children[i]->size.y + 2 * theme->padding) {
            elem->size.y = elem->children[i]->size.y + 2 * theme->padding;
        }
    }
    elem->content_size.x = elem->size.x;

    // Choose on-screen position for children.
    int extra             = theme->dropdown_covering_menu * elem->size.y;
    int top_space         = extra + pos.y;
    int bottom_space      = extra + gfx_size.y - pos.y - elem->size.y;
    dropdown->child_pos.h = elem->content_size.y;
    if (bottom_space >= elem->content_size.y || bottom_space >= top_space) {
        // Go towards the bottom.
        dropdown->child_pos.y = pos.y + elem->size.y - extra;
        if (bottom_space < elem->content_size.y) {
            dropdown->child_pos.h = bottom_space;
        }
    } else {
        // Go towards the top.
        if (top_space < elem->content_size.y) {
            dropdown->child_pos.h = top_space;
        }
        dropdown->child_pos.y = pos.y + extra - dropdown->child_pos.h;
    }
    dropdown->child_pos.x = pos.x;
    dropdown->child_pos.w = elem->size.x;

    // Calculate child element positions.
    int x_offset = dropdown->child_pos.x;
    int y_offset = dropdown->child_pos.y;
    if (!theme->dropdown_covering_menu) {
        y_offset += elem->size.y;
    }
    for (size_t i = 0; i < elem->children_len; i++) {
        pgui_elem_t *child = elem->children[i];
        if (!child) {
            continue;
        }
        if (child->flags & PGUI_FLAG_FILLCELL) {
            child->pos.x  = x_offset + theme->padding;
            child->pos.y  = y_offset + theme->padding;
            child->size.x = elem->size.x - 2 * theme->padding;
            child->size.y = elem->size.y - 2 * theme->padding;
        } else {
            child->pos.x = x_offset + (elem->size.x - child->size.x) / 2;
            child->pos.y = y_offset + (elem->size.y - child->size.y) / 2;
        }
        y_offset += elem->size.y;
    }

    if (elem->selected >= 0 && elem->selected < elem->children_len) {
        // Update scroll position.
        elem->scroll.y = pgui_adjust_scroll(
            elem->size.y * elem->selected,
            elem->size.y,
            4 * theme->padding,
            dropdown->child_pos.h,
            elem->scroll.y,
            elem->content_size.y
        );
    }
}

// GUI element event call.
pgui_resp_t pgui_event_dropdown(
    pax_vec2i gfx_size, pax_vec2i pos, pgui_elem_t *elem, pgui_theme_t const *theme, uint32_t flags, pgui_event_t event
) {
    pgui_dropdown_t *dropdown = (pgui_dropdown_t *)elem;
    if (flags & PGUI_FLAG_ACTIVE) {
        if (event.input == PGUI_INPUT_ACCEPT) {
            if (event.type == PGUI_EVENT_TYPE_RELEASE) {
                // Select an item and close the dropdown.
                dropdown->selected  = elem->selected;
                elem->flags        &= ~(PGUI_FLAG_ACTIVE | PGUI_FLAG_NOBORDER);
                for (size_t i = 0; i < elem->children_len; i++) {
                    elem->children[i]->flags |= PGUI_FLAG_HIDDEN;
                }
                if (elem->callback) {
                    elem->callback(elem);
                }
                return PGUI_RESP_CAPTURED_DIRTY;
            }
            return PGUI_RESP_CAPTURED;

        } else if (event.input == PGUI_INPUT_BACK) {
            if (event.type == PGUI_EVENT_TYPE_RELEASE) {
                // Close the dropdown.
                elem->flags &= ~(PGUI_FLAG_ACTIVE | PGUI_FLAG_NOBORDER);
                for (size_t i = 0; i < elem->children_len; i++) {
                    elem->children[i]->flags |= PGUI_FLAG_HIDDEN;
                }
                return PGUI_RESP_CAPTURED_DIRTY;
            }
            return PGUI_RESP_CAPTURED;

        } else if (event.input == PGUI_INPUT_NEXT || event.input == PGUI_INPUT_DOWN) {
            // Navigate down.
            if (event.type != PGUI_EVENT_TYPE_RELEASE) {
                elem->flags    |= PGUI_FLAG_DIRTY;
                elem->selected  = (elem->selected + 1) % elem->children_len;
                // Update scroll position.
                elem->scroll.y  = pgui_adjust_scroll(
                    elem->size.y * elem->selected,
                    elem->size.y,
                    4 * theme->padding,
                    dropdown->child_pos.h,
                    elem->scroll.y,
                    elem->content_size.y
                );
            }
            return PGUI_RESP_CAPTURED;

        } else if (event.input == PGUI_INPUT_PREV || event.input == PGUI_INPUT_UP) {
            // Navigate up.
            if (event.type != PGUI_EVENT_TYPE_RELEASE) {
                elem->flags    |= PGUI_FLAG_DIRTY;
                elem->selected  = (elem->selected + elem->children_len - 1) % elem->children_len;
                // Update scroll position.
                elem->scroll.y  = pgui_adjust_scroll(
                    elem->size.y * elem->selected,
                    elem->size.y,
                    4 * theme->padding,
                    dropdown->child_pos.h,
                    elem->scroll.y,
                    elem->content_size.y
                );
            }
            return PGUI_RESP_CAPTURED;

        } else {
            return PGUI_RESP_CAPTURED_ERR;
        }

    } else {
        if (event.input == PGUI_INPUT_ACCEPT) {
            if (event.type == PGUI_EVENT_TYPE_RELEASE) {
                // Open the dropdown.
                elem->flags |= PGUI_FLAG_ACTIVE | PGUI_FLAG_DIRTY;
                if (theme->dropdown_covering_menu) {
                    elem->flags |= PGUI_FLAG_NOBORDER;
                }
                for (size_t i = 0; i < elem->children_len; i++) {
                    elem->children[i]->flags &= ~PGUI_FLAG_HIDDEN;
                }
                elem->selected = dropdown->selected;
            }
            return PGUI_RESP_CAPTURED;
        }
        return PGUI_RESP_IGNORED;
    }
}



pgui_type_t const pgui_type_dropdown_raw = {
    .attr  = PGUI_ATTR_DROPDOWN | PGUI_ATTR_SELECTABLE | PGUI_ATTR_ABSPOS,
    .clip  = pgui_clip_dropdown,
    .draw  = pgui_draw_dropdown,
    .event = pgui_event_dropdown,
    .calc  = pgui_calc_dropdown,
};
