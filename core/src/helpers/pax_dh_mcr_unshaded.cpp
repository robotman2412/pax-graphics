
// SPDX-License-Identifier: MIT

#include "helpers/pax_drawing_helpers.h"
#include "pax_internal.h"

/* ======= UNSHADED DRAWING ====== */

// Multi-core method for unshaded trapezoids.
// Used internally for triangles and quads.
#define PDHG_NAME paxmcr_tzoid_unshaded
#define PDHG_STATIC
#define PDHG_MCR
#include "helpers/pax_dh_generic_tzoid.inc"

// Multi-core method for unshaded triangles.
// If odd_scanline is true, the odd (counted from 0) lines are drawn, otherwise the even lines are drawn.
#define PDHG_NAME paxmcr_tri_unshaded
#define PDHG_MCR
#define PDHG_TZOID_NAME paxmcr_tzoid_unshaded
#include "helpers/pax_dh_generic_tri.inc"

// Multi-core method for rectangle drawing.
// If odd_scanline is true, the odd (counted from 0) lines are drawn, otherwise the even lines are drawn.
#define PDHG_NAME paxmcr_rect_unshaded
#define PDHG_MCR
#include "helpers/pax_dh_generic_rect.inc"

// Multi-core method for unshaded quads.
// If odd_scanline is true, the odd (counted from 0) lines are drawn, otherwise the even lines are drawn.
#define PDHG_NAME paxmcr_quad_unshaded
#define PDHG_MCR
#define PDHG_TZOID_NAME paxmcr_tzoid_unshaded
#include "helpers/pax_dh_generic_quad.inc"

// Multi-core method for unshaded lines.
// If odd_scanline is true, the odd (counted from 0) lines are drawn, otherwise the even lines are drawn.
#define PDHG_NAME paxmcr_line_unshaded
#define PDHG_MCR
#include "helpers/pax_dh_generic_line.inc"
