
// SPDX-License-Identifier: MIT

#ifndef PAX_GUI_H
#define PAX_GUI_H

#include "pax_gfx.h"

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus



// GUI input button type.
typedef enum {
    // DPAD left.
    PGUI_INPUT_LEFT,
    // DPAD right.
    PGUI_INPUT_RIGHT,
    // DPAD up.
    PGUI_INPUT_UP,
    // DPAD down.
    PGUI_INPUT_DOWN,

    // Accept / enter.
    PGUI_INPUT_ACCEPT,
    // Back / escape.
    PGUI_INPUT_BACK,
} pgui_input_t;

// GUI input button action.
typedef enum {
    // Initial button pressed.
    PGUI_EVENT_PRESS,
    // Button repeated.
    PGUI_EVENT_HOLD,
    // Button released.
    PGUI_EVENT_RELEASE,
} pgui_event_t;

// GUI hierarchical event response.
typedef enum {
    // Event ignored.
    PGUI_RESP_IGNORED,
    // Event captured (action taken).
    PGUI_RESP_CAPTURED,
    // Event captured (complete re-draw required).
    PGUI_RESP_CAPTURED_DIRTY,
    // Event captured (action not possible).
    PGUI_RESP_CAPTURED_ERR,
} pgui_resp_t;


// GUI theme properties.
typedef struct {
    /* ==== Element styles ==== */
    // Background color.
    pax_col_t bg_col;
    // Foreground color.
    pax_col_t fg_col;
    // Button/dropdown background color.
    pax_col_t input_col;
    // Pressed button color.
    pax_col_t pressed_col;
    // Border color.
    pax_col_t border_col;
    // Highlighted border color.
    pax_col_t highlight_col;
    // Element corner rounding.
    float     rounding;
    // Internal padding for inputs.
    float     input_padding;
    // Internal padding for text and labels.
    float     text_padding;
    // Internal padding for boxes grid cells.
    float     box_padding;

    /* ==== Text style ==== */
    // GUI font.
    pax_font_t const *font;
    // GUI font scale.
    float             font_size;

    /* ==== Dropdown style ==== */
    struct {
        // Use the segmented drop-down style with the thinner menu.
        uint32_t dropdown_segmented     : 1;
        // Use the solid arrow for the drop-down.
        uint32_t dropdown_solid_arrow   : 1;
        // Cover the dropdown its menu, instead of avoiding covering it.
        uint32_t dropdown_covering_menu : 1;
        // Padding.
        uint32_t                        : 29;
    };

    /* ==== Scrollbar style ==== */
    // Scrollbar background color.
    pax_col_t scroll_bg_col;
    // Scrollbar foreground color.
    pax_col_t scroll_fg_col;
    // Scrollbar width.
    float     scroll_width;
    // Scroller minimum size.
    float     scroll_min_size;
    // Scrollbar offset.
    float     scroll_offset;
    // Scrollbar rounding.
    float     scroll_rounding;
} pgui_theme_t;

// Default theme.
extern pgui_theme_t const pgui_theme_default;


// GUI element inheritable flag: Hidden.
#define PGUI_FLAG_HIDDEN       0x00000001
// GUI element inheritable flag: Inactive.
// Buttons can't be pressed, inputs can't be edited.
#define PGUI_FLAG_INACTIVE     0x00000002
// GUI element inheritable flag: Needs re-draw.
#define PGUI_FLAG_DIRTY        0x00000004
// Bitmask of inheritable flags.
#define PGUI_FLAGS_INHERITABLE 0x000000ff
// GUI container flag: Do not draw background/border.
#define PGUI_FLAG_NOBACKGROUND 0x00000100
// GUI grid flag: Do not draw separators between cells.
#define PGUI_FLAG_NOSEPARATOR  0x00000200
// GUI element flag: Button pressed / dropdown opened.
#define PGUI_FLAG_ACTIVE       0x00000400
// GUI element flag: Draw as highlighted.
#define PGUI_FLAG_HIGHLIGHT    0x00000800
// GUI element flag: Fill the cell width of the parent.
#define PGUI_FLAG_FILLCELL     0x00001000
// GUI element flag: Do not add padding.
#define PGUI_FLAG_NOPADDING    0x00002000

// Whether an element type is one of the boxes.
#define PGUI_IS_BOX(x) (x == PGUI_TYPE_BOX || x == PGUI_TYPE_GRID)
// Whether an element type is selectable.
#define PGUI_IS_SELECTABLE(x)                                                                                          \
    (x == PGUI_TYPE_BOX || x == PGUI_TYPE_GRID || x == PGUI_TYPE_BUTTON || x == PGUI_TYPE_DROPDOWN)

