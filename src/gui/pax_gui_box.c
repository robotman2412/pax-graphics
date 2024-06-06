
// SPDX-License-Identifier: MIT

#include "pax_gui_box.h"

#include "pax_internal.h"



// Box element type.
pgui_type_t pgui_type_box_raw = {
    .attr = PGUI_ATTR_BOX | PGUI_ATTR_SELECTABLE,
    .draw = pgui_draw_base,
};
