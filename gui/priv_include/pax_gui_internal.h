
// SPDX-License-Identifier: MIT

#ifndef PAX_GUI_INTERNAL_H
#define PAX_GUI_INTERNAL_H

#include "pax_gui.h"

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus



// Data for grid elements.
typedef struct pgui_grid     pgui_grid_t;
// Data for dropdown elements.
typedef struct pgui_dropdown pgui_dropdown_t;
// Data for text-based elements.
typedef struct pgui_text     pgui_text_t;
// Data for image elements.
typedef struct pgui_image    pgui_image_t;

// Type base struct ID.
typedef enum {
    // Uses `pgui_elem_t` directly.
    PGUI_STRUCT_BASE,
    // Uses `pgui_grid_t`.
    PGUI_STRUCT_GRID,
    // Uses `pgui_dropdown_t`.
    PGUI_STRUCT_DROPDOWN,
    // Uses `pgui_text_t`.
    PGUI_STRUCT_TEXT,
    // Uses `pgui_image_t`.
    PGUI_STRUCT_IMAGE,
} pgui_struct_id_t;



struct pgui_type {
    // Type ID.
    pgui_type_id_t   id;
    // Type base struct.
    pgui_struct_id_t base_struct;
    // Extra size allocated to custom types.
    size_t           custom_struct_size;
    // Type name for debug purposes.
    char const      *name;
    // Static element attributes.
    uint32_t         attr;
    // Set clip rectangle for children.
    pgui_draw_fn_t   clip;
    // Draw call (mandatory).
    pgui_draw_fn_t   draw;
    // Minimum element size calculation call.
    pgui_calc_fn_t   calc1;
    // Internal layout calculation call.
    pgui_calc_fn_t   calc2;
    // Event call.
    pgui_event_fn_t  event;
    // Child list changed callback.
    pgui_callback_t  child;
    // Additional delete callback.
    pgui_del_fn_t    del;
    // Additional delete callback for custom types.
    pgui_del_fn_t    del2;
};