// GUI element type.
typedef enum {
    // `pgui_box_t` elements.
    PGUI_TYPE_BOX,
    // `pgui_grid_t` elements.
    PGUI_TYPE_GRID,
    // `pgui_button_t` elements.
    PGUI_TYPE_BUTTON,
    // `pgui_dropdown_t` elements.
    PGUI_TYPE_DROPDOWN,
    // `pgui_text_t` elements.
    PGUI_TYPE_TEXT,
    // `pgui_label_t` elements.
    PGUI_TYPE_LABEL,
} pgui_type_t;

// Common GUI element data.
typedef struct pgui_base     pgui_base_t;
// Contains other GUI elements, which may include other boxes.
// This is the abstract container and does not have any logic for selecting children.
// However, selected children will still receive input events.
typedef struct pgui_box      pgui_box_t;
// A grid or list of GUI elements.
// Implements arrow-key / DPAD selection logic.
// Can also be used as a list if either `cells.x` or `cells.y` is 1.
typedef struct pgui_grid     pgui_grid_t;
// A button that can be pressed, optionally calling a function.
typedef struct pgui_button   pgui_button_t;
// A drop-down menu for selecting from a fixed set of options.
typedef struct pgui_dropdown pgui_dropdown_t;
// A paragraph of text with adjustable alignment.
typedef struct pgui_text     pgui_text_t;
// A vertically centered, shrink-to-fit label with adjustable alignment.
typedef struct pgui_text     pgui_label_t;

// Button press callback.
typedef void (*pgui_button_cb_t)(pgui_button_t *button, void *cookie);
// Dropdown select callback.
typedef void (*pgui_dropdown_cb_t)(pgui_dropdown_t *dropdown, void *cookie, size_t selected);

struct pgui_base {
    // Element type.
    pgui_type_t  type;
    // Relative element position.
    pax_vec2f    pos;
    // Element size.
    pax_vec2f    size;
    // Element flags.
    // Effects of inheritable flags are applied to child elements.
    uint32_t     flags;
    // Parent element, set automatically.
    pgui_base_t *parent;
};

struct pgui_box {
    // Common GUI element data.
    pgui_base_t   base;
    // Index of selected child, if any.
    ptrdiff_t     selected;
    // Number of children.
    size_t        children_len;
    // Pointers to child elements.
    pgui_base_t **children;
};

struct pgui_grid {
    union {
        // Common GUI element data.
        pgui_base_t base;
        // Common GUI container data.
        pgui_box_t  box;
    };
    // How many cells wide or high the grid is.
    pax_vec2i cells;
    // Size of an individual cell.
    pax_vec2f cell_size;
};

struct pgui_button {
    // Common GUI element data.
    pgui_base_t      base;
    // Button text.
    char const      *text;
    // Button press callback.
    pgui_button_cb_t callback;
    // Button press cookie.
    void            *cookie;
};

struct pgui_dropdown {
    // Common GUI element data.
    pgui_base_t        base;
    // Scroll offset.
    float              scroll;
    // Selected option.
    size_t             selected;
    // Option hovered in the selection menu.
    size_t             to_select;
    // Number of options.
    size_t             options_len;
    // Option text.
    char const       **options;
    // Dropdown select callback.
    pgui_dropdown_cb_t callback;
    // Dropdown select cookie.
    void              *cookie;
};

struct pgui_text {
    // Common GUI element data.
    pgui_base_t      base;
    // Text to show.
    char const      *text;
    // Text alignment.
    pax_text_align_t align;
};



// Recalculate the position of a GUI element and its children.
void        pgui_calc_layout(pgui_base_t *elem, pgui_theme_t const *theme);
// Draw a GUI element and its children.
void        pgui_draw(pax_buf_t *gfx, pgui_base_t *elem, pgui_theme_t const *theme);
// Re-draw dirty parts of the GUI and mark the elements clean.
void        pgui_redraw(pax_buf_t *gfx, pgui_base_t *elem, pgui_theme_t const *theme);
// Handle a button event.
// Returns if and how the event was handled.
pgui_resp_t pgui_event(pgui_base_t *elem, pgui_input_t button, pgui_event_t event);

#ifdef __cplusplus
} // extern "C"
#endif //__cplusplus

#endif // PAX_GUI_H
