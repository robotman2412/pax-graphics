
// SPDX-License-Identifier: MIT

#include "pax_gui.h"

#include "pax_gui_util.h"
#include "pax_internal.h"

#include <math.h>

static char const TAG[] = "pax-gui";

// Default theme.
pgui_theme_t const pgui_theme_default = {
    // Element styles.
    .bg_col                 = 0xffffffff,
    .fg_col                 = 0xff000000,
    .input_col              = 0xffffffff,
    .active_col             = 0xffe0e0e0,
    .button_col             = 0xffd0d0d0,
    .pressed_col            = 0xff909090,
    .border_col             = 0xff000000,
    .highlight_col          = 0xff00e0e0,
    // Size parameters.
    .min_size               = {100, 30},
    .min_input_size         = {100, 30},
    .min_label_size         = {92, 22},
    .border_thickness       = 1,
    .highlight_thickness    = 2,
    .rounding               = 7,
    .padding                = 4,
    // Text style.
    .font                   = pax_font_saira_regular,
    .font_size              = 18,
    // Dropdown style.
    .dropdown_segmented     = false,
    .dropdown_solid_arrow   = false,
    .dropdown_covering_menu = true,
    // Scrollbar style.
    .scroll_bg_col          = 0x3f000000,
    .scroll_fg_col          = 0x7fffffff,
    .scroll_width           = 6,
    .scroll_min_size        = 12,
    .scroll_offset          = 4,
    .scroll_rounding        = 3,
};



// Recalculate the position of a GUI element and its children.
static void pgui_calc_layout_int(
    pax_vec2i gfx_size, pax_vec2i pos, pgui_elem_t *elem, pgui_theme_t const *theme, uint32_t flags
) {
    flags |= elem->flags;
    if (!theme) {
        theme = &pgui_theme_default;
    }

    // Calculate layout of children.
    uint32_t  child_flags  = flags & PGUI_FLAGS_INHERITABLE;
    pax_vec2i child_offset = pos;
    if (elem->type->attr & PGUI_ATTR_ABSPOS) {
        child_offset = (pax_vec2i){0, 0};
    }
    for (size_t i = 0; i < elem->children_len; i++) {
        if (elem->children[i]) {
            pgui_calc_layout_int(
                gfx_size,
                (pax_vec2i){
                    child_offset.x + elem->children[i]->pos.x - elem->scroll.x,
                    child_offset.y + elem->children[i]->pos.y - elem->scroll.y,
                },
                elem->children[i],
                theme,
                child_flags
            );
        }
    }

    // Clamp minimum size.
    if (!(elem->flags & PGUI_FLAG_ANYSIZE)) {
        pax_vec2i min_size;
        if (elem->type->attr & (PGUI_ATTR_INPUT | PGUI_ATTR_BUTTON | PGUI_ATTR_DROPDOWN)) {
            min_size = theme->min_input_size;
        } else if (elem->type->attr & PGUI_ATTR_TEXT) {
            min_size = theme->min_label_size;
        } else {
            min_size = theme->min_size;
        }
        if (elem->size.x < min_size.x) {
            elem->size.x = min_size.x;
        }
        if (elem->size.y < min_size.y) {
            elem->size.y = min_size.y;
        }
    }

    // Calculate layout of this element.
    if (elem->type->calc) {
        elem->type->calc(gfx_size, pos, elem, theme, flags);
    }
}

// Recalculate the position of a GUI element and its children.
void pgui_calc_layout(pax_vec2i gfx_size, pgui_elem_t *elem, pgui_theme_t const *theme) {
    if (!theme) {
        theme = &pgui_theme_default;
    }
    pgui_calc_layout_int(gfx_size, (pax_vec2i){0, 0}, elem, theme, 0);
}



