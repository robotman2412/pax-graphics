
// SPDX-License-Identifier: MIT

// clang-format off

#include "helpers/pax_drawing_helpers.h"
#include "pax_internal.h"

#include <string.h>



/* ======== TRAPEZOIDS ======= */

// Multi-core method for shaded trapezoids.
// Used internally for triangles and quads.
#define PDHG_NAME pax_tzoid_shaded_nouv
#define PDHG_SHADED
#define PDHG_STATIC
#define PDHG_IGNORE_UV
#include "helpers/pax_dh_generic_tzoid.inc"

// Multi-core method for shaded trapezoids.
// Used internally for triangles and quads.
#define PDHG_NAME pax_tzoid_shaded_uv
#define PDHG_SHADED
#define PDHG_STATIC
#include "helpers/pax_dh_generic_tzoid.inc"



/* ======== TRIANGLES ======== */

// Internal method for shaded triangles.
// Has no UV co-ordinates.
#define PDHG_NAME pax_tri_shaded_nouv
#define PDHG_SHADED
#define PDHG_STATIC
#define PDHG_IGNORE_UV
#define PDHG_TZOID_NAME pax_tzoid_shaded_nouv
#include "helpers/pax_dh_generic_tri.inc"

// Internal method for shaded triangles.
#define PDHG_NAME pax_tri_shaded_uv
#define PDHG_SHADED
#define PDHG_STATIC
#define PDHG_TZOID_NAME pax_tzoid_shaded_uv
#include "helpers/pax_dh_generic_tri.inc"

// Internal method for shaded triangles.
void pax_tri_shaded(
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
            pax_tri_shaded_nouv(buf, color, shader, x0, y0, x1, y1, x2, y2);
            return;
        }
    }
    pax_tri_shaded_uv(buf, color, shader, x0, y0, x1, y1, x2, y2, u0, v0, u1, v1, u2, v2);
}



/* ========== QUADS ========== */

// Internal method for shaded quads.
#define PDHG_NAME pax_quad_shaded_nouv
#define PDHG_SHADED
#define PDHG_IGNORE_UV
#define PDHG_STATIC
#define PDHG_TZOID_NAME pax_tzoid_shaded_nouv
#include "helpers/pax_dh_generic_quad.inc"

// Internal method for shaded quads.
#define PDHG_NAME pax_quad_shaded_uv
#define PDHG_SHADED
#define PDHG_STATIC
#define PDHG_TZOID_NAME pax_tzoid_shaded_uv
#include "helpers/pax_dh_generic_quad.inc"

// Internal method for shaded quads.
void pax_quad_shaded(
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
            pax_quad_shaded_nouv(buf, color, shader, x0, y0, x1, y1, x2, y2, x3, y3);
            return;
        }
    }
    pax_quad_shaded_uv(buf, color, shader, x0, y0, x1, y1, x2, y2, x3, y3, u0, v0, u1, v1, u2, v2, u3, v3);
}



/* ======= RECTANGLES ======== */

// Optimisation which has no UVs.
#define PDHG_SHADED
#define PDHG_IGNORE_UV
#define PDHG_STATIC
#define PDHG_NAME pax_rect_shaded_nouv
#include "helpers/pax_dh_generic_rect.inc"

// Optimisation which makes more assumptions about UVs.
#define PDHG_SHADED
#define PDHG_RESTRICT_UV
#define PDHG_STATIC
#define PDHG_NAME pax_rect_shaded_resuv
#include "helpers/pax_dh_generic_rect.inc"

// Internal method for shaded rects.
#define PDHG_SHADED
#define PDHG_STATIC
#define PDHG_NAME pax_rect_shaded_uv
#include "helpers/pax_dh_generic_rect.inc"

// Internal method for shaded rects.
void pax_rect_shaded(
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
        pax_rect_shaded_nouv(buf, color, shader, x, y, width, height);
        return;
    }

    bool is_default_uv = u0 == 0 && v0 == 0 && u1 == 1 && v1 == 0 && u2 == 1 && v2 == 1 && u3 == 0 && v3 == 1;

    if (is_default_uv || (v0 == v1 && v2 == v3 && u0 == u3 && u1 == u2)) {
        // Make some assumptions about UVs.
        pax_rect_shaded_resuv(buf, color, shader, x, y, width, height, u0, v0, u2, v2);
        return;
    }

    // Use the more expensive generic implementation.
    pax_rect_shaded_uv(buf, color, shader, x, y, width, height, u0, v0, u1, v1, u2, v2, u3, v3);
}



/* ========== LINES ========== */

// Internal method for shaded lines.
#define PDHG_NAME pax_line_shaded
#define PDHG_SHADED
#include "helpers/pax_dh_generic_line.inc"

