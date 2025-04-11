
// SPDX-License-Identifier: MIT

#include "pax_fixpt.hpp"
#include "pax_internal.h"

#ifdef PDHG_NAME

    #if defined(PDHG_SHADED) && !defined(PDHG_RESTRICT_UV) && !defined(PDHG_IGNORE_UV)
        #define PDHG_NORMAL_UV
    #endif

    #ifdef PDHG_MCR
        #define PDHG_INCREMENT 2
    #else
        #define PDHG_INCREMENT 1
    #endif

    #if !defined(PDHG_SHADED) && defined(PDHG_IGNORE_UV)
        #error "Cannot define `PDHG_IGNORE_UV` without `PDHG_SHADED`."
    #endif

    #if !defined(PDHG_SHADED) && defined(PDHG_RESTRICT_UV)
        #error "Cannot define `PDHG_RESTRICT_UV` without `PDHG_SHADED`."
    #endif

    #if defined(PDHG_RESTRICT_UV) && defined(PDHG_IGNORE_UV)
        #error "Cannot define both `PDHG_IGNORE_UV` and `PDHG_RESTRICT_UV`; select at most one."
    #endif

    // Generic rectangle drawing code, assuming some defines are made.
    #ifdef PDHG_STATIC
static
    #endif
    void
    PDHG_NAME(
    #ifdef PDHG_MCR
        bool odd_scanline,
    #endif
        pax_buf_t *buf,
        pax_col_t  color,
    #ifdef PDHG_SHADED
        pax_shader_t const *shader,
    #endif
        float x0,
        float y0,
        float x1,
        float y1
    #ifdef PDHG_NORMAL_UV
        ,
        float u0,
        float v0,
        float u1,
        float v1
    #endif
    ) {

    #ifdef PDHG_SHADED
    // Get shader context.
    pax_shader_ctx_t shader_ctx = pax_get_shader_ctx(buf, color, shader);
    if (shader_ctx.skip)
        return;
        #ifdef PDHG_NORMAL_UV
    pax_col_conv_t buf2col = buf->type_info.fmt_type == PAX_BUF_SUBTYPE_PALETTE ? pax_col_conv_dummy : buf->buf2col;
        #endif // PDHG_NORMAL_UV
    // Get pixel setter.
    pax_index_setter_t setter = pax_get_setter(buf, &color, shader);
    if (!setter)
        return;
    #else // PDHG_SHADED
        #ifdef PDHG_NORMAL_UV
    pax_col_conv_t buf2col = buf->type_info.fmt_type == PAX_BUF_SUBTYPE_PALETTE ? pax_col_conv_dummy : buf->buf2col;
        #endif // PDHG_NORMAL_UV
    // Get pixel setter.
    pax_index_setter_t setter = pax_get_setter(buf, &color, NULL);
    if (!setter)
        return;
    #endif     // PDHG_SHADED

    // Sort points vertially.
    if (y0 > y1) {
        PAX_SWAP(float, x0, x1)
        PAX_SWAP(float, y0, y1)
    #ifdef PDHG_NORMAL_UV
        PAX_SWAP(float, u0, u1)
        PAX_SWAP(float, v0, v1)
    #endif
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
    #ifdef PDHG_NORMAL_UV
        u0 = u0 + (u1 - u0) * coeff;
        v0 = v0 + (v1 - v0) * coeff;
    #endif
        x0 = x0 + (x1 - x0) * coeff;
        y0 = buf->clip.y;
    }
    // Clip bottom.
    if (y1 > buf->clip.y + buf->clip.h - 1) {
        float coeff = (buf->clip.y + buf->clip.h - 1 - y0) / (y1 - y0);
    #ifdef PDHG_NORMAL_UV
        u1 = u0 + (u1 - u0) * coeff;
        v1 = v0 + (v1 - v0) * coeff;
    #endif
        x1 = x0 + (x1 - x0) * coeff;
        y1 = buf->clip.y + buf->clip.h - 1;
    }
    // Clip left.
    if (x1 <= x0 && x1 < buf->clip.x) {
        if (x0 < buf->clip.x)
            return;
        float coeff = (buf->clip.x - x0) / (x1 - x0);
    #ifdef PDHG_NORMAL_UV
        u1 = u0 + (u1 - u0) * coeff;
        v1 = v0 + (v1 - v0) * coeff;
    #endif
        y1 = y0 + (y1 - y0) * coeff;
        x1 = buf->clip.x;

    } else if (x0 < x1 && x0 < buf->clip.x) {
        if (x1 < buf->clip.x)
            return;
        float coeff = (buf->clip.x - x0) / (x1 - x0);
    #ifdef PDHG_NORMAL_UV
        u0 = u0 + (u1 - u0) * coeff;
        v0 = v0 + (v1 - v0) * coeff;
    #endif
        y0 = y0 + (y1 - y0) * coeff;
        x0 = buf->clip.x;
    }
    // Clip right.
    if (x1 >= x0 && x1 > buf->clip.x + buf->clip.w - 1) {
        if (x0 > buf->clip.x + buf->clip.w - 1)
            return;
        float coeff = (buf->clip.x + buf->clip.w - 1 - x0) / (x1 - x0);
    #ifdef PDHG_NORMAL_UV
        u1 = u0 + (u1 - u0) * coeff;
        v1 = v0 + (v1 - v0) * coeff;
    #endif
        y1 = y0 + (y1 - y0) * coeff;
        x1 = buf->clip.x + buf->clip.w - 1;

    } else if (x0 > x1 && x0 > buf->clip.x + buf->clip.w - 1) {
        if (x1 > buf->clip.x + buf->clip.w - 1)
            return;
        float coeff = (buf->clip.x + buf->clip.w - 1 - x0) / (x1 - x0);
    #ifdef PDHG_NORMAL_UV
        u0 = u0 + (u1 - u0) * coeff;
        v0 = v0 + (v1 - v0) * coeff;
    #endif
        y0 = y0 + (y1 - y0) * coeff;
        x0 = buf->clip.x + buf->clip.w - 1;
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
    #ifdef PDHG_MCR
        if (((int)y0 & 1) != odd_scanline) {
            return;
        }
    #endif
        int index = (int)y0 * buf->width;
        if (dx < 0) {
            PAX_SWAP(float, x0, x1)
    #ifdef PDHG_NORMAL_UV
            PAX_SWAP(float, u0, u1)
            PAX_SWAP(float, v0, v1)
    #endif
        }
        nIter = (int)x1 - (int)x0 + 1;
    #ifdef PDHG_NORMAL_UV
        float u = u0, v = v0;
        float du = (u1 - u0) / nIter;
        float dv = (v1 - v0) / nIter;
    #endif
        for (int i = x0; i <= x1; i++) {
    #ifdef PDHG_NORMAL_UV
            pax_col_t result = (shader_ctx.callback)(
                color,
                shader_ctx.do_getter ? buf2col(buf, buf->getter(buf, index + i)) : 0,
                i,
                y0,
                u,
                v,
                shader_ctx.callback_args
            );
            setter(buf, result, index + i);
            u += du;
            v += dv;
    #else
            setter(buf, color, index + i);
    #endif
        }
    } else if (x0 == x1) {
        // Vertical line.
        int index = x0 + (int)y0 * buf->width;
        nIter     = (int)y1 - (int)y0 + 1;
    #ifdef PDHG_NORMAL_UV
        float u = u0, v = v0;
        float du = (u1 - u0) / nIter;
        float dv = (v1 - v0) / nIter;
    #endif
        int i = y0;
    #ifdef PDHG_MCR
        if ((i & 1) != odd_scanline) {
        #ifdef PDHG_NORMAL_UV
            u += du;
            v += dv;
        #endif
            i++;
            index += buf->width;
        }
    #endif
        for (; i <= y1; i += PDHG_INCREMENT, index += buf->width * PDHG_INCREMENT) {
    #ifdef PDHG_NORMAL_UV
            pax_col_t result = (shader_ctx.callback)(
                color,
                shader_ctx.do_getter ? buf2col(buf, buf->getter(buf, index)) : 0,
                x0,
                i,
                u,
                v,
                shader_ctx.callback_args
            );
            setter(buf, result, index);
            u += du * PDHG_INCREMENT;
            v += dv * PDHG_INCREMENT;
    #else
            setter(buf, color, index);
    #endif
        }
    } else {
        // Any other line.
    #ifdef PDHG_NORMAL_UV
        float du = (u1 - u0) / nIter;
        float dv = (v1 - v0) / nIter;
    #endif
        int_fast32_t x   = x0 * 0x10000 + 0x08000;
        int_fast32_t y   = y0 * 0x10000 + 0x08000;
        int_fast32_t idx = dx * 0x10000;
        int_fast32_t idy = dy * 0x10000;
    #ifdef PDHG_NORMAL_UV
        float u = u0;
        float v = v0;
    #endif
        for (int i = 0; i <= nIter; i++) {
            size_t delta = (x >> 16) + (y >> 16) * buf->width;
    #ifdef PDHG_MCR
            if (((y >> 16) & 1) == odd_scanline) {
    #endif
    #ifdef PDHG_NORMAL_UV
                pax_col_t result = (shader_ctx.callback)(
                    color,
                    shader_ctx.do_getter ? buf2col(buf, buf->getter(buf, delta)) : 0,
                    x >> 16,
                    y >> 16,
                    u,
                    v,
                    shader_ctx.callback_args
                );
                setter(buf, result, delta);
    #else
            setter(buf, color, delta);
    #endif
    #ifdef PDHG_MCR
            }
    #endif
            x += idx;
            y += idy;
    #ifdef PDHG_NORMAL_UV
            u += du;
            v += dv;
    #endif
        }
    }
}

#endif // PDHG_NAME

// Clean up macros.
#undef PDHG_INCREMENT
#undef PDHG_NORMAL_UV

// Clean up parameter macros.
#undef PDHG_STATIC
#undef PDHG_NAME
#undef PDHG_SHADED
#undef PDHG_MCR
#undef PDHG_IGNORE_UV
#undef PDHG_RESTRICT_UV
