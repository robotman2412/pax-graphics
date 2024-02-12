
// SPDX-License-Identifier: MIT

#include "pax_internal.h"

#include <string.h>

/* ======== SHADED DRAWING ======= */

// Multi-core method for shaded triangles.
// If odd_scanline is true, the odd (counted from 0) lines are drawn, otherwise the even lines are drawn.
#define PDHG_NAME paxmcr_tri_shaded1
#define PDHG_SHADED
#define PDHG_STATIC
#define PDHG_IGNORE_UV
#define PDHG_MCR
#include "pax_dh_generic_tri.hpp"

// Multi-core method for shaded triangles.
// If odd_scanline is true, the odd (counted from 0) lines are drawn, otherwise the even lines are drawn.
#define PDHG_NAME paxmcr_tri_shaded0
#define PDHG_SHADED
#define PDHG_STATIC
#define PDHG_MCR
#include "pax_dh_generic_tri.hpp"

// Multi-core method for shaded triangles.
// If odd_scanline is true, the odd (counted from 0) lines are drawn, otherwise the even lines are drawn.
void paxmcr_tri_shaded(
    bool                odd_scanline,
    pax_buf_t          *buf,
    pax_col_t           color,
    pax_shader_t const *shader,
    float               x0,
    float               y0,
    float               x1,
    float               y1,
    float               x2,
    float               y2,
    float               u0,
    float               v0,
    float               u1,
    float               v1,
    float               u2,
    float               v2
) {
    if (shader->promise_callback) {
        uint32_t promises = (*(pax_promise_func_t)shader->promise_callback)(buf, color, shader->callback_args);
        if (promises & PAX_PROMISE_INVISIBLE)
            return;
        if (promises & PAX_PROMISE_IGNORE_UVS) {
            paxmcr_tri_shaded1(odd_scanline, buf, color, shader, x0, y0, x1, y1, x2, y2);
            return;
        }
    }
    paxmcr_tri_shaded0(odd_scanline, buf, color, shader, x0, y0, x1, y1, x2, y2, u0, v0, u1, v1, u2, v2);
}

// Multi-core optimisation which maps a buffer directly onto another.
// If odd_scanline is true, the odd (counted from 0) lines are drawn, otherwise the even lines are drawn.
void paxmcr_overlay_buffer(
    bool odd_scanline, pax_buf_t *base, pax_buf_t *top, int x, int y, int width, int height, bool assume_opaque
) {
    int tex_x = 0, tex_y = 0;

    // Perform clipping.
    if (x < base->clip.x) {
        tex_x  = base->clip.x - x;
        width -= tex_x;
        x      = base->clip.x;
    }
    if (x + width > base->clip.x + base->clip.w) {
        width = base->clip.x + base->clip.w - x;
    }
    if (y < base->clip.y) {
        tex_y   = base->clip.y - y;
        height -= tex_y;
        y       = base->clip.y;
    }
    if (y + height > base->clip.y + base->clip.h) {
        height = base->clip.y + base->clip.h - y;
    }

    bool equal = top->type == base->type;
    if (equal && x == 0 && y == 0 && width == base->width && height == base->height &&
        base->reverse_endianness == top->reverse_endianness) {
        // When copying one buffer onto another as a background,
        // and the types are the same, perform a memcpy() instead.
        // memcpy(base->buf, top->buf, (PAX_GET_BPP(base->type) * width * height + 7) >> 3);
        // return;
    }
    // Fix Y co-ordinates.
    if ((y & 1) != odd_scanline) {
        y++;
        tex_y++;
    }

    // Check alpha channel presence.
    if (!PAX_IS_ALPHA(top->type)) {
        assume_opaque = true;
    }

    // Now, let us MAP.
    int top_delta  = tex_y * top->width;
    int base_delta = y * base->width;
    if (assume_opaque) {
        if (equal) {
            // Equal types and alpha.
            for (int c_y = odd_scanline; c_y < height; c_y += 2) {
                for (int c_x = 0; c_x < width; c_x++) {
                    pax_col_t col = top->getter(top, tex_x + top_delta);
                    base->setter(base, col, x + base_delta);
                    tex_x++;
                    x++;
                }
                tex_x      -= width;
                x          -= width;
                top_delta  += 2 * top->width;
                base_delta += 2 * base->width;
            }
        } else {
            // Not equal types, but no alpha.
            for (int c_y = odd_scanline; c_y < height; c_y += 2) {
                for (int c_x = 0; c_x < width; c_x++) {
                    pax_col_t col = top->buf2col(top, top->getter(top, tex_x + top_delta));
                    base->setter(base, base->col2buf(base, col), x + base_delta);
                    tex_x++;
                    x++;
                }
                tex_x      -= width;
                x          -= width;
                top_delta  += 2 * top->width;
                base_delta += 2 * base->width;
            }
        }
    } else {
        // With alpha.
        for (int c_y = odd_scanline; c_y < height; c_y += 2) {
            for (int c_x = 0; c_x < width; c_x++) {
                pax_col_t col = top->buf2col(top, top->getter(top, tex_x + top_delta));
                pax_merge_index(base, col, x + base_delta);
                tex_x++;
                x++;
            }
            tex_x      -= width;
            x          -= width;
            top_delta  += 2 * top->width;
            base_delta += 2 * base->width;
        }
    }
}