// Internal GUI drawing function.
static void pgui_draw_int(pax_buf_t *gfx, pax_vec2i pos, pgui_elem_t *elem, pgui_theme_t const *theme, uint32_t flags) {
    flags |= elem->flags;
    if (flags & PGUI_FLAG_HIDDEN) {
        // Don't draw hidden elements.
        return;
    }

    // Draw the base of the element.
    pax_recti clip = pax_get_clip(gfx);
    if (flags & PGUI_FLAG_DIRTY) {
        pgui_drawutil_base(gfx, pos, elem->size, elem, theme, flags);
        if (elem->type->draw) {
            elem->type->draw(gfx, pos, elem, theme, flags);
        }
        pgui_drawutil_border(gfx, pos, elem->size, elem, theme, flags);
    }

    // Apply clip rectangle to children.
    if (elem->type->clip) {
        // Element has a custom clip rectangle function.
        elem->type->clip(gfx, pos, elem, theme, flags);
    } else {
        // Apply default child clip rectangle.
        pax_recti bounds = {pos.x - 1, pos.y - 1, elem->size.x + 2, elem->size.y + 2};
        if (!(elem->flags & PGUI_FLAG_NOPADDING)) {
            bounds = pgui_add_padding(bounds, theme->padding);
        }
        pax_set_clip(gfx, pax_recti_intersect(clip, bounds));
    }

    // Draw children.
    uint32_t  child_flags  = flags & PGUI_FLAGS_INHERITABLE;
    pax_vec2i child_offset = pos;
    if (elem->type->attr & PGUI_ATTR_ABSPOS) {
        child_offset = (pax_vec2i){0, 0};
    }
    for (size_t i = 0; i < elem->children_len; i++) {
        if (i != elem->selected && elem->children[i]) {
            pgui_draw_int(
                gfx,
                (pax_vec2i){
                    child_offset.x + elem->children[i]->pos.x - elem->scroll.x,
                    child_offset.y + elem->children[i]->pos.y - elem->scroll.y,
                },
                elem->children[i],
                theme,
                child_flags
            );
        }
    }
    // Draw selected last so it appears on top.
    if (elem->selected >= 0 && elem->selected < elem->children_len && elem->children[elem->selected]) {
        pgui_draw_int(
            gfx,
            (pax_vec2i){
                child_offset.x + elem->children[elem->selected]->pos.x - elem->scroll.x,
                child_offset.y + elem->children[elem->selected]->pos.y - elem->scroll.y,
            },
            elem->children[elem->selected],
            theme,
            flags
        );
    }
    pax_set_clip(gfx, clip);

    // Clear dirty flag.
    elem->flags &= ~PGUI_FLAG_DIRTY;
}

// Draw a GUI element and its children.
void pgui_draw(pax_buf_t *gfx, pgui_elem_t *elem, pgui_theme_t const *theme) {
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
void pgui_redraw(pax_buf_t *gfx, pgui_elem_t *elem, pgui_theme_t const *theme) {
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
static pgui_resp_t pgui_event_int(
    pax_vec2i gfx_size, pax_vec2i pos, pgui_elem_t *elem, pgui_theme_t const *theme, uint32_t flags, pgui_event_t event
) {
    flags |= elem->flags;

    // Send event to selected child first.
    uint32_t  child_flags  = flags & PGUI_FLAGS_INHERITABLE;
    pax_vec2i child_offset = pos;
    if (elem->type->attr & PGUI_ATTR_ABSPOS) {
        child_offset = (pax_vec2i){0, 0};
    }
    if (elem->selected >= 0 && elem->selected < elem->children_len && elem->children[elem->selected]) {
        pgui_resp_t resp = pgui_event_int(
            gfx_size,
            (pax_vec2i){
                child_offset.x + elem->children[elem->selected]->pos.x + elem->scroll.x,
                child_offset.y + elem->children[elem->selected]->pos.y + elem->scroll.y,
            },
            elem->children[elem->selected],
            theme,
            child_flags,
            event
        );
        if (resp) {
            return resp;
        }
    }

    // Event not captured by children.
    if (elem->type->event) {
        return elem->type->event(gfx_size, pos, elem, theme, flags, event);
    }
    return PGUI_RESP_IGNORED;
}

// Handle a button event.
// Returns whether any element was marked dirty in response.
pgui_resp_t pgui_event(pax_vec2i gfx_size, pgui_elem_t *elem, pgui_theme_t const *theme, pgui_event_t event) {
    if (!theme) {
        theme = &pgui_theme_default;
    }
    pgui_resp_t resp = pgui_event_int(gfx_size, elem->pos, elem, theme, 0, event);
    if (resp == PGUI_RESP_CAPTURED_DIRTY) {
        elem->flags |= PGUI_FLAG_DIRTY;
    }
    return resp;
}
