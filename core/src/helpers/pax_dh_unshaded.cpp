
// SPDX-License-Identifier: MIT

#include "helpers/pax_drawing_helpers.h"
#include "pax_internal.h"

/* ======= UNSHADED DRAWING ====== */

// Internal method for unshaded trapezoids.
// Used internally for triangles and quads.
#define PDHG_NAME pax_tzoid_unshaded
#define PDHG_STATIC
#include "helpers/pax_dh_generic_tzoid.inc"

// Internal method for unshaded triangles.
#define PDHG_NAME       pax_tri_unshaded
#define PDHG_TZOID_NAME pax_tzoid_unshaded
#include "helpers/pax_dh_generic_tri.inc"

// Internal method for unshaded rects.
#define PDHG_NAME pax_rect_unshaded
#include "helpers/pax_dh_generic_rect.inc"

// Internal methods for unshaded quads.
#define PDHG_NAME       pax_quad_unshaded
#define PDHG_TZOID_NAME pax_tzoid_unshaded
#include "helpers/pax_dh_generic_quad.inc"

// Internal method for unshaded lines.
#define PDHG_NAME pax_line_unshaded
#include "helpers/pax_dh_generic_line.inc"

// Internal method for line drawing.
void pax_line_unshaded_old(pax_buf_t *buf, pax_col_t color, float x0, float y0, float x1, float y1) {
    pax_index_setter_t setter = pax_get_setter(buf, &color, NULL);
    if (!setter)
        return;

    if (y1 < y0) {
        PAX_SWAP(float, x0, x1)
        PAX_SWAP(float, y0, y1)
    }

    // Bounds check.
    if ((x0 < 0 && x1 < 0) || (x0 >= buf->clip.x + buf->clip.w && x1 >= buf->clip.x + buf->clip.w) || (y0 < 0 && y1 < 0)
        || (y0 >= buf->clip.y + buf->clip.h && y1 >= buf->clip.y + buf->clip.h)) {
        return;
    }

    // Clip: left.
    if (x0 <= x1 && x0 < buf->clip.x) {
        if (x1 < buf->clip.x)
            return;
        // Adjust X0 against left clip.
        y0 = y0 + (y1 - y0) * (buf->clip.x - x0) / (x1 - x0);
        x0 = buf->clip.x;
    } else if (x1 < buf->clip.x) {
        if (x0 < buf->clip.x)
            return;
        // Adjust X1 against left clip.
        y1 = y1 + (y0 - y1) * (buf->clip.x - x1) / (x0 - x1);
        x1 = buf->clip.x;
    }

    // Clip: right.
    if (x1 >= x0 && x1 > buf->clip.x + buf->clip.w - 1) {
        if (x0 > buf->clip.x + buf->clip.w - 1)
            return;
        // Adjust X1 against right of clip.
        y1 = y0 + (y1 - y0) * (buf->clip.x + buf->clip.w - 1 - x0) / (x1 - x0);
        x1 = buf->clip.x + buf->clip.w - 1;
    } else if (x0 > buf->clip.x + buf->clip.w - 1) {
        if (x1 > buf->clip.x + buf->clip.w - 1)
            return;
        // Adjust X0 against right of clip.
        y0 = y1 + (y0 - y1) * (buf->clip.x + buf->clip.w - 1 - x1) / (x0 - x1);
        x0 = buf->clip.x + buf->clip.w - 1;
    }

    // Clip: top.
    if (y0 < buf->clip.y) {
        if (y1 < buf->clip.y)
            return;
        // Adjust Y0 against top of clip.
        x0 = x0 + (x1 - x0) * (buf->clip.y - y0) / (y1 - y0);
        y0 = buf->clip.y;
    }

    // Clip: bottom.
    if (y1 > buf->clip.y + buf->clip.h - 1) {
        if (y0 > buf->clip.y + buf->clip.h - 1)
            return;
        // Adjust Y1 against bottom of clip.
        x1 = x1 + (x1 - x0) * (buf->clip.y + buf->clip.h - 1 - y1) / (y1 - y0);
        y1 = buf->clip.y + buf->clip.h - 1;
    }

    // Determine whether the line is "steep" (dx*dx > dy*dy).
    float dx       = x1 - x0;
    float dy       = y1 - y0;
    bool  is_steep = fabsf(dx) < fabsf(dy);
    int   nIter;

    // Determine the number of iterations.
    nIter = ceilf(fabsf(is_steep ? dy : dx));
    if (nIter < 1)
        nIter = 1;

    // Adjust dx and dy.
    dx /= nIter;
    dy /= nIter;

    if (y0 == y1) {
        int index = (int)y0 * buf->width;
        if (dx < 0) {
            PAX_SWAP(float, x0, x1);
        }
        for (int i = x0; i <= x1; i++) {
            setter(buf, color, index + i);
        }
    } else if (x0 == x1) {
        int index = x0 + (int)y0 * buf->width;
        for (int i = y0; i <= y1; i++, index += buf->width) {
            setter(buf, color, index);
        }
    } else {
        int_fast32_t x   = x0 * 0x10000 + 0x08000;
        int_fast32_t y   = y0 * 0x10000 + 0x08000;
        int_fast32_t idx = dx * 0x10000;
        int_fast32_t idy = dy * 0x10000;
        for (int i = 0; i <= nIter; i++) {
            setter(buf, color, (x >> 16) + (y >> 16) * buf->width);
            x += idx;
            y += idy;
        }
    }
}
