
// SPDX-License-Identifier: MIT

#ifndef PAX_GUI_TYPES_H
#define PAX_GUI_TYPES_H

#include "pax_gfx.h"

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus



// GUI input button type.
typedef enum {
    // No equivalent input.
    PGUI_INPUT_NONE,

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
    PGUI_EVENT_TYPE_PRESS,
    // Button repeated.
    PGUI_EVENT_TYPE_HOLD,
    // Button released.
    PGUI_EVENT_TYPE_RELEASE,
} pgui_event_type_t;

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

// GUI input event.
typedef struct {
    // Event type.
    pgui_event_type_t type;
    // Equivalent input action, if any.
    pgui_input_t      input;
    // Equivalent character, if any.
    char              value;
} pgui_event_t;


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
    (x == PGUI_TYPE_BOX || x == PGUI_TYPE_GRID || x == PGUI_TYPE_BUTTON || x == PGUI_TYPE_DROPDOWN                     \
     || x == PGUI_TYPE_TEXTBOX)

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
    // `pgui_textbox_t` elements.
    PGUI_TYPE_TEXTBOX,
    // `pgui_text_t` elements.
    PGUI_TYPE_TEXT,
    // `pgui_label_t` elements.
    PGUI_TYPE_LABEL,
} pgui_type_t;

// Common GUI element data.
typedef struct pgui_base pgui_base_t;

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



// Draw the base of a box or input element.
void pgui_draw_base(pax_buf_t *gfx, pax_vec2f pos, pgui_base_t *elem, pgui_theme_t const *theme, uint32_t flags);

// Shrink text to fit bounds.
void pgui_draw_bounded_text(
    pax_buf_t        *gfx,
    pax_col_t         color,
    pax_font_t const *font,
    float             font_size,
    char const       *text,
    pax_rectf         bounds,
    pax_text_align_t  align
);

// Draw a scrollbar.
void pgui_draw_scrollbar(
    pax_buf_t *gfx, pax_vec2f pos, pax_vec2f size, pgui_theme_t const *theme, float scroll, float window, float total
);

// Adjust a scrollbar to show as much of the desired area as possible.
float pgui_adjust_scroll(
    float view_offset, float view_margin, float view_window, float scroll, float window, float total
);

#ifdef __cplusplus
} // extern "C"
#endif //__cplusplus

#endif // PAX_GUI_TYPES_H
