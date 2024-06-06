
// SPDX-License-Identifier: MIT

#include "pax_gui_textbox.h"

#include "pax_internal.h"

#include <string.h>



// Draw a textbox.
static void
    pgui_draw_textbox(pax_buf_t *gfx, pax_vec2f pos, pgui_textbox_t *elem, pgui_theme_t const *theme, uint32_t flags) {
    if (flags & PGUI_FLAG_INACTIVE) {
        // Stop editing if inactive.
        elem->base.flags &= PGUI_FLAG_ACTIVE;
    }
    // Draw backdrop.
    pgui_draw_base(gfx, pos, &elem->base, theme, flags);
    // Calculate text bounds.
    pax_rectf bounds = {
        pos.x + theme->input_padding,
        pos.y + theme->input_padding,
        elem->base.size.x - 2 * theme->input_padding,
        elem->base.size.y - 2 * theme->input_padding,
    };
    // Adjust clip to bounds of text.
    pax_recti clip = pax_get_clip(gfx);
    pax_clip(gfx, bounds.x, bounds.y, bounds.w, bounds.h);

    if (elem->buf && (flags & PGUI_FLAG_ACTIVE)) {
        // Measure until cursor.
        char tmp                = elem->buf[elem->cursor];
        elem->buf[elem->cursor] = 0;
        pax_vec2f size_pre      = pax_text_size(theme->font, theme->font_size, elem->buf);
        elem->buf[elem->cursor] = tmp;
        // Measure after cursor.
        pax_vec2f size_post     = pax_text_size(theme->font, theme->font_size, elem->buf + elem->cursor);
        // Adjust scroll offset.
        elem->scroll            = pgui_adjust_scroll(
            size_pre.x - 2 * theme->font_size,
            0,
            bounds.w,
            elem->scroll,
            4 * theme->font_size,
            size_pre.x + size_post.x
        );

        // Draw all text.
        pax_draw_text(
            gfx,
            theme->fg_col,
            theme->font,
            theme->font_size,
            bounds.x - elem->scroll,
            pos.y + (elem->base.size.y - theme->font_size) / 2,
            elem->buf
        );
        // Draw cursor.
        pax_clip(gfx, pos.x, pos.y, elem->base.size.x, elem->base.size.y);
        pax_draw_line(
            gfx,
            theme->fg_col,
            bounds.x - elem->scroll + size_pre.x,
            pos.y + (elem->base.size.y - theme->font_size) / 2,
            bounds.x - elem->scroll + size_pre.x,
            pos.y + (elem->base.size.y + theme->font_size) / 2
        );

    } else if (flags & PGUI_FLAG_ACTIVE) {
        // Draw a dummy cursor.
        pax_draw_line(
            gfx,
            theme->fg_col,
            bounds.x,
            pos.y + (elem->base.size.y - theme->font_size) / 2,
            bounds.x,
            pos.y + (elem->base.size.y + theme->font_size) / 2
        );

    } else if (elem->buf) {
        // Draw all the text at once.
        pax_draw_text(
            gfx,
            theme->fg_col,
            theme->font,
            theme->font_size,
            bounds.x - elem->scroll,
            pos.y + (elem->base.size.y - theme->font_size) / 2,
            elem->buf
        );
    }

    // Restore original clip.
    pax_set_clip(gfx, clip);
}

