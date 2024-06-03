
// SPDX-License-Identifier: MIT

#include "pax_gui.h"
#include "pax_internal.h"

static char const TAG[] = "pax-gui";



// Internal event handler.
static pgui_resp_t pgui_event_int(pgui_base_t *elem, pgui_input_t input, pgui_event_t event, uint32_t flags);

// Send an event to a box.
pgui_resp_t pgui_event_box(pgui_box_t *elem, pgui_input_t input, pgui_event_t event, uint32_t flags) {
    return PGUI_RESP_IGNORED;
}

// Navigation for grid elements.
pgui_resp_t pgui_grid_nav(pgui_grid_t *elem, ptrdiff_t dx, ptrdiff_t dy) {
    // Original position.
    ptrdiff_t x0 = elem->box.selected % elem->cells.x;
    ptrdiff_t y0 = elem->box.selected / elem->cells.x;

    // Current position.
    ptrdiff_t x = (x0 + dx + elem->cells.x) % elem->cells.x;
    ptrdiff_t y = (y0 + dy + elem->cells.y) % elem->cells.y;
    while (x != x0 || y != y0) {
        ptrdiff_t i = x + y * elem->cells.x;
        if (elem->box.children[i] && PGUI_IS_SELECTABLE(elem->box.children[i]->type)) {
            if (elem->box.selected >= 0) {
                // Unmark previous selection.
                elem->box.children[elem->box.selected]->flags &= ~PGUI_FLAG_HIGHLIGHT;
                elem->box.children[elem->box.selected]->flags |= PGUI_FLAG_DIRTY;
            }
            // Mark new selection.
            elem->box.selected            = i;
            elem->box.children[i]->flags |= PGUI_FLAG_HIGHLIGHT | PGUI_FLAG_DIRTY;
            return PGUI_RESP_CAPTURED;
        }
        x = (x + dx + elem->cells.x) % elem->cells.x;
        y = (y + dy + elem->cells.y) % elem->cells.y;
    }

    return PGUI_RESP_CAPTURED_ERR;
}

// Send an event to a grid.
pgui_resp_t pgui_event_grid(pgui_grid_t *elem, pgui_input_t input, pgui_event_t event, uint32_t flags) {
    if (elem->box.selected < 0) {
        if (input == PGUI_INPUT_ACCEPT && event == PGUI_EVENT_RELEASE) {
            // Select lowest-indexed selectable child.
            for (size_t i = 0; i < elem->box.children_len; i++) {
                if (elem->box.children[i] && PGUI_IS_SELECTABLE(elem->box.children[i]->type)) {
                    elem->box.selected            = i;
                    elem->box.children[i]->flags |= PGUI_FLAG_HIGHLIGHT | PGUI_FLAG_DIRTY;
                    elem->base.flags             |= PGUI_FLAG_DIRTY;
                    elem->base.flags             &= ~PGUI_FLAG_HIGHLIGHT;
                    return PGUI_RESP_CAPTURED;
                }
            }
            return PGUI_RESP_CAPTURED_ERR;
        } else if (input == PGUI_INPUT_ACCEPT) {
            // Selecting happens on release, not press.
            return PGUI_RESP_CAPTURED;
        } else {
            // Other inputs while not selected are ignored.
            return PGUI_RESP_IGNORED;
        }

    } else {
        // Send input to children first.
        pgui_resp_t resp = pgui_event_int(
            elem->box.children[elem->box.selected],
            input,
            event,
            (flags | elem->base.flags) & PGUI_FLAGS_INHERITABLE
        );
        if (resp) {
            return resp;
        }

        // If not captured, handle events at this level.
        if (event == PGUI_EVENT_RELEASE) {
            // No action on button release.
            return PGUI_RESP_CAPTURED;

        } else if (input == PGUI_INPUT_BACK && event == PGUI_EVENT_PRESS) {
            // Un-select child; re-select self.
            elem->box.children[elem->box.selected]->flags &= ~PGUI_FLAG_HIGHLIGHT;
            elem->box.children[elem->box.selected]->flags |= PGUI_FLAG_DIRTY;
            elem->box.selected                             = -1;
            elem->base.flags                              |= PGUI_FLAG_HIGHLIGHT | PGUI_FLAG_DIRTY;

        } else if (input == PGUI_INPUT_UP) {
            // Navigate up.
            return pgui_grid_nav(elem, 0, -1);

        } else if (input == PGUI_INPUT_DOWN) {
            // Navigate down.
            return pgui_grid_nav(elem, 0, 1);

        } else if (input == PGUI_INPUT_LEFT) {
            // Navigate left.
            return pgui_grid_nav(elem, -1, 0);

        } else if (input == PGUI_INPUT_RIGHT) {
            // Navigate right.
            return pgui_grid_nav(elem, 1, 0);
        }
        return PGUI_RESP_CAPTURED;
    }
}

