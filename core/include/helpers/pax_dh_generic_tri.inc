
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



    // Generic triangle drawing code, assuming some defines are made.
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
        float _x0,
        float _y0,
        float _x1,
        float _y1,
        float _x2,
        float _y2
    #ifdef PDHG_NORMAL_UV
        ,
        float _u0,
        float _v0,
        float _u1,
        float _v1,
        float _u2,
        float _v2
    #endif
    ) {

    fixpt_t x0 = _x0;
    fixpt_t y0 = _y0;
    fixpt_t x1 = _x1;
    fixpt_t y1 = _y1;
    fixpt_t x2 = _x2;
    fixpt_t y2 = _y2;
    #ifdef PDHG_NORMAL_UV
    fixpt_t u0 = _u0;
    fixpt_t v0 = _v0;
    fixpt_t u1 = _u1;
    fixpt_t v1 = _v1;
    fixpt_t u2 = _u2;
    fixpt_t v2 = _v2;
    #endif

    // Sort points by height.
    if (y1 < y0) {
        PAX_SWAP(fixpt_t, x0, x1)
        PAX_SWAP(fixpt_t, y0, y1)
    #ifdef PDHG_NORMAL_UV
        PAX_SWAP(fixpt_t, u0, u1)
        PAX_SWAP(fixpt_t, v0, v1)
    #endif
    }
    if (y2 < y0) {
        PAX_SWAP(fixpt_t, x0, x2)
        PAX_SWAP(fixpt_t, y0, y2)
    #ifdef PDHG_NORMAL_UV
        PAX_SWAP(fixpt_t, u0, u2)
        PAX_SWAP(fixpt_t, v0, v2)
    #endif
    }
    if (y2 < y1) {
        PAX_SWAP(fixpt_t, x1, x2)
        PAX_SWAP(fixpt_t, y1, y2)
    #ifdef PDHG_NORMAL_UV
        PAX_SWAP(fixpt_t, u1, u2)
        PAX_SWAP(fixpt_t, v1, v2)
    #endif
    }

    // Interpolate coordinates.
    fixpt_t coeff = (y1 - y0) / (y2 - y0);
    fixpt_t x1b   = x0 + (x2 - x0) * coeff;
    #ifdef PDHG_NORMAL_UV
    fixpt_t u1b = u0 + (u2 - u0) * coeff;
    fixpt_t v1b = v0 + (v2 - v0) * coeff;
    #endif

    // Top half.
    PDHG_TZOID_NAME(
    #ifdef PDHG_MCR
        odd_scanline,
    #endif
        buf,
        color,
    #ifdef PDHG_SHADED
        shader,
    #endif
        x0,
        x0,
        y0,
        x1,
        x1b,
        y1
    #ifdef PDHG_NORMAL_UV
        ,
        u0,
        v0,
        u0,
        v0,
        u1,
        v1,
        u1b,
        v1b
    #endif
    );
    // Bottom half.
    PDHG_TZOID_NAME(
    #ifdef PDHG_MCR
        odd_scanline,
    #endif
        buf,
        color,
    #ifdef PDHG_SHADED
        shader,
    #endif
        x1,
        x1b,
        y1,
        x2,
        x2,
        y2
    #ifdef PDHG_NORMAL_UV
        ,
        u1,
        v1,
        u1b,
        v1b,
        u2,
        v2,
        u2,
        v2
    #endif
    );
}



#endif // PDHG_NAME

// Clean up macros.
#undef PDHG_INCREMENT
#undef PDHG_NORMAL_UV
#undef PDHG_TZOID_NAME

// Clean up parameter macros.
#undef PDHG_STATIC
#undef PDHG_NAME
#undef PDHG_SHADED
#undef PDHG_MCR
#undef PDHG_IGNORE_UV
#undef PDHG_RESTRICT_UV
