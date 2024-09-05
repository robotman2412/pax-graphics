
// SPDX-License-Identifier: MIT

#include "pax_gui_internal.h"
#include "pax_gui_util.h"



// Create a new overlay.
pgui_elem_t *pgui_new_box() {
    pgui_elem_t *elem = calloc(1, sizeof(pgui_elem_t));
    if (!elem)
        return NULL;
    elem->type     = &pgui_type_overlay;
    elem->flags    = PGUI_FLAG_FIX_WIDTH | PGUI_FLAG_FIX_HEIGHT | PGUI_FLAG_NOPADDING;
    elem->selected = -1;
    return elem;
}

// Overlay element type.
pgui_type_t const pgui_type_box = {
    .id          = PGUI_TYPE_ID_BOX,
    .base_struct = PGUI_STRUCT_BASE,
    .name        = "box",
    .attr        = 0,
};
