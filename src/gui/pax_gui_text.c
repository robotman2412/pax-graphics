
// SPDX-License-Identifier: MIT

#include "pax_gui_text.h"

#include "pax_internal.h"



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

// Text element type.
pgui_type_t pgui_type_text_raw = {
    .draw = (pgui_draw_fn_t)pgui_draw_text,
};

// Label element type.
pgui_type_t pgui_type_label_raw = {
    .draw = (pgui_draw_fn_t)pgui_draw_label,
};
