
// SPDX-License-Identifier: MIT

#ifndef PAX_TRIS_H
#define PAX_TRIS_H

#include "pax_types.h"

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus


// Draw a triangle, ignoring matrix transform.
void pax_simple_tri(pax_buf_t *buf, pax_col_t color, float x0, float y0, float x1, float y1, float x2, float y2);

// Draw a triangle.
void pax_draw_tri(pax_buf_t *buf, pax_col_t color, float x0, float y0, float x1, float y1, float x2, float y2);
// Draw a triangle.
void pax_outline_tri(pax_buf_t *buf, pax_col_t color, float x0, float y0, float x1, float y1, float x2, float y2);

// Draw a triangle with a shader.
// If uvs is NULL, a default will be used (0,0; 1,0; 0,1).
void pax_shade_tri(
    pax_buf_t          *buf,
    pax_col_t           color,
    pax_shader_t const *shader,
    pax_trif const     *uvs,
    float               x0,
    float               y0,
    float               x1,
    float               y1,
    float               x2,
    float               y2
);
// Draw a triangle with a shader.
// If uvs is NULL, a default will be used (0,0; 1,0; 1,1; 0,1).
void pax_shade_outline_tri(
    pax_buf_t          *buf,
    pax_col_t           color,
    pax_shader_t const *shader,
    pax_trif const     *uvs,
    float               x0,
    float               y0,
    float               x1,
    float               y1,
    float               x2,
    float               y2
);


#ifdef __cplusplus
}
#endif //__cplusplus

#endif // PAX_TRIS_H