struct pgui_elem {
    // Element type.
    pgui_type_t const *type;
    // Element flags.
    // Effects of inheritable flags are applied to child elements.
    uint32_t           flags;
    // Element palette variation.
    pgui_variant_t     variant;
    // Parent element, set automatically.
    pgui_elem_t       *parent;
    // Theme and property overrides.
    pgui_override_t   *overrides;

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

struct pgui_grid {
    // Common element data.
    pgui_elem_t base;
    // Size in grid cells.
    pax_vec2i   cells;
    // Per-row size.
    int        *row_height;
    // Per-column size.
    int        *col_width;
    // Which rows are allowed to resize.
    bool       *row_resizable;
    // Which columns are allowed to resize.
    bool       *col_resizable;
};

struct pgui_dropdown {
    // Common element data.
    pgui_elem_t base;
    // Last on-screen position.
    pax_vec2i   last_pos;
    // Selected child index.
    size_t      selected;
    // On-screen position of child elements.
    pax_recti   child_pos;
};

struct pgui_image {
    // Common element data.
    pgui_elem_t base;
    // Image buffer.
    pax_buf_t  *image;
    // Whether to delete the image buffer when the element is deleted.
    bool        do_free_image;
};



// Extra init function for grid struct based custom types.
bool pgui_grid_custominit(pgui_grid_t *grid);

// Child clipping rectangle for dropdowns.
void pgui_clip_dropdown(pax_buf_t *gfx, pax_vec2i pos, pgui_elem_t *elem, pgui_theme_t const *theme, uint32_t flags);

// Visuals for text-based elements.
void pgui_draw_text(pax_buf_t *gfx, pax_vec2i pos, pgui_elem_t *elem, pgui_theme_t const *theme, uint32_t flags);
// Visuals for editable text-based elements.
void pgui_draw_textbox(pax_buf_t *gfx, pax_vec2i pos, pgui_elem_t *elem, pgui_theme_t const *theme, uint32_t flags);
// Visuals for grid-based elements.
void pgui_draw_grid(pax_buf_t *gfx, pax_vec2i pos, pgui_elem_t *elem, pgui_theme_t const *theme, uint32_t flags);
// Visuals for dropdowns.
void pgui_draw_dropdown(pax_buf_t *gfx, pax_vec2i pos, pgui_elem_t *elem, pgui_theme_t const *theme, uint32_t flags);
// Visuals for image elements.
void pgui_draw_image(pax_buf_t *gfx, pax_vec2i pos, pgui_elem_t *elem, pgui_theme_t const *theme, uint32_t flags);

// Calculate the minimum size of button elements.
void pgui_calc1_button(pax_vec2i gfx_size, pax_vec2i pos, pgui_elem_t *elem, pgui_theme_t const *theme, uint32_t flags);
// Calculate the minimum size of text-based elements.
void pgui_calc1_text(pax_vec2i gfx_size, pax_vec2i pos, pgui_elem_t *elem, pgui_theme_t const *theme, uint32_t flags);
// Calculate the minimum size of a grid.
void pgui_calc1_grid(pax_vec2i gfx_size, pax_vec2i pos, pgui_elem_t *elem, pgui_theme_t const *theme, uint32_t flags);
// Calculate the minimum size of a dropdown.
void pgui_calc1_dropdown(
    pax_vec2i gfx_size, pax_vec2i pos, pgui_elem_t *elem, pgui_theme_t const *theme, uint32_t flags
);
// Calculate the minimum size of image elements.
void pgui_calc1_image(pax_vec2i gfx_size, pax_vec2i pos, pgui_elem_t *elem, pgui_theme_t const *theme, uint32_t flags);

// Calculate the internal layout of editable text-based elements.
void pgui_calc2_textbox(
    pax_vec2i gfx_size, pax_vec2i pos, pgui_elem_t *elem, pgui_theme_t const *theme, uint32_t flags
);
// Calculate the internal layout of a grid.
void pgui_calc2_grid(pax_vec2i gfx_size, pax_vec2i pos, pgui_elem_t *elem, pgui_theme_t const *theme, uint32_t flags);
// Calculate the internal layout of a dropdown.
void pgui_calc2_dropdown(
    pax_vec2i gfx_size, pax_vec2i pos, pgui_elem_t *elem, pgui_theme_t const *theme, uint32_t flags
);
// Calculate the internal layout of overlay elements.
void pgui_calc2_overlay(
    pax_vec2i gfx_size, pax_vec2i pos, pgui_elem_t *elem, pgui_theme_t const *theme, uint32_t flags
);

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
// Behaviour for dropdown elements.
pgui_resp_t pgui_event_dropdown(
    pax_vec2i gfx_size, pax_vec2i pos, pgui_elem_t *elem, pgui_theme_t const *theme, uint32_t flags, pgui_event_t event
);

// Child list changed callback for grid-based elements.
void pgui_child_grid(pgui_elem_t *elem);
// Child list changed callback for dropdowns.
void pgui_child_dropdown(pgui_elem_t *elem);

// Additional delete function for editable text-based elements.
void pgui_del_text(pgui_elem_t *elem);
// Additional delete function for grid-based elements.
void pgui_del_grid(pgui_elem_t *elem);
// Additional delete function for image elements.
void pgui_del_image(pgui_elem_t *elem);

extern pgui_type_t const pgui_type_button;
extern pgui_type_t const pgui_type_text;
extern pgui_type_t const pgui_type_textbox;
extern pgui_type_t const pgui_type_grid;
extern pgui_type_t const pgui_type_dropdown;
extern pgui_type_t const pgui_type_image;
extern pgui_type_t const pgui_type_overlay;
extern pgui_type_t const pgui_type_box;



#ifdef __cplusplus
} // extern "C"
#endif //__cplusplus

#endif // PAX_GUI_INTERNAL_H
