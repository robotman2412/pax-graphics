
// SPDX-License-Identifier: MIT

#include "pax_gui_internal.h"
#include "pax_gui_util.h"



// Create a new label.
pgui_elem_t *pgui_new_text(char const *text) {
    pgui_text_t *elem = malloc(sizeof(pgui_text_t));
    if (!elem)
        return NULL;
    memset(elem, 0, sizeof(pgui_text_t));
    elem->base.type     = &pgui_type_text;
    elem->base.flags    = PGUI_FLAG_NOBACKGROUND | PGUI_FLAG_NOBORDER;
    elem->base.selected = -1;
    elem->text          = (char *)text;
    elem->text_len      = strlen(text);
    elem->text_halign   = PAX_ALIGN_CENTER;
    elem->text_valign   = PAX_ALIGN_CENTER;
    return (pgui_elem_t *)elem;
}

// Visuals for text-based elements.
void pgui_draw_text(pax_buf_t *gfx, pax_vec2i pos, pgui_elem_t *elem, pgui_theme_t const *theme, uint32_t flags) {
    pgui_drawutil_textbox(gfx, pos, elem, theme, flags, false);
}

// Calculate the minimum size of text-based elements.
void pgui_calc1_text(pax_vec2i gfx_size, pax_vec2i pos, pgui_elem_t *elem, pgui_theme_t const *theme, uint32_t flags) {
    pgui_text_t *text = (pgui_text_t *)elem;
    if (text->shrink_to_fit)
        return;

    pax_2vec2f text_size = pax_text_size_adv(
        theme->font,
        theme->font_size,
        text->text,
        text->text_len,
        PAX_ALIGN_BEGIN,
        PAX_ALIGN_BEGIN,
        -1
    );
    int min_w = ceilf(text_size.x0);
    int min_h = ceilf(text_size.y0);

    if (!(flags & PGUI_FLAG_NOPADDING)) {
        min_w += 2 * theme->padding;
        min_h += 2 * theme->padding;
    }
    if (!(flags & PGUI_FLAG_FIX_WIDTH)) {
        elem->size.x = min_w;
    }
    if (!(flags & PGUI_FLAG_FIX_HEIGHT)) {
        elem->size.y = min_h;
    }
}

// Additional delete function for text-based elements.
void pgui_del_text(pgui_elem_t *elem) {
    pgui_text_t *text = (pgui_text_t *)elem;
    if (text->allow_realloc) {
        free(text->text);
    }
}

// Text element type.
pgui_type_t const pgui_type_text = {
    .name  = "text",
    .attr  = PGUI_ATTR_TEXT | PGUI_ATTR_TEXTSTRUCT,
    .draw  = pgui_draw_text,
    .calc1 = pgui_calc1_text,
    .del   = pgui_del_text,
};
