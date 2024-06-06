
// SPDX-License-Identifier: MIT

#include "pax_gui_dropdown.h"

#include "pax_internal.h"



// Draw a dropdown element's menu.
static void pgui_draw_dropdown_menu(
    pax_buf_t *gfx, pax_vec2f pos, pgui_dropdown_t *elem, pgui_theme_t const *theme, uint32_t flags
) {
    float width = elem->base.size.x;
    if (theme->dropdown_segmented) {
        width -= elem->base.size.y;
    }

    // Calculate heights.
    float buf_height   = pax_buf_get_height(gfx);
    float total_height = elem->base.size.y * elem->options_len;
    float low_space    = buf_height - pos.y - !theme->dropdown_covering_menu * elem->base.size.y;
    float high_space   = pos.y + theme->dropdown_covering_menu * elem->base.size.y;
    float view_height;

    // If it doesn't fit below and there is more space above than below, put it above.
    if (total_height > low_space && high_space > low_space) {
        // Decided to place above dropdown.
        view_height  = fminf(total_height, high_space);
        pos.y       -= view_height;
        if (theme->dropdown_covering_menu) {
            pos.y += elem->base.size.y;
        }
    } else {
        // Decided to place below dropdown.
        if (!theme->dropdown_covering_menu) {
            pos.y += elem->base.size.y;
        }
        view_height = fminf(total_height, low_space);
    }

    // Draw the selection menu.
    if (total_height <= view_height) {
        // Everything fits; no scrolling required.
        elem->scroll = 0;

        if (elem->to_select) {
            // Background above selection.
            pax_draw_round_rect4(
                gfx,
                theme->pressed_col,
                pos.x,
                pos.y,
                width,
                elem->base.size.y * elem->to_select,
                theme->rounding,
                theme->rounding,
                0,
                0
            );
        }
        // Background at selection.
        pax_draw_round_rect4(
            gfx,
            theme->input_col,
            pos.x,
            pos.y + elem->base.size.y * elem->to_select,
            width,
            elem->base.size.y,
            elem->to_select == 0 ? theme->rounding : 0,
            elem->to_select == 0 ? theme->rounding : 0,
            elem->to_select == elem->options_len - 1 ? theme->rounding : 0,
            elem->to_select == elem->options_len - 1 ? theme->rounding : 0
        );
        if (elem->to_select < elem->options_len - 1) {
            // Background below selection.
            pax_draw_round_rect4(
                gfx,
                theme->pressed_col,
                pos.x,
                pos.y + elem->base.size.y * (elem->to_select + 1),
                width,
                view_height - elem->base.size.y * (elem->to_select + 1),
                0,
                0,
                theme->rounding,
                theme->rounding
            );
        }

        // Draw options.
        for (size_t i = 0; i < elem->options_len; i++) {
            pgui_draw_bounded_text(
                gfx,
                theme->fg_col,
                theme->font,
                theme->font_size,
                elem->options[i],
                (pax_rectf){
                    pos.x + theme->input_padding,
                    pos.y + i * elem->base.size.y + theme->input_padding,
                    width - 2 * theme->input_padding,
                    elem->base.size.y - 2 * theme->input_padding,
                },
                PAX_ALIGN_CENTER
            );
        }
    } else {
        // It doesn't fit; draw with scrolling.
        pax_recti clip = pax_get_clip(gfx);

        // Adjust scroll if necessary.
        elem->scroll = pgui_adjust_scroll(
            elem->to_select * elem->base.size.y,
            elem->base.size.y,
            view_height,
            elem->scroll,
            elem->base.size.y,
            total_height
        );

        // Compute clip rectangles.
        pax_recti sel_clip = {
            pos.x,
            pos.y + elem->to_select * elem->base.size.y - elem->scroll,
            width,
            elem->base.size.y,
        };
        pax_recti pre_clip = {
            pos.x,
            pos.y,
            width,
            sel_clip.y - pos.y,
        };
        pax_recti post_clip = {
            pos.x,
            sel_clip.y + sel_clip.h,
            width,
            view_height + pos.y - sel_clip.y - sel_clip.h,
        };

        // Background before selection.
        if (pre_clip.h > 0) {
            pax_set_clip(gfx, pre_clip);
            pax_draw_round_rect(gfx, theme->pressed_col, pos.x, pos.y, width, view_height, theme->rounding);
        }
        // Background at selection.
        if (sel_clip.y + sel_clip.h > pos.y && sel_clip.y < pos.y + view_height) {
            pax_set_clip(gfx, sel_clip);
            pax_draw_round_rect(gfx, theme->input_col, pos.x, pos.y, width, view_height, theme->rounding);
        }
        // Background after selection.
        if (post_clip.h > 0) {
            pax_set_clip(gfx, post_clip);
            pax_draw_round_rect(gfx, theme->pressed_col, pos.x, pos.y, width, view_height, theme->rounding);
        }

        // Draw options.
        pax_clip(gfx, pos.x, pos.y, width, view_height);
        for (size_t i = 0; i < elem->options_len; i++) {
            // Compute text bounds.
            pax_rectf bounds = {
                pos.x + theme->input_padding,
                pos.y + i * elem->base.size.y + theme->input_padding - elem->scroll,
                width - 2 * theme->input_padding,
                elem->base.size.y - 2 * theme->input_padding,
            };
            // Clip entire strings.
            if (bounds.y >= pos.y + view_height || bounds.y + bounds.h < pos.y) {
                continue;
            }
            // Draw text.
            pgui_draw_bounded_text(
                gfx,
                theme->fg_col,
                theme->font,
                theme->font_size,
                elem->options[i],
                bounds,
                PAX_ALIGN_CENTER
            );
        }

        // Restore clip rect.
        pax_set_clip(gfx, clip);

        // Draw the scrollbar.
        pgui_draw_scrollbar(gfx, pos, (pax_vec2f){width, view_height}, theme, elem->scroll, view_height, total_height);
    }

    // Draw the ouline.
    pax_outline_round_rect(gfx, theme->highlight_col, pos.x, pos.y, width, view_height, theme->rounding);
}