// Send an event to a textbox.
static pgui_resp_t pgui_event_textbox(pgui_textbox_t *elem, pgui_event_t event, uint32_t flags) {
    if (flags & PGUI_FLAG_INACTIVE) {
        // Stop editing if inactive.
        elem->base.flags &= PGUI_FLAG_ACTIVE;
    }
    if (flags & PGUI_FLAG_ACTIVE) {
        // Currently in typing mode.
        if (event.input == PGUI_INPUT_LEFT) {
            // Move cursor left.
            if (event.type == PGUI_EVENT_TYPE_RELEASE) {
                return PGUI_RESP_CAPTURED;
            }
            if (elem->cursor) {
                elem->cursor--;
                elem->base.flags |= PGUI_FLAG_DIRTY;
                return PGUI_RESP_CAPTURED;
            } else {
                return PGUI_RESP_CAPTURED_ERR;
            }

        } else if (event.input == PGUI_INPUT_RIGHT) {
            // Move cursor right.
            if (event.type == PGUI_EVENT_TYPE_RELEASE) {
                return PGUI_RESP_CAPTURED;
            }
            if (elem->cursor < elem->buf_len) {
                elem->cursor++;
                elem->base.flags |= PGUI_FLAG_DIRTY;
                return PGUI_RESP_CAPTURED;
            } else {
                return PGUI_RESP_CAPTURED_ERR;
            }

        } else if (event.value == '\b' || event.value == 0x7F) {
            // Delete / backspace.
            if (event.type == PGUI_EVENT_TYPE_RELEASE) {
                return PGUI_RESP_CAPTURED;
            }
            // Adjust cursor position in case of delete.
            if (event.value == 0x7F) {
                if (elem->cursor < elem->buf_len) {
                    elem->cursor++;
                } else {
                    return PGUI_RESP_CAPTURED_ERR;
                }
            } else if (elem->cursor == 0) {
                return PGUI_RESP_CAPTURED_ERR;
            }
            // Move memory.
            memmove(elem->buf + elem->cursor - 1, elem->buf + elem->cursor, elem->buf_len - elem->cursor + 1);
            elem->cursor--;
            elem->buf_len--;
            // If the buffer is way too large, shrink it.
            if (elem->buf_cap >= 8 && elem->buf_cap >= 2 * elem->buf_len) {
                void *mem = realloc(elem->buf, elem->buf_cap / 2 + 1);
                if (mem) {
                    elem->buf      = mem;
                    elem->buf_cap /= 2;
                }
            }
            // Mark as dirty.
            elem->base.flags |= PGUI_FLAG_DIRTY;

        } else if (event.value >= 0x20 && event.value <= 0x7E) {
            // Typable character.
            if (event.type == PGUI_EVENT_TYPE_RELEASE) {
                return PGUI_RESP_CAPTURED;
            }

            if (!elem->buf) {
                // Allocate memory.
                elem->buf = malloc(5);
                if (!elem->buf) {
                    return PGUI_RESP_CAPTURED_ERR;
                }
                elem->buf_cap = 4;
                elem->buf[0]  = 0;

            } else if (elem->buf_len >= elem->buf_cap) {
                // Enlarge memory.
                void *mem = realloc(elem->buf, elem->buf_cap * 2 + 1);
                if (!mem) {
                    return PGUI_RESP_CAPTURED_ERR;
                }
                elem->buf      = mem;
                elem->buf_cap *= 2;
            }

            // Move text after cursor.
            memmove(elem->buf + elem->cursor + 1, elem->buf + elem->cursor, elem->buf_len - elem->cursor + 1);
            // Insert character.
            elem->buf[elem->cursor] = event.value;
            elem->cursor++;
            elem->buf_len++;
            // Mark as dirty.
            elem->base.flags |= PGUI_FLAG_DIRTY;

        } else if (event.input == PGUI_INPUT_ACCEPT || event.input == PGUI_INPUT_BACK) {
            // Finish typing.
            if (event.type == PGUI_EVENT_TYPE_RELEASE) {
                if (elem->callback) {
                    elem->callback(elem, elem->cookie);
                }
                elem->base.flags &= ~PGUI_FLAG_ACTIVE;
                elem->base.flags |= PGUI_FLAG_DIRTY;
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
                elem->base.flags |= PGUI_FLAG_ACTIVE;
                elem->base.flags |= PGUI_FLAG_DIRTY;
            }
            return PGUI_RESP_CAPTURED;
        }
        return PGUI_RESP_IGNORED;
    }
}

// Textbox element type.
pgui_type_t pgui_type_textbox_raw = {
    .attr  = PGUI_ATTR_SELECTABLE,
    .draw  = (pgui_draw_fn_t)pgui_draw_textbox,
    .event = (pgui_event_fn_t)pgui_event_textbox,
};
