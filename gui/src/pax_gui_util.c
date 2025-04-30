
// SPDX-License-Identifier: MIT

#include "pax_gui_util.h"

#include "pax_gui_internal.h"

static char const TAG[] = "pax-gui";



// Move cursor left/right and return the UTF-8 character.
typedef struct {
    // New cursor position.
    size_t   cursor;
    // Found UTF-8 character, or 0xFFFD if invalid.
    uint32_t codepoint;
} text_nav_t;

// Move cursor left/right and return the UTF-8 character.
static text_nav_t pgui_text_nav(char const *cstr, size_t cstr_len, size_t cursor, bool go_right) {
    text_nav_t out;
    out.cursor = cursor;
    if (!go_right) {
        out.cursor = pax_utf8_seekprev_l(cstr, cstr_len, cursor);
    }
    pax_utf8_getch_l(cstr + out.cursor, cstr_len - out.cursor, &out.codepoint);
    if (go_right) {
        out.cursor = pax_utf8_seeknext_l(cstr, cstr_len, cursor);
    }
    return out;
}

// Is a space char?
static bool is_space(uint32_t val) {
    return val == 0x0020 || val == 0x00A0;
}

// Is an alphanumeric char?
static bool is_alphanumeric(uint32_t val) {
    if (val == '_') {
        return true;
    }
    if (val >= '0' && val <= '9') {
        return true;
    }
    val |= 0x20;
    return val >= 'a' && val <= 'z';
}

// Is neither space nor alphanumeric?
static bool is_other(uint32_t val) {
    return !is_space(val) && !is_alphanumeric(val);
}

// Logic for CTRL+RIGHT or CTRL+LEFT in editable text.
static size_t pgui_text_ctrl_nav(char const *cstr, size_t cstr_len, size_t cursor, bool merge_space, bool go_right) {
    // Pop first character.
    text_nav_t next = pgui_text_nav(cstr, cstr_len, cursor, go_right);

    while (merge_space && cursor != next.cursor && is_space(next.codepoint)) {
        cursor = next.cursor;
        next   = pgui_text_nav(cstr, cstr_len, cursor, go_right);
    }

    if (is_space(next.codepoint)) {
        // Skipping over space.
        do {
            cursor = next.cursor;
            next   = pgui_text_nav(cstr, cstr_len, cursor, go_right);
        } while (cursor != next.cursor && is_space(next.codepoint));
        return cursor;

    } else if (is_alphanumeric(next.codepoint)) {
        // Skipping over text.
        do {
            cursor = next.cursor;
            next   = pgui_text_nav(cstr, cstr_len, cursor, go_right);
        } while (cursor != next.cursor && is_alphanumeric(next.codepoint));
        return cursor;
    }

    if (merge_space) {
        cursor = next.cursor;
        next   = pgui_text_nav(cstr, cstr_len, cursor, go_right);
    }

    if (merge_space && is_alphanumeric(next.codepoint)) {
        // Skipping over text.
        do {
            cursor = next.cursor;
            next   = pgui_text_nav(cstr, cstr_len, cursor, go_right);
        } while (cursor != next.cursor && is_alphanumeric(next.codepoint));
        return cursor;

    } else {
        // Skipping over special chars.
        do {
            cursor = next.cursor;
            next   = pgui_text_nav(cstr, cstr_len, cursor, go_right);
        } while (cursor != next.cursor && is_other(next.codepoint));
        return cursor;
    }
}

// Logic for CTRL+RIGHT in editable text.
size_t pgui_text_ctrl_right(char const *cstr, size_t cstr_len, size_t cursor, bool merge_space) {
    return pgui_text_ctrl_nav(cstr, cstr_len, cursor, merge_space, true);
}

// Logic for CTRL+LEFT in editable text.
size_t pgui_text_ctrl_left(char const *cstr, size_t cstr_len, size_t cursor, bool merge_space) {
    return pgui_text_ctrl_nav(cstr, cstr_len, cursor, merge_space, false);
}