// Draw a dropdown.
static void pgui_draw_dropdown(
    pax_buf_t *gfx, pax_vec2f pos, pgui_dropdown_t *elem, pgui_theme_t const *theme, uint32_t flags
) {
    if (flags & PGUI_FLAG_INACTIVE) {
        // Close dropdown if inactive.
        elem->base.flags &= PGUI_FLAG_ACTIVE;
    }
    if (flags & PGUI_FLAG_INACTIVE) {
        // Close dropdown if inactive.
        elem->base.flags &= PGUI_FLAG_ACTIVE;
    }
    if (theme->dropdown_covering_menu && !theme->dropdown_segmented && (flags & PGUI_FLAG_ACTIVE)) {
        // Menu covers the dropdown, don't render anything else.
        pgui_draw_dropdown_menu(gfx, pos, elem, theme, flags);
        return;
    }

    // Draw backdrop.
    pgui_draw_base(gfx, pos, &elem->base, theme, flags);

    // Draw segment.
    if (theme->dropdown_segmented) {
        pax_draw_line(
            gfx,
            theme->border_col,
            pos.x + elem->base.size.x - elem->base.size.y,
            pos.y + 1,
            pos.x + elem->base.size.x - elem->base.size.y,
            pos.y + elem->base.size.y - 1
        );
    }

    // Draw arrow.
    pax_push_2d(gfx);
    pax_apply_2d(gfx, matrix_2d_translate(pos.x + elem->base.size.x, pos.y));
    pax_apply_2d(gfx, matrix_2d_scale(elem->base.size.y, elem->base.size.y));
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

    if (theme->dropdown_covering_menu && (flags & PGUI_FLAG_ACTIVE)) {
        // Menu covers the text, don't render anything else.
        pgui_draw_dropdown_menu(gfx, pos, elem, theme, flags);
        return;
    }

    // Draw text.
    pgui_draw_bounded_text(
        gfx,
        theme->fg_col,
        theme->font,
        theme->font_size,
        elem->options[elem->selected],
        (pax_rectf){
            pos.x + theme->text_padding,
            pos.y + theme->text_padding,
            elem->base.size.x - elem->base.size.y - 2 * theme->text_padding,
            elem->base.size.y - 2 * theme->text_padding,
        },
        PAX_ALIGN_CENTER
    );

    if (flags & PGUI_FLAG_ACTIVE) {
        // Menu doesn't cover, render after everything else.
        pgui_draw_dropdown_menu(gfx, pos, elem, theme, flags);
        return;
    }
}

