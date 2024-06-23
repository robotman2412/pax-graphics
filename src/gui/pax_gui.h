
// SPDX-License-Identifier: MIT

#ifndef PAX_GUI_TYPES_H
#define PAX_GUI_TYPES_H

#include "pax_gfx.h"

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus



/* ==== SDL2-compatible modifier keys ==== */
// Left shift pressed.
#define PGUI_MODKEY_L_SHIFT 0x0001
// Right shift pressed.
#define PGUI_MODKEY_R_SHIFT 0x0002
// Left control pressed.
#define PGUI_MODKEY_L_CTRL  0x0040
// Right control pressed.
#define PGUI_MODKEY_R_CTRL  0x0080
// Left alt pressed.
#define PGUI_MODKEY_L_ALT   0x0100
// Right alt pressed.
#define PGUI_MODKEY_R_ALT   0x0200
// Num lock active.
#define PGUI_MODKEY_NUM     0x1000
// Caps lock active.
#define PGUI_MODKEY_CAPS    0x2000
// Scroll lock active.
#define PGUI_MODKEY_SCROLL  0x8000
// Any control key pressed.
#define PGUI_MODKEY_CTRL    (PGUI_MODKEY_L_CTRL | PGUI_MODKEY_R_CTRL)
// Any shift key pressed.
#define PGUI_MODKEY_SHIFT   (PGUI_MODKEY_L_SHIFT | PGUI_MODKEY_R_SHIFT)
// Any alt kay pressed.
#define PGUI_MODKEY_ALT     (PGUI_MODKEY_L_ALT | PGUI_MODKEY_R_ALT)

// GUI input button type.
typedef enum {
    // No equivalent input.
    PGUI_INPUT_NONE,

    // Navigate to the previous element/option.
    PGUI_INPUT_PREV,
    // Navigate to the next element/option.
    PGUI_INPUT_NEXT,

    // DPAD left.
    PGUI_INPUT_LEFT,
    // DPAD right.
    PGUI_INPUT_RIGHT,
    // DPAD up.
    PGUI_INPUT_UP,
    // DPAD down.
    PGUI_INPUT_DOWN,

    // Home / fast left.
    PGUI_INPUT_HOME,
    // End / fast right.
    PGUI_INPUT_END,
    // PageUp / fast up.
    PGUI_INPUT_PGUP,
    // PageDn / fast down.
    PGUI_INPUT_PGDN,

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
    // Active modifier keys, if any.
    uint32_t          modkeys;
} pgui_event_t;


