
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

// Element type IDs.
typedef enum {
    // Custom type.
    /* PGUI_TYPE_ID_CUSTOM = -1, */
    // Built-in: Button.
    PGUI_TYPE_ID_BUTTON,
    // Built-in: Text.
    PGUI_TYPE_ID_TEXT,
    // Built-in: Textbox.
    PGUI_TYPE_ID_TEXTBOX,
    // Built-in: Grid.
    PGUI_TYPE_ID_GRID,
    // Built-in: Dropdown
    PGUI_TYPE_ID_DROPDOWN,
    // Built-in: Image.
    PGUI_TYPE_ID_IMAGE,
} pgui_type_id_t;


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

    /* ==== Element sizes ==== */
    // Minimum element size.
    pax_vec2i min_size;
    // Minimum element size for inputs.
    pax_vec2i min_input_size;
    // Minimum element size for labels.
    pax_vec2i min_label_size;
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
#define PGUI_FLAG_HIDDEN   0x00000001
// GUI element inheritable flag: Inactive.
// Buttons can't be pressed, inputs can't be edited.
#define PGUI_FLAG_INACTIVE 0x00000002
// GUI element inheritable flag: Needs re-draw.
#define PGUI_FLAG_DIRTY    0x00000004

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
// GUI element flag: Do not add padding.
#define PGUI_FLAG_NOPADDING    0x00002000
// GUI element flag: Ignore minimum size specification.
#define PGUI_FLAG_ANYSIZE      0x00004000
// GUI element flag: Fixed width.
#define PGUI_FLAG_FIX_WIDTH    0x00008000
// GUI element flag: Fixed height.
#define PGUI_FLAG_FIX_HEIGHT   0x00010000

// GUI attribute: Type is selectable.
#define PGUI_ATTR_SELECTABLE 0x00000001
// GUI attribute: Type can have scrollbars.
// Elements with this type won't have their content size set to their actual size.
#define PGUI_ATTR_SCROLLABLE 0x00000002
// GUI attribute: Absolute child position.
// Elements with this type use absolute coordinates for their immediate children.
#define PGUI_ATTR_ABSPOS     0x00000004
// GUI attribute: Element can have children.
// Elements with this type are allowed to contain child elements.
#define PGUI_ATTR_CONTAINER  0x00000008
// GUI attribute: Type uses `pgui_text_t` struct or a superset.
// Enables text element API functions.
#define PGUI_ATTR_TEXTSTRUCT 0x00000010
// GUI attribute: Type describes a label.
// Default colors, label minimum size.
#define PGUI_ATTR_TEXT       0x00000100
// GUI attribute: Type describes a button.
// Button colors, input minimum size.
#define PGUI_ATTR_BUTTON     0x00000200
// GUI attribute: Type describes an input.
// Input colors, input minimum size.
#define PGUI_ATTR_INPUT      0x00000400
// GUI attribute: Type describes a dropdown.
// Dropdown colors, input minimum size.
#define PGUI_ATTR_DROPDOWN   0x00000800

// Base GUI element.
typedef struct pgui_elem pgui_elem_t;

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
// Additional delete callback.
typedef void (*pgui_del_fn_t)(pgui_elem_t *elem);
// GUI button press / input changed callback.
typedef void (*pgui_callback_t)(pgui_elem_t *elem);



/* ==== GUI rendering functions ==== */

// Recalculate the position of a GUI element and its children.
void        pgui_calc_layout(pax_vec2i gfx_size, pgui_elem_t *elem, pgui_theme_t const *theme);
// Draw a GUI element and its children.
void        pgui_draw(pax_buf_t *gfx, pgui_elem_t *elem, pgui_theme_t const *theme);
// Re-draw dirty parts of the GUI and mark the elements clean.
void        pgui_redraw(pax_buf_t *gfx, pgui_elem_t *elem, pgui_theme_t const *theme);
// Handle a button event.
// Returns if and how the event was handled.
pgui_resp_t pgui_event(pax_vec2i gfx_size, pgui_elem_t *elem, pgui_theme_t const *theme, pgui_event_t event);



/* ==== Element management functions ==== */