// Visuals for (editable) text-based elements.
void pgui_drawutil_textbox(
    pax_buf_t *gfx, pax_vec2i pos, pgui_elem_t *elem, pgui_theme_t const *theme, uint32_t flags, bool editable
) {
    pgui_text_t   *text    = (pgui_text_t *)elem;
    pgui_padding_t padding = *pgui_effective_padding(elem, theme);

    // Compute bounds.
    pax_recti bounds = {
        pos.x + padding.left - elem->scroll.x,
        pos.y + padding.top - elem->scroll.y,
        elem->content_size.x,
        elem->content_size.y,
    };
    if (bounds.w < elem->size.x - padding.left - padding.right) {
        bounds.w = elem->size.x - padding.left - padding.right;
    }
    if (bounds.h < elem->size.y - padding.top - padding.bottom) {
        bounds.h = elem->size.y - padding.top - padding.bottom;
    }

    // Draw text.
    pax_recti old_clip = pax_get_clip(gfx);
    pax_recti new_clip = {
        pos.x + padding.left - 1,
        pos.y + padding.top - 1,
        elem->size.x - padding.left - padding.right + 2,
        elem->size.y - padding.top - padding.bottom + 2,
    };
    pax_set_clip(gfx, pax_recti_intersect(old_clip, new_clip));
    ptrdiff_t cursorpos = -1;
    if (editable && (flags & PGUI_FLAG_ACTIVE)) {
        cursorpos = text->cursor;
    }
    pgui_drawutil_text(
        gfx,
        pgui_effective_palette(elem, theme)->fg_col,
        pgui_effective_font(elem, theme),
        pgui_effective_font_size(elem, theme),
        text->shrink_to_fit,
        text->text,
        text->text_len,
        cursorpos,
        bounds,
        text->text_halign,
        text->text_valign
    );
    pax_set_clip(gfx, old_clip);
}



// Draw the base of a box or input element.
void pgui_drawutil_base(
    pax_buf_t *gfx, pax_vec2i pos, pax_vec2i size, pgui_elem_t *elem, pgui_theme_t const *theme, uint32_t flags
) {
    if (flags & PGUI_FLAG_NOBACKGROUND) {
        return;
    }

    // Select background color.
    pax_col_t bg;
    if (!(elem->type->attr & (PGUI_ATTR_INPUT | PGUI_ATTR_BUTTON | PGUI_ATTR_DROPDOWN))
        || (flags & PGUI_FLAG_INACTIVE)) {
        bg = pgui_effective_palette(elem, theme)->bg_col;
    } else if ((elem->flags & PGUI_FLAG_ACTIVE) && (elem->type->attr & PGUI_ATTR_BUTTON)) {
        bg = pgui_effective_palette(elem, theme)->pressed_col;
    } else if ((elem->flags & PGUI_FLAG_ACTIVE) && (elem->type->attr & PGUI_ATTR_INPUT)) {
        bg = pgui_effective_palette(elem, theme)->active_col;
    } else if (elem->type->attr & (PGUI_ATTR_BUTTON | PGUI_ATTR_DROPDOWN)) {
        bg = pgui_effective_palette(elem, theme)->button_col;
    } else {
        bg = pgui_effective_palette(elem, theme)->input_col;
    }

    // Clamp rounding.
    int rounding = 0;
    if (!(flags & PGUI_FLAG_NOROUNDING)) {
        rounding = pgui_effective_dims(elem, theme)->rounding;
        if (rounding > size.x / 2) {
            rounding = size.x / 2;
        }
        if (rounding > size.y / 2) {
            rounding = size.y / 2;
        }
    }

    // Draw the backdrop.
    pax_draw_round_rect(gfx, bg, pos.x, pos.y, size.x, size.y, rounding);
}

// Draw the outline of a box or input element.
void pgui_drawutil_border(
    pax_buf_t *gfx, pax_vec2i pos, pax_vec2i size, pgui_elem_t *elem, pgui_theme_t const *theme, uint32_t flags
) {
    if (flags & PGUI_FLAG_NOBORDER) {
        return;
    }
    pgui_size_prop_t const *dims    = pgui_effective_dims(elem, theme);
    pgui_palette_t const   *palette = pgui_effective_palette(elem, theme);

    // Select border.
    pax_col_t color;
    int       thickness;
    if (flags & PGUI_FLAG_HIGHLIGHT) {
        color     = palette->highlight_col;
        thickness = dims->highlight_thickness;
    } else {
        color     = palette->border_col;
        thickness = dims->border_thickness;
    }

    // Clamp rounding.
    int rounding = 0;
    if (!(flags & PGUI_FLAG_NOROUNDING)) {
        rounding = dims->rounding;
        if (rounding > size.x / 2) {
            rounding = size.x / 2;
        }
        if (rounding > size.y / 2) {
            rounding = size.y / 2;
        }
    }

    // Draw the corners.
    pax_vec2i max = {pos.x + size.x, pos.y + size.y};
    float     a0  = 0;
    float     a1  = M_PI / 2;
    float     a2  = 2 * a1;
    float     a3  = 3 * a1;
    float     a4  = 4 * a1;
    pax_draw_hollow_arc(gfx, color, max.x - rounding, pos.y + rounding, rounding - thickness, rounding, a0, a1);
    pax_draw_hollow_arc(gfx, color, pos.x + rounding, pos.y + rounding, rounding - thickness, rounding, a1, a2);
    pax_draw_hollow_arc(gfx, color, pos.x + rounding, max.y - rounding, rounding - thickness, rounding, a2, a3);
    pax_draw_hollow_arc(gfx, color, max.x - rounding, max.y - rounding, rounding - thickness, rounding, a3, a4);

    // Draw the edges.
    if (size.x > 2 * rounding) {
        pax_draw_rect(gfx, color, pos.x + rounding, pos.y, size.x - 2 * rounding, thickness);
        pax_draw_rect(gfx, color, pos.x + rounding, max.y, size.x - 2 * rounding, -thickness);
    }
    if (size.y > 2 * rounding) {
        pax_draw_rect(gfx, color, pos.x, pos.y + rounding, thickness, size.y - 2 * rounding);
        pax_draw_rect(gfx, color, max.x, pos.y + rounding, -thickness, size.y - 2 * rounding);
    }
}