// Multi-core optimisation which does not have UVs.
// If odd_scanline is true, the odd (counted from 0) lines are drawn, otherwise the even lines are drawn.
#define PDHG_NAME paxmcr_rect_shaded2
#define PDHG_SHADED
#define PDHG_IGNORE_UV
#define PDHG_MCR
#define PDHG_STATIC
#include "pax_dh_generic_rect.hpp"

// Multi-core optimisation which makes more assumptions about UVs.
// If odd_scanline is true, the odd (counted from 0) lines are drawn, otherwise the even lines are drawn.
#define PDHG_NAME paxmcr_rect_shaded1
#define PDHG_SHADED
#define PDHG_RESTRICT_UV
#define PDHG_MCR
#define PDHG_STATIC
#include "pax_dh_generic_rect.hpp"

// Multi-core method for shaded rects.
// If odd_scanline is true, the odd (counted from 0) lines are drawn, otherwise the even lines are drawn.
#define PDHG_NAME paxmcr_rect_shaded0
#define PDHG_SHADED
#define PDHG_MCR
#define PDHG_STATIC
#include "pax_dh_generic_rect.hpp"

// Multi-core method for shaded rects.
// If odd_scanline is true, the odd (counted from 0) lines are drawn, otherwise the even lines are drawn.
void paxmcr_rect_shaded(
    bool                odd_scanline,
    pax_buf_t          *buf,
    pax_col_t           color,
    pax_shader_t const *shader,
    float               x,
    float               y,
    float               width,
    float               height,
    float               u0,
    float               v0,
    float               u1,
    float               v1,
    float               u2,
    float               v2,
    float               u3,
    float               v3
) {

    pax_promise_func_t fn      = (pax_promise_func_t)shader->promise_callback;
    uint32_t           promise = fn ? fn(buf, color, shader->callback_args) : 0;

    if (promise & PAX_PROMISE_IGNORE_UVS) {
        // Ignore UVs.
        paxmcr_rect_shaded2(odd_scanline, buf, color, shader, x, y, width, height);
        return;
    }

    bool is_default_uv = u0 == 0 && v0 == 0 && u1 == 1 && v1 == 0 && u2 == 1 && v2 == 1 && u3 == 0 && v3 == 1;

    if ((shader->callback == pax_shader_texture || shader->callback == pax_shader_texture_aa) && color == 0xffffffff) {
        // Use a more direct copying of textures.
        pax_buf_t *top = (pax_buf_t *)shader->callback_args;
        if (is_default_uv && roundf(width) == top->width && roundf(height) == top->height) {
            paxmcr_overlay_buffer(
                odd_scanline,
                buf,
                top,
                x + 0.5,
                y + 0.5,
                width + 0.5,
                height + 0.5,
                shader->alpha_promise_255
            );
            return;
        }
    } else if (is_default_uv || (v0 == v1 && v2 == v3 && u0 == u3 && u1 == u2)) {
        // Make some assumptions about UVs.
        paxmcr_rect_shaded1(odd_scanline, buf, color, shader, x, y, width, height, u0, v0, u2, v2);
        return;
    }

    // Use the more expensive generic implementation.
    paxmcr_rect_shaded0(odd_scanline, buf, color, shader, x, y, width, height, u0, v0, u1, v1, u2, v2, u3, v3);
}
