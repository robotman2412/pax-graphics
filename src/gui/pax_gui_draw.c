
// SPDX-License-Identifier: MIT

#include "pax_gui.h"
#include "pax_internal.h"

static char const TAG[] = "pax-gui";



// Draw the base of a box or input element.
static void
    pgui_draw_base(pax_buf_t *gfx, pax_vec2f pos, pgui_base_t *elem, pgui_theme_t const *theme, uint32_t flags) {
    if (flags & PGUI_FLAG_NOBACKGROUND) {
        return;
    }

    // Select border color.
    pax_col_t border;
    if (flags & PGUI_FLAG_HIGHLIGHT) {
        border = theme->highlight_col;
    } else {
        border = theme->border_col;
    }

    // Select background color.
    pax_col_t bg;
    if (PGUI_IS_BOX(elem->type) || (flags & PGUI_FLAG_INACTIVE)) {
        bg = theme->bg_col;
    } else if (elem->flags & PGUI_FLAG_ACTIVE) {
        bg = theme->pressed_col;
    } else {
        bg = theme->input_col;
    }

    // Draw the backdrop.
    pax_draw_round_rect(gfx, bg, pos.x, pos.y, elem->size.x, elem->size.y, theme->rounding);
    pax_outline_round_rect(gfx, border, pos.x, pos.y, elem->size.x, elem->size.y, theme->rounding);
}

// Shrink text to fit bounds.
static void pgui_draw_bounded_text(
    pax_buf_t        *gfx,
    pax_col_t         color,
    pax_font_t const *font,
    float             font_size,
    char const       *text,
    pax_rectf         bounds,
    pax_text_align_t  align
) {
    // Measure text size.
    float     scale = font_size;
    pax_vec2f size  = pax_text_size(font, scale, text);

    // Scale down to fit.
    if (size.x > bounds.w) {
        // Too wide to fit; scale down.
        float mul  = bounds.w / size.x;
        scale     *= mul;
        size.x    *= mul;
        size.y    *= mul;
    }
    if (size.y > bounds.h) {
        // Too tall to fit; scale down.
        float mul  = bounds.h / size.y;
        scale     *= mul;
        size.x    *= mul;
        size.y    *= mul;
    }

    // Draw the label.
    switch (align) {
        default:
        case PAX_ALIGN_LEFT:
            pax_draw_text(gfx, color, font, scale, bounds.x, bounds.y + (bounds.h - size.y) / 2, text);
            break;
        case PAX_ALIGN_CENTER:
            pax_center_text(gfx, color, font, scale, bounds.x + bounds.w / 2, bounds.y + (bounds.h - size.y) / 2, text);
            break;
        case PAX_ALIGN_RIGHT:
            pax_right_text(gfx, color, font, scale, bounds.x + bounds.w, bounds.y + (bounds.h - size.y) / 2, text);
            break;
    }
}

// Draw a scrollbar.
static void pgui_draw_scrollbar(
    pax_buf_t *gfx, pax_vec2f pos, pax_vec2f size, pgui_theme_t const *theme, float value, float window, float total
) {
    // Calculate scroller size.
    float total_height    = size.y - 2 * theme->scroll_offset;
    float scroller_height = window / total * total_height;
    scroller_height       = fmaxf(scroller_height, theme->scroll_min_size);
    // Scroller offset multiplier.
    float off_mul         = (total_height - scroller_height) / (total - window);

    // Scrollbar background.
    pax_draw_round_rect(
        gfx,
        theme->scroll_bg_col,
        pos.x + size.x - theme->scroll_offset,
        pos.y + theme->scroll_offset,
        -theme->scroll_width,
        size.y - 2 * theme->scroll_offset,
        theme->scroll_rounding
    );
    // Scrollbar foreground.
    pax_draw_round_rect(
        gfx,
        theme->scroll_fg_col,
        pos.x + size.x - theme->scroll_offset,
        pos.y + theme->scroll_offset + off_mul * value,
        -theme->scroll_width,
        scroller_height,
        theme->scroll_rounding
    );
}


// Draw a box.
static void pgui_draw_box(pax_buf_t *gfx, pax_vec2f pos, pgui_box_t *elem, pgui_theme_t const *theme, uint32_t flags) {
    pgui_draw_base(gfx, pos, &elem->base, theme, flags);
}

