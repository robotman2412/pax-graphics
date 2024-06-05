
// SPDX-License-Identifier: MIT

#ifndef PAX_GUI_GRID_H
#define PAX_GUI_GRID_H

#include "pax_gui_box.h"
#include "pax_gui_types.h"

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

// A grid or list of GUI elements.
// Implements arrow-key / DPAD selection logic.
// Can also be used as a list if either `cells.x` or `cells.y` is 1.
typedef struct pgui_grid pgui_grid_t;

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



// Calculate the layout of a grid.
void pgui_calc_grid(pgui_grid_t *elem, pgui_theme_t const *theme);

// Draw a grid.
void pgui_draw_grid(pax_buf_t *gfx, pax_vec2f pos, pgui_grid_t *elem, pgui_theme_t const *theme, uint32_t flags);

// Send an event to a grid.
pgui_resp_t pgui_event_grid(pgui_grid_t *elem, pgui_event_t event, uint32_t flags);

#ifdef __cplusplus
} // extern "C"
#endif //__cplusplus

#endif // PAX_GUI_GRID_H
