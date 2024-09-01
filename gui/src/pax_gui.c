
// SPDX-License-Identifier: MIT

#include "pax_gui.h"

#include "pax_gui_internal.h"
#include "pax_gui_util.h"
#include "pax_internal.h"

#include <math.h>

static char const TAG[] = "pax-gui";



/* ==== GUI rendering functions ==== */

// Recalculate the position of a GUI element and its children 1/2.
static void
    pgui_calc1_int(pax_vec2i gfx_size, pax_vec2i pos, pgui_elem_t *elem, pgui_theme_t const *theme, uint32_t flags) {
    flags |= elem->flags;

    // Calculate layout of children.
    uint32_t  child_flags  = flags & PGUI_FLAGS_INHERITABLE;
    pax_vec2i child_offset = pos;
    if (elem->type->attr & PGUI_ATTR_ABSPOS) {
        child_offset = (pax_vec2i){0, 0};
    }
    for (size_t i = 0; i < elem->children_len; i++) {
        if (elem->children[i]) {
            pgui_calc1_int(
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

    // Calculate layout of this element.
    // This accounts for the minimum size of children but does not modify them.
    if (elem->type->calc1) {
        elem->type->calc1(gfx_size, pos, elem, theme, flags);
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
        if (elem->size.x < min_size.x && !(flags & PGUI_FLAG_FIX_WIDTH)) {
            elem->size.x = min_size.x;
        }
        if (elem->size.y < min_size.y && !(flags & PGUI_FLAG_FIX_HEIGHT)) {
            elem->size.y = min_size.y;
        }
    }
}

// Recalculate the position of a GUI element and its children 2/2.
static void
    pgui_calc2_int(pax_vec2i gfx_size, pax_vec2i pos, pgui_elem_t *elem, pgui_theme_t const *theme, uint32_t flags) {
    flags |= elem->flags;

    // Calculate layout of this element.
    // This may move and grow the size of children.
    if (elem->type->calc2) {
        elem->type->calc2(gfx_size, pos, elem, theme, flags);
    }

    // Calculate layout of children.
    uint32_t  child_flags  = flags & PGUI_FLAGS_INHERITABLE;
    pax_vec2i child_offset = pos;
    if (elem->type->attr & PGUI_ATTR_ABSPOS) {
        child_offset = (pax_vec2i){0, 0};
    }
    for (size_t i = 0; i < elem->children_len; i++) {
        if (elem->children[i]) {
            pgui_calc2_int(
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
}

// Recalculate the position of a GUI element and its children.
void pgui_calc_layout(pax_vec2i gfx_size, pgui_elem_t *elem, pgui_theme_t const *theme) {
    if (!theme) {
        theme = pgui_get_default_theme();
    }
    pgui_calc1_int(gfx_size, (pax_vec2i){0, 0}, elem, theme, 0);
    pgui_calc2_int(gfx_size, (pax_vec2i){0, 0}, elem, theme, 0);
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
            child_flags
        );
    }
    pax_set_clip(gfx, clip);

    // Clear dirty flag.
    elem->flags &= ~PGUI_FLAG_DIRTY;
}

// Draw a GUI element and its children.
void pgui_draw(pax_buf_t *gfx, pgui_elem_t *elem, pgui_theme_t const *theme) {
    if (!theme) {
        theme = pgui_get_default_theme();
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
        theme = pgui_get_default_theme();
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
        theme = pgui_get_default_theme();
    }
    pgui_resp_t resp = pgui_event_int(gfx_size, elem->pos, elem, theme, 0, event);
    if (resp == PGUI_RESP_CAPTURED_DIRTY) {
        elem->flags |= PGUI_FLAG_DIRTY;
    }
    return resp;
}



/* ==== Element management functions ==== */

// Delete an element.
void pgui_delete(pgui_elem_t *elem) {
    if (!elem)
        return;
    if (elem->type->del)
        elem->type->del(elem);
    free(elem);
}

// Delete an element and all its children recursively.
void pgui_delete_recursive(pgui_elem_t *elem) {
    if (!elem)
        return;
    for (size_t i = 0; i < elem->children_len; i++) {
        pgui_delete_recursive(elem->children[i]);
    }
    pgui_delete(elem);
}


// Set element custom user data.
void pgui_set_userdata(pgui_elem_t *elem, void *userdata) {
    if (!elem)
        return;
    elem->userdata = userdata;
}

// Get element custom user data.
void *pgui_get_userdata(pgui_elem_t *elem) {
    if (!elem)
        return NULL;
    return elem->userdata;
}

// Set element on change / on press callback.
void pgui_set_callback(pgui_elem_t *elem, pgui_callback_t cb) {
    if (!elem)
        return;
    elem->callback = cb;
}

// Get element on change / on press callback.
pgui_callback_t pgui_get_callback(pgui_elem_t *elem) {
    if (!elem)
        return NULL;
    return elem->callback;
}


// Change the text of a button, label or textbox.
void pgui_set_text(pgui_elem_t *elem, char const *new_label) {
    if (!elem || !(elem->type->attr & PGUI_ATTR_TEXTSTRUCT))
        return;
    pgui_text_t *text = (pgui_text_t *)elem;
    if (text->allow_realloc) {
        free(text->text);
        text->text_cap = strlen(new_label);
        text->text_len = text->text_cap;
        text->text     = strdup(new_label);
    } else {
        text->text     = (char *)new_label;
        text->text_len = strlen(new_label);
    }
}

// Get the txt of a button, label or textbox.
// Take care not to edit in the textbox while still using this value.
char const *pgui_get_text(pgui_elem_t *elem) {
    if (!elem || !(elem->type->attr & PGUI_ATTR_TEXTSTRUCT))
        return NULL;
    pgui_text_t *text = (pgui_text_t *)elem;
    return text->text;
}

// Set the horizontal alignment of a button, label or textbox.
void pgui_set_halign(pgui_elem_t *elem, pax_align_t align) {
    if (!elem || !(elem->type->attr & PGUI_ATTR_TEXTSTRUCT))
        return;
    pgui_text_t *text = (pgui_text_t *)elem;
    text->text_halign = align;
}

// Get the horizontal alignment of a button, label or textbox.
pax_align_t pgui_get_halign(pgui_elem_t *elem) {
    if (!elem || !(elem->type->attr & PGUI_ATTR_TEXTSTRUCT))
        return -1;
    pgui_text_t *text = (pgui_text_t *)elem;
    return text->text_halign;
}

// Set the vertical alignment of a button, label or textbox.
void pgui_set_valign(pgui_elem_t *elem, pax_align_t align) {
    if (!elem || !(elem->type->attr & PGUI_ATTR_TEXTSTRUCT))
        return;
    pgui_text_t *text = (pgui_text_t *)elem;
    text->text_valign = align;
}

// Get the vertical alignment of a button, label or textbox.
pax_align_t pgui_get_valign(pgui_elem_t *elem) {
    if (!elem || !(elem->type->attr & PGUI_ATTR_TEXTSTRUCT))
        return -1;
    pgui_text_t *text = (pgui_text_t *)elem;
    return text->text_valign;
}

// Change the selection index of a grid or dropdown.
// Negative values indicate no selection and aren't applicable to dropdowns.
void pgui_set_selection(pgui_elem_t *elem, ptrdiff_t selection) {
    if (!elem)
        return;
    if (selection < 0)
        elem->selected = -1;
    else if (selection >= elem->children_len)
        elem->selected = elem->children_len - 1;
    elem->selected = selection;
}

// Get the selection index of a grid or dropdown.
// Negative values indicate no selection and aren't applicable to dropdowns.
ptrdiff_t pgui_get_selection(pgui_elem_t *elem) {
    if (!elem)
        return -1;
    return elem->selected;
}

// Printf with indentation.
#define pgui_di_printf(...)                                                                                            \
    do {                                                                                                               \
        int tmp = depth;                                                                                               \
        while (tmp--) {                                                                                                \
            fputs("  ", stdout);                                                                                       \
        }                                                                                                              \
        printf(__VA_ARGS__);                                                                                           \
    } while (0)

// Print GUI debug information.
static void debug_info_impl(pgui_elem_t *elem, int depth, bool selected) {
    if (elem) {
        pgui_di_printf("Element @ %p", elem);
    } else {
        pgui_di_printf("(null)");
    }
    if (selected) {
        fputs(" [selected]", stdout);
    }
    fputc('\n', stdout);
    if (!elem) {
        return;
    }
    depth++;
    pgui_di_printf("Type:         %s\n", elem->type->name);
    pgui_di_printf("Flags:        0x%08" PRIx32 "\n", elem->flags);
    pgui_di_printf("Pos:          {%d, %d}\n", elem->pos.x, elem->pos.y);
    pgui_di_printf("Size:         {%d, %d}\n", elem->size.x, elem->size.y);
    pgui_di_printf("Content size: {%d, %d}\n", elem->content_size.x, elem->content_size.y);
    pgui_di_printf("Scroll:       {%d, %d}\n", elem->scroll.x, elem->scroll.y);

    if (elem->type->attr & PGUI_ATTR_TEXTSTRUCT) {
        pgui_text_t *text = (pgui_text_t *)elem;
        pgui_di_printf("Text:         ");
        if (!text->text) {
            puts("(null)");
        } else if (!*text->text) {
            puts("(empty)");
        } else {
            printf("'%s'\n", text->text);
        }
    } else if (elem->type == &pgui_type_grid) {
        pgui_grid_t *grid = (pgui_grid_t *)elem;
        pgui_di_printf("Col widths:  ");
        for (int i = 0; i < grid->cells.x; i++) {
            printf(" %3d", grid->col_width[i]);
        }
        fputc('\n', stdout);
        pgui_di_printf("Row heights: ");
        for (int i = 0; i < grid->cells.y; i++) {
            printf(" %3d", grid->row_height[i]);
        }
        fputc('\n', stdout);
    }
}

// Print GUI debug information for element and all children.
static void debug_recurse_impl(pgui_elem_t *elem, int depth, bool selected) {
    debug_info_impl(elem, depth++, selected);
    if (elem->children_len) {
        pgui_di_printf("%zu children:\n", elem->children_len);
        for (size_t i = 0; i < elem->children_len; i++) {
            debug_recurse_impl(elem->children[i], depth, elem->selected == i);
        }
    }
}

// Print GUI debug information.
void pgui_print_debug_info(pgui_elem_t *elem) {
    debug_info_impl(elem, 0, false);
}

// Print GUI debug information for element and all children.
void pgui_print_debug_info_recursive(pgui_elem_t *elem) {
    debug_recurse_impl(elem, 0, false);
}



/* ==== GUI composition functions ==== */

// Append a child to a container element.
bool pgui_child_append(pgui_elem_t *parent, pgui_elem_t *child) {
    return parent && pgui_child_insert(parent, parent->children_len, child);
}

// Insert a child element at a specific index, shifting siblings after it.
bool pgui_child_insert(pgui_elem_t *parent, ptrdiff_t index, pgui_elem_t *child) {
    if (!parent || !child)
        return false;
    if (!(parent->type->attr & PGUI_ATTR_CONTAINER))
        return false;
    if (index < 0 || index > parent->children_len)
        return false;
    void *mem = realloc(parent->children, sizeof(void *) * (parent->children_len + 1));
    if (!mem)
        return false;
    parent->children = mem;
    memmove(parent->children + index + 1, parent->children + index, parent->children_len - index);
    parent->children[index] = child;
    parent->children_len++;
    if (parent->type->child) {
        parent->type->child(parent);
    }
    return true;
}

// Insert a child element at a specific index, replacing the element in that place.
pgui_elem_t *pgui_child_replace(pgui_elem_t *parent, ptrdiff_t index, pgui_elem_t *child) {
    if (!parent)
        return NULL;
    if (index < 0 || index >= parent->children_len)
        return NULL;
    pgui_elem_t *prev       = parent->children[index];
    parent->children[index] = child;
    if (parent->type->child) {
        parent->type->child(parent);
    }
    return prev;
}

// Remove a child element by reference.
bool pgui_child_remove_p(pgui_elem_t *parent, pgui_elem_t *child) {
    if (!parent || !child)
        return false;
    for (ptrdiff_t i = 0; i < parent->children_len; i++) {
        if (parent->children[i] == child) {
            pgui_child_remove_i(parent, i);
            return true;
        }
    }
    return false;
}

// Remove a child element by index.
pgui_elem_t *pgui_child_remove_i(pgui_elem_t *parent, ptrdiff_t index) {
    if (!parent)
        return NULL;
    if (index < 0 || index >= parent->children_len)
        return NULL;
    pgui_elem_t *removed = parent->children[index];
    memmove(parent->children + index, parent->children + index + 1, parent->children_len - index - 1);
    parent->children_len--;
    void *mem        = realloc(parent->children, sizeof(void *) * parent->children_len);
    parent->children = mem ?: parent->children;
    if (parent->type->child) {
        parent->type->child(parent);
    }
    return removed;
}

// Get a child element by index.
pgui_elem_t *pgui_child_get(pgui_elem_t *parent, ptrdiff_t index) {
    if (!parent)
        return NULL;
    if (index < 0 || index >= parent->children_len)
        return NULL;
    return parent->children[index];
}


// Set palette variation.
void pgui_set_palette(pgui_elem_t *elem, pgui_variant_t variant) {
    if (!elem)
        return;
    elem->variant = variant;
}

// Get palette variation.
pgui_variant_t pgui_get_palette(pgui_elem_t *elem) {
    if (!elem)
        return PGUI_VARIANT_DEFAULT;
    return elem->variant;
}

// Override element flags.
void pgui_set_flags(pgui_elem_t *elem, uint32_t flags) {
    if (!elem)
        return;
    elem->flags = flags;
}

// Add element flags.
void pgui_enable_flags(pgui_elem_t *elem, uint32_t flags) {
    if (!elem)
        return;
    elem->flags |= flags;
}

// Remove element flags
void pgui_disable_flags(pgui_elem_t *elem, uint32_t flags) {
    if (!elem)
        return;
    elem->flags &= ~flags;
}

// Get element flags.
uint32_t pgui_get_flags(pgui_elem_t *elem) {
    if (!elem)
        return 0;
    return elem->flags;
}

// Override element size.
void pgui_set_size(pgui_elem_t *elem, pax_vec2i size) {
    if (!elem)
        return;
    elem->size = size;
}

// Get element size.
pax_vec2i pgui_get_size(pgui_elem_t *elem) {
    if (!elem)
        return (pax_vec2i){0, 0};
    return elem->size;
}

// Override element position.
void pgui_set_pos(pgui_elem_t *elem, pax_vec2i position) {
    if (!elem)
        return;
    elem->pos = position;
}

// Get element position.
pax_vec2i pgui_get_pos(pgui_elem_t *elem) {
    if (!elem)
        return (pax_vec2i){0, 0};
    return elem->pos;
}
