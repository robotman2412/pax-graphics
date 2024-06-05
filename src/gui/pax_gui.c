
// SPDX-License-Identifier: MIT

#include "pax_gui.h"

#include "pax_internal.h"

char const TAG[] = "pax-gui";



// Draw the base of a box or input element.
void pgui_draw_base(pax_buf_t *gfx, pax_vec2f pos, pgui_base_t *elem, pgui_theme_t const *theme, uint32_t flags) {
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
void pgui_draw_bounded_text(
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
void pgui_draw_scrollbar(
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
        case PGUI_TYPE_TEXTBOX: break;
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
            case PGUI_TYPE_TEXTBOX: pgui_draw_textbox(gfx, pos, (pgui_textbox_t *)elem, theme, flags); break;
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

// Internal event handler.
static pgui_resp_t pgui_event_int(pgui_base_t *elem, pgui_event_t event, uint32_t flags) {
    flags |= elem->flags;

    // Send event to children first.
    if (PGUI_IS_BOX(elem->type)) {
        pgui_box_t *box = (pgui_box_t *)elem;
        if (box->selected >= 0 && box->children[box->selected]) {
            pgui_resp_t resp = pgui_event_int(box->children[box->selected], event, flags);
            if (resp) {
                return resp;
            }
        }
    }

    // Not a box or not captured by children.
    switch (elem->type) {
        default: PAX_LOGE(TAG, "Cannot send event to unknown element type %d", elem->type); return PGUI_RESP_IGNORED;
        case PGUI_TYPE_BOX: return PGUI_RESP_IGNORED;
        case PGUI_TYPE_GRID: return pgui_event_grid((pgui_grid_t *)elem, event, flags);
        case PGUI_TYPE_BUTTON: return pgui_event_button((pgui_button_t *)elem, event, flags);
        case PGUI_TYPE_DROPDOWN: return pgui_event_dropdown((pgui_dropdown_t *)elem, event, flags);
        case PGUI_TYPE_TEXTBOX: return pgui_event_textbox((pgui_textbox_t *)elem, event, flags);
        case PGUI_TYPE_TEXT: return PGUI_RESP_IGNORED;
        case PGUI_TYPE_LABEL: return PGUI_RESP_IGNORED;
    }
}

// Handle a button event.
// Returns whether any element was marked dirty in response.
pgui_resp_t pgui_event(pgui_base_t *elem, pgui_event_t event) {
    pgui_resp_t resp = pgui_event_int(elem, event, 0);
    if (resp == PGUI_RESP_CAPTURED_DIRTY) {
        elem->flags |= PGUI_FLAG_DIRTY;
    }
    return resp;
}
