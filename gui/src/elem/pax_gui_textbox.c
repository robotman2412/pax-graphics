
// SPDX-License-Identifier: MIT

#include "pax_gui_internal.h"
#include "pax_gui_util.h"

#include <malloc.h>
#include <string.h>



// Create a new editable textbox.
pgui_elem_t *pgui_new_textbox(pgui_callback_t cb) {
    pgui_text_t *elem = malloc(sizeof(pgui_text_t));
    if (!elem)
        return NULL;
    memset(elem, 0, sizeof(pgui_text_t));
    elem->base.type     = &pgui_type_textbox;
    elem->base.callback = cb;
    elem->base.selected = -1;
    elem->allow_realloc = true;
    return (pgui_elem_t *)elem;
}

// Calculate the layout of editable text-based elements.
void pgui_calc2_textbox(
    pax_vec2i gfx_size, pax_vec2i pos, pgui_elem_t *elem, pgui_theme_t const *theme, uint32_t flags
) {
    (void)gfx_size;
    (void)pos;
    (void)flags;
    pgui_text_t   *text    = (pgui_text_t *)elem;
    pgui_padding_t padding = *pgui_effective_padding(elem, theme);

    // Compute content size.
    if (text->shrink_to_fit) {
        elem->content_size = elem->size;
    } else {
        pax_2vec2f s = pax_text_size_adv(
            pgui_effective_font(elem, theme),
            pgui_effective_font_size(elem, theme),
            text->text,
            text->text_len,
            PAX_ALIGN_BEGIN,
            PAX_ALIGN_BEGIN,
            -1
        );
        elem->content_size.x = roundf(s.x0);
        elem->content_size.y = roundf(s.y0);
    }

    // Compute bounds.
    pax_recti bounds = {
        0,
        0,
        elem->content_size.x,
        elem->content_size.y,
    };

    // Compute cursor position.
    pax_recti cursor = pgui_drawutil_getcursor(
        pgui_effective_font(elem, theme),
        pgui_effective_font_size(elem, theme),
        text->shrink_to_fit,
        text->text,
        text->text_len,
        text->cursor,
        bounds,
        text->text_halign,
        text->text_valign
    );

    // Update scroll position to match cursor.
    elem->scroll = pgui_adjust_scroll_2d(
        cursor,
        pgui_effective_font_size(elem, theme),
        (pax_vec2i){
            elem->size.x - padding.left - padding.right,
            elem->size.y - padding.top - padding.bottom,
        },
        elem->scroll,
        elem->content_size
    );
}

// Visuals for editable text-based elements.
void pgui_draw_textbox(pax_buf_t *gfx, pax_vec2i pos, pgui_elem_t *elem, pgui_theme_t const *theme, uint32_t flags) {
    pgui_drawutil_textbox(gfx, pos, elem, theme, flags, true);
}

// Combined logic for nav left/right, backspace/delete and CTRL.
static pgui_resp_t
    textbox_nav(pgui_elem_t *elem, pgui_theme_t const *theme, uint32_t flags, bool go_right, bool erase, bool ctrl) {
    pgui_text_t *text = (pgui_text_t *)elem;

    // Move cursor to the new location.
    size_t new_cursor;
    if (go_right) {
        if (ctrl) {
            new_cursor = pgui_text_ctrl_right(text->text, text->text_len, text->cursor, !erase);
        } else {
            new_cursor = text->cursor < text->text_len ? text->cursor + 1 : text->cursor;
        }
    } else {
        if (ctrl) {
            new_cursor = pgui_text_ctrl_left(text->text, text->text_len, text->cursor, !erase);
        } else {
            new_cursor = text->cursor ? text->cursor - 1 : text->cursor;
        }
    }

    // If cursor hasn't moved at all
    if (new_cursor == text->cursor) {
        return PGUI_RESP_CAPTURED_ERR;
    }

    if (erase) {
        // Delete / backspace.
        size_t start = go_right ? text->cursor : new_cursor;
        size_t end   = go_right ? new_cursor : text->cursor;

        // Erase characters.
        memcpy(text->text + start, text->text + end, text->text_len - end + 1);
        text->cursor    = start;
        text->text_len -= end - start;

        // If the buffer is too large, shrink it.
        if (text->allow_realloc && text->text_cap >= 8 && text->text_cap >= 2 * text->text_len) {
            void *mem = realloc(text->text, text->text_cap / 2 + 1);
            if (mem) {
                text->text      = mem;
                text->text_cap /= 2;
            }
        }

    } else {
        // Move the cursor.
        text->cursor = new_cursor;
    }
    elem->flags |= PGUI_FLAG_DIRTY;
    pgui_calc2_textbox((pax_vec2i){0}, (pax_vec2i){0}, elem, theme, flags);
    return PGUI_RESP_CAPTURED;
}