// Internal method for line drawing.
void pax_line_shaded_old(
    pax_buf_t          *buf,
    pax_col_t           color,
    pax_shader_t const *shader,
    float x0, float y0, float x1, float y1,
    float u0, float v0, float u1, float v1
) {

    pax_shader_ctx_t shader_ctx = pax_get_shader_ctx(buf, color, shader);
    if (shader_ctx.skip)
        return;
    pax_col_conv_t buf2col = buf->type_info.fmt_type == PAX_BUF_SUBTYPE_PALETTE
                           ? pax_col_conv_dummy : buf->buf2col;

    // Sort points vertially.
    if (y0 > y1) {
        PAX_SWAP(float, x0, x1)
        PAX_SWAP(float, y0, y1)
        PAX_SWAP(float, u0, u1)
        PAX_SWAP(float, v0, v1)
    }

    // Determine whether the line might fall within the clip rect.
    if (!buf->clip.w || !buf->clip.h)
        return;
    if (y1 < buf->clip.y || y0 > buf->clip.y + buf->clip.h - 1)
        return;
    if (x0 == x1 && (x0 < buf->clip.x || x0 > buf->clip.x + buf->clip.w - 1))
        return;
    if (x0 < buf->clip.x && x1 < buf->clip.x)
        return;
    if (x0 > buf->clip.x + buf->clip.w - 1 && x1 > buf->clip.x + buf->clip.w - 1)
        return;

    // Clip top.
    if (y0 < buf->clip.y) {
        float coeff = (buf->clip.y - y0) / (y1 - y0);
        u0          = u0 + (u1 - u0) * coeff;
        v0          = v0 + (v1 - v0) * coeff;
        x0          = x0 + (x1 - x0) * coeff;
        y0          = buf->clip.y;
    }
    // Clip bottom.
    if (y1 > buf->clip.y + buf->clip.h - 1) {
        float coeff = (buf->clip.y + buf->clip.h - 1 - y0) / (y1 - y0);
        u1          = u0 + (u1 - u0) * coeff;
        v1          = v0 + (v1 - v0) * coeff;
        x1          = x0 + (x1 - x0) * coeff;
        y1          = buf->clip.y + buf->clip.h - 1;
    }
    // Clip left.
    if (x1 <= x0 && x1 < buf->clip.x) {
        if (x0 < buf->clip.x)
            return;
        float coeff = (buf->clip.x - x0) / (x1 - x0);
        u1          = u0 + (u1 - u0) * coeff;
        v1          = v0 + (v1 - v0) * coeff;
        y1          = y0 + (y1 - y0) * coeff;
        x1          = buf->clip.x;

    } else if (x0 < x1 && x0 < buf->clip.x) {
        if (x1 < buf->clip.x)
            return;
        float coeff = (buf->clip.x - x0) / (x1 - x0);
        u0          = u0 + (u1 - u0) * coeff;
        v0          = v0 + (v1 - v0) * coeff;
        y0          = y0 + (y1 - y0) * coeff;
        x0          = buf->clip.x;
    }
    // Clip right.
    if (x1 >= x0 && x1 > buf->clip.x + buf->clip.w - 1) {
        if (x0 > buf->clip.x + buf->clip.w - 1)
            return;
        float coeff = (buf->clip.x + buf->clip.w - 1 - x0) / (x1 - x0);
        u1          = u0 + (u1 - u0) * coeff;
        v1          = v0 + (v1 - v0) * coeff;
        y1          = y0 + (y1 - y0) * coeff;
        x1          = buf->clip.x + buf->clip.w - 1;

    } else if (x0 > x1 && x0 > buf->clip.x + buf->clip.w - 1) {
        if (x1 > buf->clip.x + buf->clip.w - 1)
            return;
        float coeff = (buf->clip.x + buf->clip.w - 1 - x0) / (x1 - x0);
        u0          = u0 + (u1 - u0) * coeff;
        v0          = v0 + (v1 - v0) * coeff;
        y0          = y0 + (y1 - y0) * coeff;
        x0          = buf->clip.x + buf->clip.w - 1;
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
        // Horizontal line.
        int index = (int)y0 * buf->width;
        if (dx < 0) {
            PAX_SWAP(float, x0, x1);
            PAX_SWAP(float, u0, u1)
            PAX_SWAP(float, v0, v1)
        }
        nIter   = (int)x1 - (int)x0 + 1;
        float u = u0, v = v0;
        float du = (u1 - u0) / nIter;
        float dv = (v1 - v0) / nIter;
        for (int i = x0; i <= x1; i++) {
            pax_col_t result = (shader_ctx.callback)(
                color,
                shader_ctx.do_getter ? buf2col(buf, buf->getter(buf, index + i)) : 0,
                i,
                y0,
                u,
                v,
                shader_ctx.callback_args
            );
            pax_set_index_conv(buf, result, index + i);
            u += du;
            v += dv;
        }
    } else if (x0 == x1) {
        // Vertical line.
        int index = x0 + (int)y0 * buf->width;
        nIter     = (int)y1 - (int)y0 + 1;
        float u = u0, v = v0;
        float du = (u1 - u0) / nIter;
        float dv = (v1 - v0) / nIter;
        for (int i = y0; i <= y1; i++, index += buf->width) {
            pax_col_t result = (shader_ctx.callback)(
                color,
                shader_ctx.do_getter ? buf2col(buf, buf->getter(buf, index)) : 0,
                x0,
                i,
                u,
                v,
                shader_ctx.callback_args
            );
            pax_set_index_conv(buf, result, index);
            u += du;
            v += dv;
        }
    } else {
        // Any other line.
        float        du  = (u1 - u0) / nIter;
        float        dv  = (v1 - v0) / nIter;
        int_fast32_t x   = x0 * 0x10000 + 0x08000;
        int_fast32_t y   = y0 * 0x10000 + 0x08000;
        int_fast32_t idx = dx * 0x10000;
        int_fast32_t idy = dy * 0x10000;
        int_fast32_t u   = u0 * 0x10000;
        int_fast32_t v   = v0 * 0x10000;
        int_fast32_t idu = du * 0x10000;
        int_fast32_t idv = dv * 0x10000;
        for (int i = 0; i <= nIter; i++) {
            size_t    delta  = (x >> 16) + (y >> 16) * buf->width;
            pax_col_t result = (shader_ctx.callback)(
                color,
                shader_ctx.do_getter ? buf2col(buf, buf->getter(buf, delta)) : 0,
                x >> 16,
                y >> 16,
                u / (float)0x10000,
                v / (float)0x10000,
                shader_ctx.callback_args
            );
            pax_set_index_conv(buf, result, delta);
            x += idx;
            y += idy;
            u += idu;
            v += idv;
        }
    }
}
