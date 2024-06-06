
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

// Grid element type.
extern pgui_type_t pgui_type_grid_raw;
#define PGUI_TYPE_GRID (&pgui_type_grid_raw)

#ifdef __cplusplus
} // extern "C"
#endif //__cplusplus

#endif // PAX_GUI_GRID_H