// Draw a grid.
static void
    pgui_draw_grid(pax_buf_t *gfx, pax_vec2f pos, pgui_grid_t *elem, pgui_theme_t const *theme, uint32_t flags) {
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

// Draw a button.
static void
    pgui_draw_button(pax_buf_t *gfx, pax_vec2f pos, pgui_button_t *elem, pgui_theme_t const *theme, uint32_t flags) {
    // Determine colors.
    pax_col_t fg     = theme->fg_col;
    pax_col_t bg     = (flags & PGUI_FLAG_INACTIVE) ? theme->bg_col
                       : (flags & PGUI_FLAG_ACTIVE) ? theme->pressed_col
                                                    : theme->input_col;
    pax_col_t border = (flags & PGUI_FLAG_HIGHLIGHT) ? theme->highlight_col : theme->border_col;
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
        float margin = elem->base.size.y;
        if (view_height < 2 * margin + elem->base.size.y) {
            // Too little area for proper scrolling; center around element.
            elem->scroll  = elem->to_select * elem->base.size.y - margin;
            elem->scroll -= (view_height - elem->base.size.y) / 2 - margin;

        } else if (elem->to_select == 0) {
            // First is selected; scroll always at the top.
            elem->scroll = 0;

        } else if (elem->to_select == elem->options_len - 1) {
            // Last is selected; scroll always at the bottom.
            elem->scroll = total_height - view_height;

        } else if (elem->scroll > elem->to_select * elem->base.size.y - margin) {
            // Scrolled too far up.
            elem->scroll = elem->to_select * elem->base.size.y - margin;

        } else if (elem->scroll < (elem->to_select + 1) * elem->base.size.y + margin - view_height) {
            // Scrolled too far down.
            elem->scroll = (elem->to_select + 1) * elem->base.size.y + margin - view_height;
        }
        // Clamp scroll.
        elem->scroll = fmaxf(0, elem->scroll);
        elem->scroll = fminf(total_height - view_height, elem->scroll);

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

// Draw a text paragraph.
static void
    pgui_draw_text(pax_buf_t *gfx, pax_vec2f pos, pgui_text_t *elem, pgui_theme_t const *theme, uint32_t flags) {
    switch (elem->align) {
        default:
        case PAX_ALIGN_LEFT:
            pax_draw_text(
                gfx,
                theme->fg_col,
                theme->font,
                theme->font_size,
                pos.x + theme->text_padding,
                pos.y + theme->text_padding,
                elem->text
            );
            break;
        case PAX_ALIGN_CENTER:
            pax_center_text(
                gfx,
                theme->fg_col,
                theme->font,
                theme->font_size,
                pos.x + elem->base.size.x / 2,
                pos.y + theme->text_padding,
                elem->text
            );
            break;
        case PAX_ALIGN_RIGHT:
            pax_right_text(
                gfx,
                theme->fg_col,
                theme->font,
                theme->font_size,
                pos.x + elem->base.size.x - theme->text_padding,
                pos.y + theme->text_padding,
                elem->text
            );
            break;
    }
}

// Draw a text label.
static void
    pgui_draw_label(pax_buf_t *gfx, pax_vec2f pos, pgui_label_t *elem, pgui_theme_t const *theme, uint32_t flags) {
    pgui_draw_bounded_text(
        gfx,
        theme->fg_col,
        theme->font,
        theme->font_size,
        elem->text,
        (pax_rectf){
            pos.x + theme->text_padding,
            pos.y + theme->text_padding,
            elem->base.size.x - 2 * theme->text_padding,
            elem->base.size.y - 2 * theme->text_padding,
        },
        elem->align
    );
}

// Internal GUI drawing function.
static void pgui_draw_int(pax_buf_t *gfx, pax_vec2f pos, pgui_base_t *elem, pgui_theme_t const *theme, uint32_t flags) {
    flags |= elem->flags;
    if (flags & PGUI_FLAG_HIDDEN) {
        // Don't draw hidden elements.
        return;
    }

    if (flags & PGUI_FLAG_DIRTY) {
        switch (elem->type) {
            default:
                PAX_LOGE(TAG, "Cannot draw unknown element type %d", elem->type);
                elem->flags |= PGUI_FLAG_HIDDEN;
                return;
            case PGUI_TYPE_BOX: pgui_draw_box(gfx, pos, (pgui_box_t *)elem, theme, flags); break;
            case PGUI_TYPE_GRID: pgui_draw_grid(gfx, pos, (pgui_grid_t *)elem, theme, flags); break;
            case PGUI_TYPE_BUTTON: pgui_draw_button(gfx, pos, (pgui_button_t *)elem, theme, flags); break;
            case PGUI_TYPE_DROPDOWN: pgui_draw_dropdown(gfx, pos, (pgui_dropdown_t *)elem, theme, flags); break;
            case PGUI_TYPE_TEXT: pgui_draw_text(gfx, pos, (pgui_text_t *)elem, theme, flags); break;
            case PGUI_TYPE_LABEL: pgui_draw_label(gfx, pos, (pgui_label_t *)elem, theme, flags); break;
        }
    }

    if (PGUI_IS_BOX(elem->type)) {
        // Update flags.
        pgui_box_t *box = (pgui_box_t *)elem;
        flags           = (elem->flags | flags) & PGUI_FLAGS_INHERITABLE;
        // Draw children.
        for (size_t i = 0; i < box->children_len; i++) {
            if (i != box->selected && box->children[i]) {
                pgui_draw_int(
                    gfx,
                    (pax_vec2f){
                        pos.x + theme->box_padding + box->children[i]->pos.x,
                        pos.y + theme->box_padding + box->children[i]->pos.y,
                    },
                    box->children[i],
                    theme,
                    flags
                );
            }
        }
        // Draw selected last so it appears on top.
        if (box->selected >= 0 && box->children[box->selected]) {
            pgui_draw_int(
                gfx,
                (pax_vec2f){
                    pos.x + theme->box_padding + box->children[box->selected]->pos.x,
                    pos.y + theme->box_padding + box->children[box->selected]->pos.y,
                },
                box->children[box->selected],
                theme,
                flags
            );
        }
    }

    // Clear dirty flag.
    elem->flags &= ~PGUI_FLAG_DIRTY;
}

// Draw a GUI element and its children.
void pgui_draw(pax_buf_t *gfx, pgui_base_t *elem, pgui_theme_t const *theme) {
    if (!theme) {
        theme = &pgui_theme_default;
    }
    pax_push_2d(gfx);
    pax_reset_2d(gfx, PAX_RESET_TOP);
    elem->parent = NULL;
    pgui_draw_int(gfx, elem->pos, elem, theme, PGUI_FLAG_DIRTY);
    pax_pop_2d(gfx);
}

// Re-draw dirty parts of the GUI and mark the elements clean.
void pgui_redraw(pax_buf_t *gfx, pgui_base_t *elem, pgui_theme_t const *theme) {
    if (!theme) {
        theme = &pgui_theme_default;
    }
    pax_push_2d(gfx);
    pax_reset_2d(gfx, PAX_RESET_TOP);
    elem->parent = NULL;
    pgui_draw_int(gfx, elem->pos, elem, theme, 0);
    pax_pop_2d(gfx);
}