// PAX GUI text measuring helper.
// Returns the bounds of the cursor.
pax_recti pgui_drawutil_getcursor(
    pax_font_t const *font,
    float             font_size,
    bool              shrink_to_fit,
    char const       *text,
    size_t            text_len,
    ptrdiff_t         cursorpos,
    pax_recti         bounds,
    pax_align_t       halign,
    pax_align_t       valign
) {
    // Measure text size.
    float      scale = font_size;
    pax_2vec2f size  = pax_text_size_adv(font, scale, text, text_len, PAX_ALIGN_BEGIN, PAX_ALIGN_BEGIN, -1);

    // Scale down to fit.
    if (shrink_to_fit) {
        if (size.x0 > bounds.w) {
            // Too wide to fit; scale down.
            float mul  = bounds.w / size.x0;
            scale     *= mul;
            size.x0   *= mul;
            size.y0   *= mul;
        }
        if (size.y0 > bounds.h) {
            // Too tall to fit; scale down.
            float mul  = bounds.h / size.y0;
            scale     *= mul;
            size.x0   *= mul;
            size.y0   *= mul;
        }
    }

    // Find cursor.
    int x;
    switch (halign) {
        default:
        case PAX_ALIGN_BEGIN: x = bounds.x; break;
        case PAX_ALIGN_CENTER: x = bounds.x + bounds.w / 2; break;
        case PAX_ALIGN_END: x = bounds.x + bounds.w; break;
    }
    int y;
    switch (valign) {
        default:
        case PAX_ALIGN_BEGIN: y = bounds.y; break;
        case PAX_ALIGN_CENTER: y = bounds.y + (bounds.h - size.y0) * 0.5f; break;
        case PAX_ALIGN_END: y = bounds.y + bounds.h - size.y0; break;
    }
    pax_2vec2f s = pax_text_size_adv(font, scale, text, text_len, halign, PAX_ALIGN_BEGIN, cursorpos);

    // Transform cursor into on-screen position.
    if (isnan(s.x1) || isnan(s.y1)) {
        return (pax_recti){0, 0, 0, 0};
    } else {
        return (pax_recti){
            x + (int)(s.x1 + 0.5f),
            y + (int)(s.y1),
            1,
            ceilf(scale),
        };
    }
}

// Shrink text to fit bounds.
// Returns the relative bounding box of the cursor.
void pgui_drawutil_text(
    pax_buf_t        *gfx,
    pax_col_t         color,
    pax_font_t const *font,
    float             font_size,
    bool              shrink_to_fit,
    char const       *text,
    size_t            text_len,
    ptrdiff_t         cursorpos,
    pax_recti         bounds,
    pax_align_t       halign,
    pax_align_t       valign
) {
    // Measure text size.
    float      scale = font_size;
    pax_2vec2f size  = pax_text_size_adv(font, scale, text, text_len, PAX_ALIGN_BEGIN, PAX_ALIGN_BEGIN, -1);

    // Scale down to fit.
    if (shrink_to_fit) {
        if (size.x0 > bounds.w) {
            // Too wide to fit; scale down.
            float mul  = bounds.w / size.x0;
            scale     *= mul;
            size.x0   *= mul;
            size.y0   *= mul;
        }
        if (size.y0 > bounds.h) {
            // Too tall to fit; scale down.
            float mul  = bounds.h / size.y0;
            scale     *= mul;
            size.x0   *= mul;
            size.y0   *= mul;
        }
    }

    // Draw the label.
    int x;
    switch (halign) {
        default:
        case PAX_ALIGN_BEGIN: x = bounds.x; break;
        case PAX_ALIGN_CENTER: x = bounds.x + bounds.w / 2; break;
        case PAX_ALIGN_END: x = bounds.x + bounds.w; break;
    }
    int y;
    switch (valign) {
        default:
        case PAX_ALIGN_BEGIN: y = bounds.y; break;
        case PAX_ALIGN_CENTER: y = bounds.y + (bounds.h - size.y0) * 0.5f; break;
        case PAX_ALIGN_END: y = bounds.y + bounds.h - size.y0; break;
    }
    pax_draw_text_adv(gfx, color, font, scale, x, y, text, text_len, halign, PAX_ALIGN_BEGIN, cursorpos);
}

