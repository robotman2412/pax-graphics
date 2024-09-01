
// SPDX-License-Identifier: MIT

#include "pax_gui_internal.h"
#include "pax_gui_util.h"



// Create a new image.
pgui_elem_t *pgui_new_image(pax_buf_t *image, bool do_free_image) {
    pgui_image_t *elem = malloc(sizeof(pgui_image_t));
    if (!elem)
        return NULL;
    memset(elem, 0, sizeof(pgui_image_t));
    elem->base.type  = &pgui_type_image;
    elem->base.flags = PGUI_FLAG_NOBACKGROUND | PGUI_FLAG_NOBORDER | PGUI_FLAG_NOPADDING | PGUI_FLAG_FIX_WIDTH
                       | PGUI_FLAG_FIX_HEIGHT;
    elem->base.selected = -1;
    elem->base.size     = pax_buf_get_dims(image);
    elem->image         = image;
    elem->do_free_image = do_free_image;
    return (pgui_elem_t *)elem;
}

// Visuals for image elements.
void pgui_draw_image(pax_buf_t *gfx, pax_vec2i pos, pgui_elem_t *elem, pgui_theme_t const *theme, uint32_t flags) {
    pgui_image_t *image   = (pgui_image_t *)elem;
    int           padding = flags & PGUI_FLAG_NOPADDING ? 0 : 2 * theme->padding;
    pax_draw_image_sized(gfx, image->image, pos.x, pos.y, elem->size.x - padding, elem->size.y - padding);
}

// Calculate the minimum size of image elements.
void pgui_calc1_image(pax_vec2i gfx_size, pax_vec2i pos, pgui_elem_t *elem, pgui_theme_t const *theme, uint32_t flags) {
    pgui_image_t *image = (pgui_image_t *)elem;

    int min_w = pax_buf_get_width(image->image);
    int min_h = pax_buf_get_height(image->image);

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

// Additional delete function for image elements.
void pgui_del_image(pgui_elem_t *elem) {
    pgui_image_t *image = (pgui_image_t *)elem;
    if (image->do_free_image) {
        pax_buf_destroy(image->image);
    }
}

// Text element type.
pgui_type_t const pgui_type_image = {
    .id    = PGUI_TYPE_ID_IMAGE,
    .name  = "image",
    .attr  = 0,
    .draw  = pgui_draw_image,
    .calc1 = pgui_calc1_image,
    .del   = pgui_del_image,
};