// Navigation behaviour for editable text-based elements.
pgui_resp_t pgui_event_textbox(
    pax_vec2i gfx_size, pax_vec2i pos, pgui_elem_t *elem, pgui_theme_t const *theme, uint32_t flags, pgui_event_t event
) {
    pgui_text_t *text = (pgui_text_t *)elem;
    if (flags & PGUI_FLAG_INACTIVE) {
        // Stop editing if inactive.
        elem->flags &= PGUI_FLAG_ACTIVE;
    }
    if (flags & PGUI_FLAG_ACTIVE) {
        // Currently in typing mode.
        if (event.input == PGUI_INPUT_HOME) {
            // Move cursor to the beginning.
            if (event.type == PGUI_EVENT_TYPE_RELEASE) {
                return PGUI_RESP_CAPTURED;
            } else if (!text->cursor) {
                return PGUI_RESP_CAPTURED_ERR;
            }
            text->cursor  = 0;
            elem->flags  |= PGUI_FLAG_DIRTY;
            pgui_calc2_textbox(gfx_size, pos, elem, theme, flags);
            return PGUI_RESP_CAPTURED;

        } else if (event.input == PGUI_INPUT_END) {
            // Move cursor to the end.
            if (event.type == PGUI_EVENT_TYPE_RELEASE) {
                return PGUI_RESP_CAPTURED;
            } else if (text->cursor >= text->text_len) {
                return PGUI_RESP_CAPTURED_ERR;
            }
            text->cursor  = text->text_len;
            elem->flags  |= PGUI_FLAG_DIRTY;
            pgui_calc2_textbox(gfx_size, pos, elem, theme, flags);
            return PGUI_RESP_CAPTURED;

        } else if (event.input == PGUI_INPUT_LEFT || event.input == PGUI_INPUT_PREV) {
            // Move cursor left.
            if (event.type == PGUI_EVENT_TYPE_RELEASE) {
                return PGUI_RESP_CAPTURED;
            }
            return textbox_nav(elem, theme, flags, false, false, event.modkeys & PGUI_MODKEY_CTRL);

        } else if (event.input == PGUI_INPUT_RIGHT || event.input == PGUI_INPUT_NEXT) {
            // Move cursor right.
            if (event.type == PGUI_EVENT_TYPE_RELEASE) {
                return PGUI_RESP_CAPTURED;
            }
            return textbox_nav(elem, theme, flags, true, false, event.modkeys & PGUI_MODKEY_CTRL);

        } else if (event.value == '\b' || event.value == 0x7F) {
            // Delete / backspace.
            if (event.type == PGUI_EVENT_TYPE_RELEASE) {
                return PGUI_RESP_CAPTURED;
            }
            return textbox_nav(elem, theme, flags, event.value == 0x7F, true, event.modkeys & PGUI_MODKEY_CTRL);

        } else if (event.value >= 0x20 && event.value <= 0x7E) {
            // Typable character.
            if (event.type == PGUI_EVENT_TYPE_RELEASE) {
                return PGUI_RESP_CAPTURED;
            }

            if (!text->text) {
                // Allocate memory.
                if (!text->allow_realloc) {
                    return PGUI_RESP_CAPTURED_ERR;
                }
                text->text = malloc(5);
                if (!text->text) {
                    return PGUI_RESP_CAPTURED_ERR;
                }
                text->text_cap = 4;
                text->text[0]  = 0;

            } else if (text->text_len >= text->text_cap) {
                // Enlarge memory.
                if (!text->allow_realloc) {
                    return PGUI_RESP_CAPTURED_ERR;
                }
                void *mem = realloc(text->text, text->text_cap * 2 + 1);
                if (!mem) {
                    return PGUI_RESP_CAPTURED_ERR;
                }
                text->text      = mem;
                text->text_cap *= 2;
            }

            // Move text after cursor.
            memmove(text->text + text->cursor + 1, text->text + text->cursor, text->text_len - text->cursor + 1);

            // Insert character.
            text->text[text->cursor] = event.value;
            text->cursor++;
            text->text_len++;

            // Mark as dirty.
            elem->flags |= PGUI_FLAG_DIRTY;
            pgui_calc2_textbox(gfx_size, pos, elem, theme, flags);
            return PGUI_RESP_CAPTURED;

        } else if (event.input == PGUI_INPUT_ACCEPT || event.input == PGUI_INPUT_BACK) {
            // Finish typing.
            if (event.type == PGUI_EVENT_TYPE_RELEASE) {
                if (elem->callback) {
                    elem->callback(elem);
                }
                elem->flags &= ~PGUI_FLAG_ACTIVE;
                elem->flags |= PGUI_FLAG_DIRTY;
            }
            return PGUI_RESP_CAPTURED;

        } else {
            // Other inputs ignored.
            return event.type == PGUI_EVENT_TYPE_RELEASE ? PGUI_RESP_CAPTURED : PGUI_RESP_CAPTURED_ERR;
        }

    } else {
        // Not in typing mode.
        if (event.input == PGUI_INPUT_ACCEPT) {
            if (event.type == PGUI_EVENT_TYPE_RELEASE) {
                if (flags & PGUI_FLAG_INACTIVE) {
                    return PGUI_RESP_CAPTURED_ERR;
                }
                // Start typing.
                elem->flags |= PGUI_FLAG_ACTIVE;
                elem->flags |= PGUI_FLAG_DIRTY;
            }
            return PGUI_RESP_CAPTURED;
        }
        return PGUI_RESP_IGNORED;
    }
}



// Textbox element type.
pgui_type_t const pgui_type_textbox = {
    .id          = PGUI_TYPE_ID_TEXTBOX,
    .base_struct = PGUI_STRUCT_TEXT,
    .name        = "textbox",
    .attr        = PGUI_ATTR_SELECTABLE | PGUI_ATTR_INPUT,
    .calc2       = pgui_calc2_textbox,
    .draw        = pgui_draw_textbox,
    .event       = pgui_event_textbox,
    .del         = pgui_del_text,
};