// Draw a scrollbar.
void pgui_drawutil_scrollbar(
    pax_buf_t          *gfx,
    pax_vec2i           pos,
    pax_vec2i           size,
    pgui_theme_t const *theme,
    int                 value,
    int                 window,
    int                 total,
    bool                horizontal
) {
    pgui_scroll_prop_t const *prop = &theme->scroll;

    // Calculate scroller size.
    int total_height    = size.y - 2 * prop->offset;
    int scroller_height = window / total * total_height;
    scroller_height     = scroller_height < prop->min_size ? prop->min_size : scroller_height;
    // Scroller offset multiplier.
    int off_mul         = total_height - scroller_height;
    int off_div         = total - window;

    if (horizontal) {
        // Scrollbar background.
        pax_draw_round_rect(
            gfx,
            prop->bg_col,
            pos.x + prop->offset,
            pos.y + size.y - prop->offset,
            size.x - 2 * prop->offset,
            -prop->width,
            prop->rounding
        );
        // Scrollbar foreground.
        pax_draw_round_rect(
            gfx,
            prop->fg_col,
            pos.x + prop->offset + value * off_mul / off_div,
            pos.y + size.y - prop->offset,
            scroller_height,
            -prop->width,
            prop->rounding
        );
    } else {
        // Scrollbar background.
        pax_draw_round_rect(
            gfx,
            prop->bg_col,
            pos.x + size.x - prop->offset,
            pos.y + prop->offset,
            -prop->width,
            size.y - 2 * prop->offset,
            prop->rounding
        );
        // Scrollbar foreground.
        pax_draw_round_rect(
            gfx,
            prop->fg_col,
            pos.x + size.x - prop->offset,
            pos.y + prop->offset + value * off_mul / off_div,
            -prop->width,
            scroller_height,
            prop->rounding
        );
    }
}

// Adjust a scrollbar to show as much of the desired area as possible.
int pgui_adjust_scroll(
    int focus_offset, int focus_size, int viewport_margin, int viewport_window, int scroll, int content_size
) {
    if (content_size <= viewport_window) {
        // The entire thing fits; scroll to the beginning.
        return 0;

    } else if (viewport_window < focus_size + 2 * viewport_margin) {
        // It doesn't fit; scroll to the halfway point.
        scroll = focus_offset - (viewport_window - focus_size) / 2;

    } else if (scroll > focus_offset - viewport_margin) {
        // Scrolled too far up.
        scroll = focus_offset - viewport_margin;

    } else if (scroll < focus_offset + focus_size + viewport_margin - viewport_window) {
        // Scrolled too far down.
        scroll = focus_offset + focus_size + viewport_margin - viewport_window;
    }

    // Clamp to visible area.
    scroll = scroll < 0 ? 0 : scroll;
    scroll = scroll > content_size - viewport_window ? content_size - viewport_window : scroll;
    return scroll;
}

// Adjust a 2D scrollbar to show as much of the desired area as possible.
pax_vec2i pgui_adjust_scroll_2d(
    pax_recti focussed_object, int viewport_margin, pax_vec2i viewport_size, pax_vec2i scroll, pax_vec2i content_size
) {
    return (pax_vec2i){
        pgui_adjust_scroll(
            focussed_object.x,
            focussed_object.w,
            viewport_margin,
            viewport_size.x,
            scroll.x,
            content_size.x
        ),
        pgui_adjust_scroll(
            focussed_object.y,
            focussed_object.h,
            viewport_margin,
            viewport_size.y,
            scroll.y,
            content_size.y
        ),
    };
}
