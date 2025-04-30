
// SPDX-License-Identifier: MIT

#ifndef PAX_DRAWING_HELPERS_H
#define PAX_DRAWING_HELPERS_H

#include "pax_gfx.h"

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus


// clang-format off

// Multi-core method for unshaded lines.
// If odd_scanline is true, the odd (counted from 0) lines are drawn, otherwise the even lines are drawn.
void paxmcr_line_shaded(
    bool                odd_scanline,
    pax_buf_t          *buf,
    pax_col_t           color,
    pax_shader_t const *shader,
    float x0, float y0, float x1, float y1,
    float u0, float v0, float u1, float v1
);

// Multi-core method for shaded triangles.
// If odd_scanline is true, the odd (counted from 0) lines are drawn, otherwise the even lines are drawn.
void paxmcr_tri_shaded(
    bool                odd_scanline,
    pax_buf_t          *buf,
    pax_col_t           color,
    pax_shader_t const *shader,
    float x0, float y0, float x1, float y1, float x2, float y2,
    float u0, float v0, float u1, float v1, float u2, float v2
);

// Internal method for shaded quads.
// If odd_scanline is true, the odd (counted from 0) lines are drawn, otherwise the even lines are drawn.
void paxmcr_quad_shaded(
    bool                odd_scanline,
    pax_buf_t          *buf,
    pax_col_t           color,
    pax_shader_t const *shader,
    float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3,
    float u0, float v0, float u1, float v1, float u2, float v2, float u3, float v3
);

//  Multi-core optimisation which maps a buffer directly onto another.
// If odd_scanline is true, the odd (counted from 0) lines are drawn, otherwise the even lines are drawn.
void paxmcr_overlay_buffer(
    bool odd_scanline, pax_buf_t *base, pax_buf_t *top,
    int x, int y, int width, int height,
    bool assume_opaque
);

//  Multi-core  method for shaded rects.
// If odd_scanline is true, the odd (counted from 0) lines are drawn, otherwise the even lines are drawn.
void paxmcr_rect_shaded(
    bool                odd_scanline,
    pax_buf_t          *buf,
    pax_col_t           color,
    pax_shader_t const *shader,
    float x, float y, float width, float height,
    float u0, float v0, float u1, float v1, float u2, float v2, float u3, float v3
);



// Multi-core method for unshaded lines.
// If odd_scanline is true, the odd (counted from 0) lines are drawn, otherwise the even lines are drawn.
void paxmcr_line_unshaded(
    bool odd_scanline, pax_buf_t *buf, pax_col_t color,
    float x0, float y0, float x1, float y1
);

// Multi-core method for unshaded triangles.
// Assumes points are sorted by Y.
// If odd_scanline is true, the odd (counted from 0) lines are drawn, otherwise the even lines are drawn.
void paxmcr_tri_unshaded(
    bool odd_scanline, pax_buf_t *buf, pax_col_t color,
    float x0, float y0, float x1, float y1, float x2, float y2
);

// Internal method for shaded quads.
// If odd_scanline is true, the odd (counted from 0) lines are drawn, otherwise the even lines are drawn.
void paxmcr_quad_unshaded(
    bool                odd_scanline,
    pax_buf_t          *buf,
    pax_col_t           color,
    float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3
);

// Multi-core method for unshaded rects.
// If odd_scanline is true, the odd (counted from 0) lines are drawn, otherwise the even lines are drawn.
void paxmcr_rect_unshaded(
    bool odd_scanline, pax_buf_t *buf, pax_col_t color,
    float x, float y, float width, float height
);



// Internal method for shaded triangles.
// Assumes points are sorted by Y.
void pax_tri_shaded(
    pax_buf_t          *buf,
    pax_col_t           color,
    pax_shader_t const *shader,
    float x0, float y0, float x1, float y1, float x2, float y2,
    float u0, float v0, float u1, float v1, float u2, float v2
);

// Internal method for shaded quads.
void pax_quad_shaded(
    pax_buf_t          *buf,
    pax_col_t           color,
    pax_shader_t const *shader,
    float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3,
    float u0, float v0, float u1, float v1, float u2, float v2, float u3, float v3
);

// Optimisation which maps a buffer directly onto another.
// If assume_opaque is true, the overlay is done without transparency.
void pax_overlay_buffer(
    pax_buf_t *base, pax_buf_t *top,
    int x, int y, int width, int height,
    bool assume_opaque
);

// Internal method for shaded rects.
void pax_rect_shaded(
    pax_buf_t          *buf,
    pax_col_t           color,
    pax_shader_t const *shader,
    float x, float y, float width, float height,
    float u0, float v0, float u1, float v1, float u2, float v2, float u3, float v3
);

// Internal method for line drawing.
void pax_line_shaded(
    pax_buf_t          *buf,
    pax_col_t           color,
    pax_shader_t const *shader,
    float x0, float y0, float x1, float y1,
    float u0, float v0, float u1, float v1
);



// Internal method for unshaded triangles.
// Assumes points are sorted by Y.
void pax_tri_unshaded(
    pax_buf_t *buf, pax_col_t color,
    float x0, float y0, float x1, float y1, float x2, float y2
);

// Internal method for shaded quads.
void pax_quad_unshaded(
    pax_buf_t *buf,
    pax_col_t  color,
    float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3
);

// Internal method for rectangle drawing.
void pax_rect_unshaded(
    pax_buf_t *buf, pax_col_t color,
    float x, float y, float width, float height
);

// Internal method for line drawing.
void pax_line_unshaded(pax_buf_t *buf, pax_col_t color,
    float x0, float y0, float x1, float y1
);

// clang-format on


#ifdef __cplusplus
} // extern "C"
#endif //__cplusplus

#endif // PAX_DRAWING_HELPERS_H
