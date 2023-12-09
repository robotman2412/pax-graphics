
// SPDX-License-Identifier: MIT

#include "pax_internal.h"

/* ======= UNSHADED DRAWING ====== */

// Multi-core method for unshaded triangles.
// Assumes points are sorted by Y.
// If odd_scanline is true, the odd (counted from 0) lines are drawn, otherwise the even lines are drawn.
#define PDHG_NAME paxmcr_tri_unshaded
#define PDHG_MCR
#include "pax_dh_generic_tri.hpp"

// Multi-core method for rectangle drawing.
// If odd_scanline is true, the odd (counted from 0) lines are drawn, otherwise the even lines are drawn.
#define PDHG_NAME paxmcr_rect_unshaded
#define PDHG_MCR
#include "pax_dh_generic_rect.hpp"