// Create a new button.
pgui_elem_t *pgui_new_button(char const *text, pgui_callback_t cb);
// Create a new label.
pgui_elem_t *pgui_new_text(char const *text);
// Create a new editable textbox.
pgui_elem_t *pgui_new_textbox(pgui_callback_t cb);
// Create a new grid / table.
pgui_elem_t *pgui_new_grid(pax_vec2i num_cells);
// Create a new dropdown.
pgui_elem_t *pgui_new_dropdown(pgui_callback_t cb);
// Create a new image.
pgui_elem_t *pgui_new_image(pax_buf_t *image, bool do_free_image);
// Delete an element.
void         pgui_delete(pgui_elem_t *elem);
// Delete an element and all its children recursively.
void         pgui_delete_recursive(pgui_elem_t *elem);

// Create a new grid / table.
static inline pgui_elem_t *pgui_new_grid2(int num_cells_x, int num_cells_y) {
    return pgui_new_grid((pax_vec2i){num_cells_x, num_cells_y});
}

// Set element custom user data.
void            pgui_set_userdata(pgui_elem_t *elem, void *userdata);
// Get element custom user data.
void           *pgui_get_userdata(pgui_elem_t *elem);
// Set element on change / on press callback.
void            pgui_set_callback(pgui_elem_t *elem, pgui_callback_t cb);
// Get element on change / on press callback.
pgui_callback_t pgui_get_callback(pgui_elem_t *elem);

// Change the text of a button, label or textbox.
void        pgui_set_text(pgui_elem_t *elem, char const *new_label);
// Get the txt of a button, label or textbox.
// Take care not to edit in the textbox while still using this value.
char const *pgui_get_text(pgui_elem_t *elem);
// Set the horizontal alignment of a button, label or textbox.
void        pgui_set_halign(pgui_elem_t *elem, pax_align_t align);
// Get the horizontal alignment of a button, label or textbox.
pax_align_t pgui_get_halign(pgui_elem_t *elem);
// Set the vertical alignment of a button, label or textbox.
void        pgui_set_valign(pgui_elem_t *elem, pax_align_t align);
// Get the vertical alignment of a button, label or textbox.
pax_align_t pgui_get_valign(pgui_elem_t *elem);
// Change the selection index of a grid or dropdown.
// Negative values indicate no selection and aren't applicable to dropdowns.
void        pgui_set_selection(pgui_elem_t *elem, ptrdiff_t selection);
// Get the selection index of a grid or dropdown.
// Negative values indicate no selection and aren't applicable to dropdowns.
ptrdiff_t   pgui_get_selection(pgui_elem_t *elem);

// Print GUI debug information.
void pgui_print_debug_info(pgui_elem_t *elem);
// Print GUI debug information for element and all children.
void pgui_print_debug_info_recursive(pgui_elem_t *elem);



/* ==== GUI composition functions ==== */

// Append a child to a container element.
bool         pgui_child_append(pgui_elem_t *parent, pgui_elem_t *child);
// Insert a child element at a specific index, shifting siblings after it.
bool         pgui_child_insert(pgui_elem_t *parent, ptrdiff_t index, pgui_elem_t *child);
// Insert a child element at a specific index, replacing the element in that place.
pgui_elem_t *pgui_child_replace(pgui_elem_t *parent, ptrdiff_t index, pgui_elem_t *child);
// Remove a child element by reference.
bool         pgui_child_remove_p(pgui_elem_t *parent, pgui_elem_t *child);
// Remove a child element by index.
pgui_elem_t *pgui_child_remove_i(pgui_elem_t *parent, ptrdiff_t index);
// Get a child element by index.
pgui_elem_t *pgui_child_get(pgui_elem_t *parent, ptrdiff_t index);

// Override element flags.
void      pgui_elem_set_flags(pgui_elem_t *elem, uint32_t flags);
// Get element flags.
uint32_t  pgui_elem_get_flags(pgui_elem_t *elem);
// Override element size.
void      pgui_elem_set_size(pgui_elem_t *elem, pax_vec2i size);
// Get element size.
pax_vec2i pgui_elem_get_size(pgui_elem_t *elem);
// Override element position.
void      pgui_elem_set_pos(pgui_elem_t *elem, pax_vec2i position);
// Get element position.
pax_vec2i pgui_elem_get_pos(pgui_elem_t *elem);

// Override element size.
static inline void pgui_elem_set_size2(pgui_elem_t *elem, int size_x, int size_y) {
    pgui_elem_set_size(elem, (pax_vec2i){size_x, size_y});
}
// Override element position.
static inline void pgui_elem_set_pos2(pgui_elem_t *elem, int position_x, int position_y) {
    pgui_elem_set_pos(elem, (pax_vec2i){position_x, position_y});
}

#ifdef __cplusplus
} // extern "C"
#endif //__cplusplus

#endif // PAX_GUI_TYPES_H
