
// SPDX-License-Identifier: MIT

// clang-format off

#include "pax_internal.h"
#include "helpers/pax_drawing_helpers.h"

#include <string.h>



/* ======== TRAPEZOIDS ======= */

// Multi-core method for shaded trapezoids.
// Used internally for triangles and quads.
#define PDHG_NAME paxmcr_tzoid_shaded_nouv
#define PDHG_SHADED
#define PDHG_STATIC
#define PDHG_IGNORE_UV
#define PDHG_MCR
#include "helpers/pax_dh_generic_tzoid.inc"

// Multi-core method for shaded trapezoids.
// Used internally for triangles and quads.
#define PDHG_NAME paxmcr_tzoid_shaded_uv
#define PDHG_SHADED
#define PDHG_STATIC
#define PDHG_MCR
#include "helpers/pax_dh_generic_tzoid.inc"



/* ======== TRIANGLES ======== */

// Multi-core method for shaded triangles.
#define PDHG_NAME paxmcr_tri_shaded_nouv
#define PDHG_SHADED
#define PDHG_STATIC
#define PDHG_IGNORE_UV
#define PDHG_MCR
#define PDHG_TZOID_NAME paxmcr_tzoid_shaded_nouv
#include "helpers/pax_dh_generic_tri.inc"

// Multi-core method for shaded triangles.
#define PDHG_NAME paxmcr_tri_shaded_uv
#define PDHG_SHADED
#define PDHG_STATIC
#define PDHG_MCR
#define PDHG_TZOID_NAME paxmcr_tzoid_shaded_uv
#include "helpers/pax_dh_generic_tri.inc"

// Multi-core method for shaded triangles.
// If odd_scanline is true, the odd (counted from 0) lines are drawn, otherwise the even lines are drawn.
void paxmcr_tri_shaded(
    bool                odd_scanline,
    pax_buf_t          *buf,
    pax_col_t           color,
    pax_shader_t const *shader,
    float x0, float y0, float x1, float y1, float x2, float y2,
    float u0, float v0, float u1, float v1, float u2, float v2
) {
    if (shader->promise_callback) {
        uint32_t promises = (*(pax_promise_func_t)shader->promise_callback)(buf, color, shader->callback_args);
        if (promises & PAX_PROMISE_INVISIBLE)
            return;
        if (promises & PAX_PROMISE_IGNORE_UVS) {
            paxmcr_tri_shaded_nouv(odd_scanline, buf, color, shader, x0, y0, x1, y1, x2, y2);
            return;
        }
    }
    paxmcr_tri_shaded_uv(odd_scanline, buf, color, shader, x0, y0, x1, y1, x2, y2, u0, v0, u1, v1, u2, v2);
}



/* ========== QUADS ========== */

// Multi-core methods for shaded quads.
// If odd_scanline is true, the odd (counted from 0) lines are drawn, otherwise the even lines are drawn.
#define PDHG_NAME paxmcr_quad_shaded_nouv
#define PDHG_SHADED
#define PDHG_IGNORE_UV
#define PDHG_MCR
#define PDHG_STATIC
#define PDHG_TZOID_NAME paxmcr_tzoid_shaded_nouv
#include "helpers/pax_dh_generic_quad.inc"

// Multi-core methods for shaded quads.
// If odd_scanline is true, the odd (counted from 0) lines are drawn, otherwise the even lines are drawn.
#define PDHG_NAME paxmcr_quad_shaded_uv
#define PDHG_SHADED
#define PDHG_MCR
#define PDHG_STATIC
#define PDHG_TZOID_NAME paxmcr_tzoid_shaded_uv
#include "helpers/pax_dh_generic_quad.inc"

// Multi-core method for shaded triangles.
// If odd_scanline is true, the odd (counted from 0) lines are drawn, otherwise the even lines are drawn.
void paxmcr_quad_shaded(
    bool                odd_scanline,
    pax_buf_t          *buf,
    pax_col_t           color,
    pax_shader_t const *shader,
    float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3,
    float u0, float v0, float u1, float v1, float u2, float v2, float u3, float v3
) {
    if (shader->promise_callback) {
        uint32_t promises = (*(pax_promise_func_t)shader->promise_callback)(buf, color, shader->callback_args);
        if (promises & PAX_PROMISE_INVISIBLE)
            return;
        if (promises & PAX_PROMISE_IGNORE_UVS) {
            paxmcr_quad_shaded_nouv(odd_scanline, buf, color, shader, x0, y0, x1, y1, x2, y2, x3, y3);
            return;
        }
    }
    paxmcr_quad_shaded_uv(odd_scanline, buf, color, shader, x0, y0, x1, y1, x2, y2, x3, y3, u0, v0, u1, v1, u2, v2, u3, v3);
}



/* ======= RECTANGLES ======== */

// Multi-core optimisation which does not have UVs.
#define PDHG_NAME paxmcr_rect_shaded_nouv
#define PDHG_SHADED
#define PDHG_IGNORE_UV
#define PDHG_MCR
#define PDHG_STATIC
#include "helpers/pax_dh_generic_rect.inc"

// Multi-core optimisation which makes more assumptions about UVs.
#define PDHG_NAME paxmcr_rect_shaded_resuv
#define PDHG_SHADED
#define PDHG_RESTRICT_UV
#define PDHG_MCR
#define PDHG_STATIC
#include "helpers/pax_dh_generic_rect.inc"

// Multi-core method for shaded rects.
#define PDHG_NAME paxmcr_rect_shaded_uv
#define PDHG_SHADED
#define PDHG_MCR
#define PDHG_STATIC
#include "helpers/pax_dh_generic_rect.inc"

// Multi-core method for shaded rects.
// Defers to optimized methods if possible.
// If odd_scanline is true, the odd (counted from 0) lines are drawn, otherwise the even lines are drawn.
void paxmcr_rect_shaded(
    bool                odd_scanline,
    pax_buf_t          *buf,
    pax_col_t           color,
    pax_shader_t const *shader,
    float x, float y, float width, float height,
    float u0, float v0, float u1, float v1, float u2, float v2, float u3, float v3
) {

    pax_promise_func_t fn      = (pax_promise_func_t)shader->promise_callback;
    uint32_t           promise = fn ? fn(buf, color, shader->callback_args) : 0;

    if (promise & PAX_PROMISE_IGNORE_UVS) {
        // Ignore UVs.
        paxmcr_rect_shaded_nouv(odd_scanline, buf, color, shader, x, y, width, height);
        return;
    }

    bool is_default_uv = u0 == 0 && v0 == 0 && u1 == 1 && v1 == 0 && u2 == 1 && v2 == 1 && u3 == 0 && v3 == 1;

    if (is_default_uv || (v0 == v1 && v2 == v3 && u0 == u3 && u1 == u2)) {
        // Make some assumptions about UVs.
        paxmcr_rect_shaded_resuv(odd_scanline, buf, color, shader, x, y, width, height, u0, v0, u2, v2);
        return;
    }

    // Use the more expensive generic implementation.
    paxmcr_rect_shaded_uv(odd_scanline, buf, color, shader, x, y, width, height, u0, v0, u1, v1, u2, v2, u3, v3);
}



/* ========== LINES ========== */

// Multi-core method for shaded lines.
// If odd_scanline is true, the odd (counted from 0) lines are drawn, otherwise the even lines are drawn.
#define PDHG_NAME paxmcr_line_shaded
#define PDHG_SHADED
#define PDHG_MCR
#include "helpers/pax_dh_generic_line.inc"
