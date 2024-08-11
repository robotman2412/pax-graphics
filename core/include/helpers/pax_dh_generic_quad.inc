
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



    #ifdef PDHG_NORMAL_UV
        #define DH_SORT(p0, p1)                                                                                        \
            do {                                                                                                       \
                if (y##p0 > y##p1) {                                                                                   \
                    PAX_SWAP(fixpt_t, x##p0, x##p1)                                                                    \
                    PAX_SWAP(fixpt_t, y##p0, y##p1)                                                                    \
                    PAX_SWAP(fixpt_t, u##p0, u##p1)                                                                    \
                    PAX_SWAP(fixpt_t, v##p0, v##p1)                                                                    \
                }                                                                                                      \
            } while (0)
    #else
        #define DH_SORT(p0, p1)                                                                                        \
            do {                                                                                                       \
                if (y##p0 > y##p1) {                                                                                   \
                    PAX_SWAP(fixpt_t, x##p0, x##p1)                                                                    \
                    PAX_SWAP(fixpt_t, y##p0, y##p1)                                                                    \
                }                                                                                                      \
            } while (0)
    #endif

    // Generic quad drawing code, assuming some defines are made.
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
        float _y2,
        float _x3,
        float _y3
    #ifdef PDHG_NORMAL_UV
        ,
        float _u0,
        float _v0,
        float _u1,
        float _v1,
        float _u2,
        float _v2,
        float _u3,
        float _v3
    #endif
    ) {

    fixpt_t x0 = _x0;
    fixpt_t y0 = _y0;
    fixpt_t x1 = _x1;
    fixpt_t y1 = _y1;
    fixpt_t x2 = _x2;
    fixpt_t y2 = _y2;
    fixpt_t x3 = _x3;
    fixpt_t y3 = _y3;
    #ifdef PDHG_NORMAL_UV
    fixpt_t u0 = _u0;
    fixpt_t v0 = _v0;
    fixpt_t u1 = _u1;
    fixpt_t v1 = _v1;
    fixpt_t u2 = _u2;
    fixpt_t v2 = _v2;
    fixpt_t u3 = _u3;
    fixpt_t v3 = _v3;
    #endif

    // Sort points by height.
    DH_SORT(0, 1);
    DH_SORT(2, 3);
    DH_SORT(0, 2);
    DH_SORT(1, 3);
    DH_SORT(1, 2);

    // Interpolate coordinates.
    fixpt_t coeff1 = (y1 - y0) / (y2 - y0);
    fixpt_t x1b    = x0 + (x2 - x0) * coeff1;
    #ifdef PDHG_NORMAL_UV
    fixpt_t u1b = u0 + (u2 - u0) * coeff1;
    fixpt_t v1b = v0 + (v2 - v0) * coeff1;
    #endif

    fixpt_t coeff2 = (y2 - y1) / (y3 - y1);
    fixpt_t x2b    = x1 + (x3 - x1) * coeff2;
    #ifdef PDHG_NORMAL_UV
    fixpt_t u2b = u1 + (u3 - u1) * coeff2;
    fixpt_t v2b = v1 + (v3 - v1) * coeff2;
    #endif

    // Top third.
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
    // Middle third.
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
        x2b,
        y2
    #ifdef PDHG_NORMAL_UV
        ,
        u1,
        v1,
        u1b,
        v1b,
        u2,
        v2,
        u2b,
        v2b
    #endif
    );
    // Bottom third.
    PDHG_TZOID_NAME(
    #ifdef PDHG_MCR
        odd_scanline,
    #endif
        buf,
        color,
    #ifdef PDHG_SHADED
        shader,
    #endif
        x2,
        x2b,
        y2,
        x3,
        x3,
        y3
    #ifdef PDHG_NORMAL_UV
        ,
        u2,
        v2,
        u2b,
        v2b,
        u3,
        v3,
        u3,
        v3
    #endif
    );
}



#endif // PDHG_NAME

// Clean up macros.
#undef PDHG_INCREMENT
#undef PDHG_NORMAL_UV
#undef DH_SORT

// Clean up parameter macros.
#undef PDHG_STATIC
#undef PDHG_NAME
#undef PDHG_TZOID_NAME
#undef PDHG_SHADED
#undef PDHG_MCR
#undef PDHG_IGNORE_UV
#undef PDHG_RESTRICT_UV