// Send an event to a button.
pgui_resp_t pgui_event_button(pgui_button_t *elem, pgui_input_t input, pgui_event_t event, uint32_t flags) {
    if (input != PGUI_INPUT_ACCEPT) {
        return (elem->base.flags & PGUI_FLAG_ACTIVE) ? PGUI_RESP_CAPTURED_ERR : PGUI_RESP_IGNORED;
    }
    if (flags & PGUI_FLAG_INACTIVE) {
        return PGUI_RESP_CAPTURED_ERR;
    }
    if (event == PGUI_EVENT_PRESS) {
        elem->base.flags |= PGUI_FLAG_DIRTY;
        elem->base.flags |= PGUI_FLAG_ACTIVE;
    } else if (event == PGUI_EVENT_RELEASE && flags & PGUI_FLAG_ACTIVE) {
        elem->base.flags |= PGUI_FLAG_DIRTY;
        elem->base.flags &= ~PGUI_FLAG_ACTIVE;
        if (elem->callback) {
            elem->callback(elem, elem->cookie);
        }
    }
    return PGUI_RESP_CAPTURED;
}

// Send an event to a dropdown.
pgui_resp_t pgui_event_dropdown(pgui_dropdown_t *elem, pgui_input_t input, pgui_event_t event, uint32_t flags) {
    if (flags & PGUI_FLAG_INACTIVE) {
        // Close dropdown if inactive.
        elem->base.flags &= PGUI_FLAG_ACTIVE;
    }
    if (flags & PGUI_FLAG_ACTIVE) {
        // The dropdown is currently open.
        if (input == PGUI_INPUT_ACCEPT) {
            if (event == PGUI_EVENT_RELEASE) {
                // Selection accepted.
                elem->selected    = elem->to_select;
                elem->base.flags &= ~PGUI_FLAG_ACTIVE;
                return PGUI_RESP_CAPTURED_DIRTY;
            } else {
                return PGUI_RESP_CAPTURED;
            }
        } else if (input == PGUI_INPUT_BACK) {
            if (event == PGUI_EVENT_RELEASE) {
                // Selection rejected.
                elem->base.flags &= ~PGUI_FLAG_ACTIVE;
                return PGUI_RESP_CAPTURED_DIRTY;
            } else {
                return PGUI_RESP_CAPTURED;
            }
        } else if (input == PGUI_INPUT_UP) {
            // Navigate up.
            if (event != PGUI_EVENT_RELEASE) {
                elem->to_select   = (elem->to_select + elem->options_len - 1) % elem->options_len;
                elem->base.flags |= PGUI_FLAG_DIRTY;
            }
            return PGUI_RESP_CAPTURED;
        } else if (input == PGUI_INPUT_DOWN) {
            // Navigate down.
            if (event != PGUI_EVENT_RELEASE) {
                elem->to_select   = (elem->to_select + 1) % elem->options_len;
                elem->base.flags |= PGUI_FLAG_DIRTY;
            }
            return PGUI_RESP_CAPTURED;
        } else {
            // Other inputs not accepted.
            return event == PGUI_EVENT_PRESS ? PGUI_RESP_CAPTURED_ERR : PGUI_RESP_CAPTURED;
        }

    } else {
        // The dropdown is currently closed.
        if (input == PGUI_INPUT_ACCEPT) {
            if (event == PGUI_EVENT_RELEASE) {
                if (flags & PGUI_FLAG_INACTIVE) {
                    return PGUI_RESP_CAPTURED_ERR;
                }
                // Open the drop-down.
                elem->to_select   = elem->selected;
                elem->base.flags |= PGUI_FLAG_DIRTY | PGUI_FLAG_ACTIVE;
            }
            return PGUI_RESP_CAPTURED;
        } else {
            // Other inputs ignored.
            return PGUI_RESP_IGNORED;
        }
    }
}

// Internal event handler.
static pgui_resp_t pgui_event_int(pgui_base_t *elem, pgui_input_t input, pgui_event_t event, uint32_t flags) {
    flags |= elem->flags;
    switch (elem->type) {
        default: PAX_LOGE(TAG, "Cannot send event to unknown element type %d", elem->type); return PGUI_RESP_IGNORED;
        case PGUI_TYPE_BOX: return pgui_event_box((pgui_box_t *)elem, input, event, flags);
        case PGUI_TYPE_GRID: return pgui_event_grid((pgui_grid_t *)elem, input, event, flags);
        case PGUI_TYPE_BUTTON: return pgui_event_button((pgui_button_t *)elem, input, event, flags);
        case PGUI_TYPE_DROPDOWN: return pgui_event_dropdown((pgui_dropdown_t *)elem, input, event, flags);
        case PGUI_TYPE_TEXT: return PGUI_RESP_IGNORED;
        case PGUI_TYPE_LABEL: return PGUI_RESP_IGNORED;
    }
}

// Handle a button event.
// Returns whether any element was marked dirty in response.
pgui_resp_t pgui_event(pgui_base_t *elem, pgui_input_t input, pgui_event_t event) {
    pgui_resp_t resp = pgui_event_int(elem, input, event, 0);
    if (resp == PGUI_RESP_CAPTURED_DIRTY) {
        elem->flags |= PGUI_FLAG_DIRTY;
    }
    return resp;
}
