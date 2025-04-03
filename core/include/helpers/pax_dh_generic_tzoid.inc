
// SPDX-License-Identifier: MIT

#include "pax_fixpt.hpp"
#include "pax_internal.h"

#ifdef PDHG_NAME

    #if defined(PDHG_SHADED) && !defined(PDHG_IGNORE_UV)
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

    #ifdef PDHG_RESTRICT_UV
        #error "Cannot define `PDHG_RESTRICT_UV` for triangles."
    #endif

    #if defined(PDHG_IGNORE_UV) && defined(PDHG_MCR)
        #define PDHG_TRAPEZOID_NAME paxmcr_trapezoid_shaded_nouv
    #elif defined(PDHG_SHADED) && defined(PDHG_MCR)
        #define PDHG_TRAPEZOID_NAME paxmcr_trapezoid_shaded
    #elif defined(PDHG_IGNORE_UV)
        #define PDHG_TRAPEZOID_NAME pax_trapezoid_shaded_nouv
    #elif defined(PDHG_SHADED)
        #define PDHG_TRAPEZOID_NAME pax_trapezoid_shaded
    #elif defined(PDHG_MCR)
        #define PDHG_TRAPEZOID_NAME paxmcr_trapezoid_unshaded
    #else
        #define PDHG_TRAPEZOID_NAME pax_trapezoid_unshaded
    #endif



    // Trapezoid function, a triangle is made out of two of these.
    // Assumes y0 < y1.
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
        fixpt_t x0a,
        fixpt_t x0b,
        fixpt_t y0,
        fixpt_t x1a,
        fixpt_t x1b,
        fixpt_t y1
    #ifdef PDHG_NORMAL_UV
        ,
        fixpt_t u0a,
        fixpt_t v0a,
        fixpt_t u0b,
        fixpt_t v0b,
        fixpt_t u1a,
        fixpt_t v1a,
        fixpt_t u1b,
        fixpt_t v1b
    #endif
    ) {

    #ifdef PDHG_SHADED
    // Get shader context.
    pax_shader_ctx_t shader_ctx = pax_get_shader_ctx(buf, color, shader);
    if (shader_ctx.skip)
        return;
    // Get pixel setter.
    pax_col_conv_t     buf2col = buf->type_info.fmt_type == PAX_BUF_SUBTYPE_PALETTE ? pax_col_conv_dummy : buf->buf2col;
    pax_index_setter_t setter  = pax_get_setter(buf, &color, shader);
    if (!setter)
        return;
    #else  // PDHG_SHADED
    // Get range setter.
    pax_range_setter_t setter = pax_get_range_setter(buf, &color);
    if (!setter)
        return;
    #endif // PDHG_SHADED

    // Determine vertical bounds.
    int iy0 = y0 + 0.5;
    int iy1 = y1 + 0.5;
    if (iy0 >= iy1)
        return;

    // Sort points by X.
    if (x0a > x0b) {
        PAX_SWAP(fixpt_t, x0a, x0b);
    #ifdef PDHG_NORMAL_UV
        PAX_SWAP(fixpt_t, u0a, u0b);
        PAX_SWAP(fixpt_t, v0a, v0b);
    #endif
    }
    if (x1a > x1b) {
        PAX_SWAP(fixpt_t, x1a, x1b);
    #ifdef PDHG_NORMAL_UV
        PAX_SWAP(fixpt_t, u1a, u1b);
        PAX_SWAP(fixpt_t, v1a, v1b);
    #endif
    }

    // Clip: Y axis.
    if (iy0 < buf->clip.y) {
        iy0 = buf->clip.y;
    }
    if (iy0 >= buf->clip.y + buf->clip.h) {
        return;
    }
    if (iy1 < buf->clip.y) {
        return;
    }
    if (iy1 >= buf->clip.y + buf->clip.h) {
        iy1 = buf->clip.y + buf->clip.h;
    }

    #ifdef PDHG_MCR
    // Snap y to correct scanline.
    if ((iy0 & 1) != odd_scanline) {
        iy0++;
    }
    #endif

    // Determine X deltas.
    fixpt_t x0a_x1a_dx = (x1a - x0a) / (y1 - y0);
    fixpt_t x0b_x1b_dx = (x1b - x0b) / (y1 - y0);

    #ifdef PDHG_NORMAL_UV
    // Determine UV deltas.
    fixpt_t u0a_u1a_du = (u1a - u0a) / (y1 - y0);
    fixpt_t u0b_u1b_du = (u1b - u0b) / (y1 - y0);
    fixpt_t v0a_v1a_dv = (v1a - v0a) / (y1 - y0);
    fixpt_t v0b_v1b_dv = (v1b - v0b) / (y1 - y0);
    #endif

    // Initial X interpolation.
    fixpt_t coeff = (iy0 + 0.5f) - y0;
    fixpt_t x_a   = x0a + x0a_x1a_dx * coeff;
    fixpt_t x_b   = x0b + x0b_x1b_dx * coeff;

    #ifdef PDHG_NORMAL_UV
    // Initial UV interpolation.
    fixpt_t u_a = u0a + u0a_u1a_du * coeff;
    fixpt_t u_b = u0b + u0b_u1b_du * coeff;
    fixpt_t v_a = v0a + v0a_v1a_dv * coeff;
    fixpt_t v_b = v0b + v0b_v1b_dv * coeff;
    #endif

    // Vertical drawing loop.
    int delta = buf->width * iy0;
    for (int y = iy0; y < iy1; y += PDHG_INCREMENT) {
        int ixa = x_a + 0.4999999, ixb = x_b + 0.5;

        // Clip: X axis.
        if (ixa < buf->clip.x) {
            ixa = buf->clip.x;
        }
        if (ixa > buf->clip.x + buf->clip.w) {
            ixa = buf->clip.x + buf->clip.w;
        }
        if (ixb < buf->clip.x) {
            ixb = buf->clip.x;
        }
        if (ixb > buf->clip.x + buf->clip.w) {
            ixb = buf->clip.x + buf->clip.w;
        }

    #ifdef PDHG_NORMAL_UV
        // Determine UV deltas.
        fixpt_t du = (u_b - u_a) / (x_b - x_a);
        fixpt_t dv = (v_b - v_a) / (x_b - x_a);

        // Interpolate UV.
        coeff     = (ixa + 0.5) - x_a;
        fixpt_t u = u_a + du * coeff;
        fixpt_t v = v_a + dv * coeff;
    #elif defined(PDHG_SHADED)
        fixpt_t u = 0, v = 0;
    #endif

    #ifdef PDHG_SHADED
        // Horizontal drawing loop.
        for (int x = ixa; x < ixb; x++) {
            // Apply the shader,
            pax_col_t result = (shader_ctx.callback)(
                color,
                shader_ctx.do_getter ? buf2col(buf, buf->getter(buf, x + delta)) : 0,
                x,
                y,
                u,
                v,
                shader_ctx.callback_args
            );
        #ifdef PDHG_NORMAL_UV
            u += du;
            v += dv;
        #endif
            // And simply merge colors accordingly.
            setter(buf, result, x + delta);
        }
    #else
        // Horizontal drawing loop.
        setter(buf, color, ixa + delta, ixb - ixa);
    #endif

    #ifdef PDHG_NORMAL_UV
        // Interpolate UVs.
        u_a += PDHG_INCREMENT * u0a_u1a_du;
        u_b += PDHG_INCREMENT * u0b_u1b_du;
        v_a += PDHG_INCREMENT * v0a_v1a_dv;
        v_b += PDHG_INCREMENT * v0b_v1b_dv;
    #endif

        // Interpolate X.
        x_a += PDHG_INCREMENT * x0a_x1a_dx;
        x_b += PDHG_INCREMENT * x0b_x1b_dx;

        delta += PDHG_INCREMENT * buf->width;
    }
}

#endif // PDHG_NAME

// Clean up macros.
#undef PDHG_INCREMENT
#undef PDHG_NORMAL_UV
#undef PDHG_TRAPEZOID_NAME

// Clean up parameter macros.
#undef PDHG_STATIC
#undef PDHG_NAME
#undef PDHG_SHADED
#undef PDHG_MCR
#undef PDHG_IGNORE_UV
#undef PDHG_RESTRICT_UV