// Send an event to a dropdown.
static pgui_resp_t pgui_event_dropdown(pgui_dropdown_t *elem, pgui_event_t event, uint32_t flags) {
    if (flags & PGUI_FLAG_INACTIVE) {
        // Close dropdown if inactive.
        elem->base.flags &= PGUI_FLAG_ACTIVE;
    }
    if (flags & PGUI_FLAG_ACTIVE) {
        // The dropdown is currently open.
        if (event.input == PGUI_INPUT_ACCEPT) {
            if (event.type == PGUI_EVENT_TYPE_RELEASE) {
                // Selection accepted.
                if (elem->callback) {
                    elem->callback(elem, elem->cookie);
                }
                elem->selected    = elem->to_select;
                elem->base.flags &= ~PGUI_FLAG_ACTIVE;
                return PGUI_RESP_CAPTURED_DIRTY;
            } else {
                return PGUI_RESP_CAPTURED;
            }
        } else if (event.input == PGUI_INPUT_BACK) {
            if (event.type == PGUI_EVENT_TYPE_RELEASE) {
                // Selection rejected.
                elem->base.flags &= ~PGUI_FLAG_ACTIVE;
                return PGUI_RESP_CAPTURED_DIRTY;
            } else {
                return PGUI_RESP_CAPTURED;
            }
        } else if (event.input == PGUI_INPUT_UP) {
            // Navigate up.
            if (event.type != PGUI_EVENT_TYPE_RELEASE) {
                elem->to_select   = (elem->to_select + elem->options_len - 1) % elem->options_len;
                elem->base.flags |= PGUI_FLAG_DIRTY;
            }
            return PGUI_RESP_CAPTURED;
        } else if (event.input == PGUI_INPUT_DOWN) {
            // Navigate down.
            if (event.type != PGUI_EVENT_TYPE_RELEASE) {
                elem->to_select   = (elem->to_select + 1) % elem->options_len;
                elem->base.flags |= PGUI_FLAG_DIRTY;
            }
            return PGUI_RESP_CAPTURED;
        } else {
            // Other inputs not accepted.
            return event.type == PGUI_EVENT_TYPE_PRESS ? PGUI_RESP_CAPTURED_ERR : PGUI_RESP_CAPTURED;
        }

    } else {
        // The dropdown is currently closed.
        if (event.input == PGUI_INPUT_ACCEPT) {
            if (event.type == PGUI_EVENT_TYPE_RELEASE) {
                if (flags & PGUI_FLAG_INACTIVE) {
                    return PGUI_RESP_CAPTURED_ERR;
                }
                // Open the drop-down.
                elem->to_select   = elem->selected;
                elem->base.flags |= PGUI_FLAG_DIRTY | PGUI_FLAG_ACTIVE;
            }
            return PGUI_RESP_CAPTURED;
        } else {
            // Other inputs ignored.
            return PGUI_RESP_IGNORED;
        }
    }
}

// Dropdown element type.
pgui_type_t pgui_type_dropdown_raw = {
    .attr  = PGUI_ATTR_SELECTABLE,
    .draw  = (pgui_draw_fn_t)pgui_draw_dropdown,
    .event = (pgui_event_fn_t)pgui_event_dropdown,
};