// GUI theme properties.
typedef struct {
    /* ==== Element styles ==== */
    // Background color.
    pax_col_t bg_col;
    // Foreground color.
    pax_col_t fg_col;
    // Textbox background color.
    pax_col_t input_col;
    // Active textbox color.
    pax_col_t active_col;
    // Button/dropdown background color.
    pax_col_t button_col;
    // Pressed button color.
    pax_col_t pressed_col;
    // Border color.
    pax_col_t border_col;
    // Highlighted border color.
    pax_col_t highlight_col;
    // Normal element border thickness
    int       border_thickness;
    // Highlighted element border thickness.
    int       highlight_thickness;
    // Element corner rounding.
    int       rounding;
    // Internal padding elements.
    int       padding;

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
    int       scroll_width;
    // Scroller minimum size.
    int       scroll_min_size;
    // Scrollbar offset.
    int       scroll_offset;
    // Scrollbar rounding.
    int       scroll_rounding;
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
// GUI element flag: Do not draw background.
#define PGUI_FLAG_NOBACKGROUND 0x00000100
// GUI element flag: Do not draw border.
#define PGUI_FLAG_NOBORDER     0x00000200
// GUI grid flag: Do not draw separators between cells.
#define PGUI_FLAG_NOSEPARATOR  0x00000400
// GUI element flag: Button pressed / dropdown opened.
#define PGUI_FLAG_ACTIVE       0x00000800
// GUI element flag: Draw as highlighted.
#define PGUI_FLAG_HIGHLIGHT    0x00001000
// GUI element flag: Fill the cell width of the parent.
#define PGUI_FLAG_FILLCELL     0x00002000
// GUI element flag: Do not add padding.
#define PGUI_FLAG_NOPADDING    0x00004000

// GUI attribute: Type is selectable.
#define PGUI_ATTR_SELECTABLE 0x00000001
// GUI attribute: Type describes an input.
// Elements with this type will use the input color scheme.
#define PGUI_ATTR_INPUT      0x00000002
// GUI attribute: Type describes a button.
// Elements with this type will use the button color scheme.
#define PGUI_ATTR_BUTTON     0x00000004
// GUI attribute: Type can have scrollbars.
// Elements with this type won't have their content size set to their actual size.
#define PGUI_ATTR_SCROLLABLE 0x00000008

// GUI element type.
typedef struct pgui_type pgui_type_t;
// Base GUI element.
typedef struct pgui_elem pgui_elem_t;
// Data for grid elements.
typedef struct pgui_grid pgui_grid_t;
// Data for text-based elements.
typedef struct pgui_text pgui_text_t;

// GUI element draw call.
typedef void (*pgui_draw_fn_t)(
    pax_buf_t *gfx, pax_vec2i pos, pgui_elem_t *elem, pgui_theme_t const *theme, uint32_t flags
);
// GUI element layout calculation call.
typedef void (*pgui_calc_fn_t)(
    pax_vec2i gfx_size, pax_vec2i pos, pgui_elem_t *elem, pgui_theme_t const *theme, uint32_t flags
);
// GUI element event call.
typedef pgui_resp_t (*pgui_event_fn_t)(
    pax_vec2i gfx_size, pax_vec2i pos, pgui_elem_t *elem, pgui_theme_t const *theme, uint32_t flags, pgui_event_t event
);
// GUI button press / input changed callback.
typedef void (*pgui_callback_t)(pgui_elem_t *elem);

// GUI element type.
struct pgui_type {
    // Static element attributes.
    uint32_t        attr;
    // Minimum element size.
    pax_vec2i       min_size;
    // Draw call (mandatory).
    pgui_draw_fn_t  draw;
    // Layout calculation call.
    pgui_calc_fn_t  calc;
    // Event call.
    pgui_event_fn_t event;
};

struct pgui_elem {
    // Element type.
    pgui_type_t const *type;
    // Element flags.
    // Effects of inheritable flags are applied to child elements.
    uint32_t           flags;
    // Parent element, set automatically.
    pgui_elem_t       *parent;

    // Relative element position.
    pax_vec2i pos;
    // Element size.
    pax_vec2i size;
    // Content size.
    pax_vec2i content_size;
    // Scroll offset.
    pax_vec2i scroll;

    // Number of child elements.
    size_t        children_len;
    // Child elements.
    pgui_elem_t **children;
    // Selected child, if any.
    ptrdiff_t     selected;

    // Button pressed / input changed callback.
    pgui_callback_t callback;
    // User-specified data pointer.
    void           *userdata;
};

struct pgui_grid {
    // Common element data.
    pgui_elem_t base;
    // Size in grid cells.
    pax_vec2i   cells;
    // Per-row size.
    int        *row_height;
    // Per-column size.
    int        *col_width;
};

struct pgui_text {
    // Common element data.
    pgui_elem_t base;
    // Text buffer capacity.
    size_t      text_cap;
    // Text buffer length.
    size_t      text_len;
    // Text to display.
    char       *text;
    // Horizontal alignment of the text.
    pax_align_t text_halign;
    // Vertical alignment of the text.
    pax_align_t text_valign;
    // Cursor position.
    size_t      cursor;
    // Shrink text to fit, instead of adding a scrollbar.
    bool        shrink_to_fit;
    // Text buffer can be realloc()'d.
    bool        allow_realloc;
    // Users are allowed to enter newlines.
    bool        allow_newline;
};



extern pgui_type_t pgui_type_button_raw;
extern pgui_type_t pgui_type_text_raw;
extern pgui_type_t pgui_type_textbox_raw;
extern pgui_type_t pgui_type_grid_raw;
#define PGUI_TYPE_BUTTON  &pgui_type_button_raw
#define PGUI_TYPE_TEXT    &pgui_type_text_raw
#define PGUI_TYPE_TEXTBOX &pgui_type_textbox_raw
#define PGUI_TYPE_GRID    &pgui_type_grid_raw



// Create a simple button.
#define PGUI_NEW_BUTTON(button_label)                                                                                  \
    (pgui_text_t) {                                                                                                    \
        .base = {.type = PGUI_TYPE_BUTTON, .flags = PGUI_FLAG_FILLCELL}, .text_len = strlen(button_label),             \
        .text = (button_label), .shrink_to_fit = true, .text_halign = PAX_ALIGN_CENTER,                                \
        .text_valign = PAX_ALIGN_CENTER,                                                                               \
    }

// Create a simple text label.
#define PGUI_NEW_LABEL(label_text)                                                                                       \
    (pgui_text_t) {                                                                                                      \
        .base     = {.type = PGUI_TYPE_TEXT, .flags = PGUI_FLAG_FILLCELL | PGUI_FLAG_NOBACKGROUND | PGUI_FLAG_NOBORDER}, \
        .text_len = strlen(label_text), .text = (label_text), .shrink_to_fit = true, .text_halign = PAX_ALIGN_CENTER,    \
        .text_valign = PAX_ALIGN_CENTER,                                                                                 \
    }

// Create a simple editable textbox.
#define PGUI_NEW_TEXTBOX()                                                                                             \
    (pgui_text_t) {                                                                                                    \
        .base = {.type = PGUI_TYPE_TEXTBOX, .flags = PGUI_FLAG_FILLCELL}, .allow_realloc = true,                       \
        .text_halign = PAX_ALIGN_BEGIN, .text_valign = PAX_ALIGN_CENTER,                                               \
    }

// Create a new scrollable grid.
#define PGUI_NEW_GRID(pos_x, pos_y, size_x, size_y, cells_x, cells_y, ...)                                             \
    (pgui_grid_t) {                                                                                                    \
        .base = {                                                                                           \
            .type         = PGUI_TYPE_GRID,                                                                            \
            .pos          = {(pos_x), (pos_y)},                                                                        \
            .size         = {(size_x), (size_y)},                                                                      \
            .children_len = (cells_x) * (cells_y),                                                                     \
            .children     = (void*)(void *[]){__VA_ARGS__},                                                            \
            .selected     = -1,                                                                                        \
            .flags        = PGUI_FLAG_FILLCELL,                                                                        \
        },                                                                                                             \
        .cells = {(cells_x), (cells_y)}, .col_width = (int[(cells_x)]){0}, .row_height = (int[(cells_y)]){0},          \
    }



// Calculate the layout of a grid.
void pgui_calc_grid(pax_vec2i gfx_size, pax_vec2i pos, pgui_elem_t *elem, pgui_theme_t const *theme, uint32_t flags);
// Calculate the layout of editable text-based elements.
void pgui_calc_textbox(pax_vec2i gfx_size, pax_vec2i pos, pgui_elem_t *elem, pgui_theme_t const *theme, uint32_t flags);

// Visuals for text-based elements.
void pgui_draw_text(pax_buf_t *gfx, pax_vec2i pos, pgui_elem_t *elem, pgui_theme_t const *theme, uint32_t flags);
// Visuals for editable text-based elements.
void pgui_draw_textbox(pax_buf_t *gfx, pax_vec2i pos, pgui_elem_t *elem, pgui_theme_t const *theme, uint32_t flags);
// Visuals for grid-based elements.
void pgui_draw_grid(pax_buf_t *gfx, pax_vec2i pos, pgui_elem_t *elem, pgui_theme_t const *theme, uint32_t flags);

// Behaviour for button elements.
pgui_resp_t pgui_event_button(
    pax_vec2i gfx_size, pax_vec2i pos, pgui_elem_t *elem, pgui_theme_t const *theme, uint32_t flags, pgui_event_t event
);
// Behaviour for editable text-based elements.
pgui_resp_t pgui_event_textbox(
    pax_vec2i gfx_size, pax_vec2i pos, pgui_elem_t *elem, pgui_theme_t const *theme, uint32_t flags, pgui_event_t event
);
// Navigation behaviour for grid-based elements.
pgui_resp_t pgui_event_grid(
    pax_vec2i gfx_size, pax_vec2i pos, pgui_elem_t *elem, pgui_theme_t const *theme, uint32_t flags, pgui_event_t event
);



// Recalculate the position of a GUI element and its children.
void        pgui_calc_layout(pax_vec2i gfx_size, pgui_elem_t *elem, pgui_theme_t const *theme);
// Draw a GUI element and its children.
void        pgui_draw(pax_buf_t *gfx, pgui_elem_t *elem, pgui_theme_t const *theme);
// Re-draw dirty parts of the GUI and mark the elements clean.
void        pgui_redraw(pax_buf_t *gfx, pgui_elem_t *elem, pgui_theme_t const *theme);
// Handle a button event.
// Returns if and how the event was handled.
pgui_resp_t pgui_event(pax_vec2i gfx_size, pgui_elem_t *elem, pgui_theme_t const *theme, pgui_event_t event);

#ifdef __cplusplus
} // extern "C"
#endif //__cplusplus

#endif // PAX_GUI_TYPES_H
