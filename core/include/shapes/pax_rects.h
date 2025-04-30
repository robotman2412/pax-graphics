
// SPDX-License-Identifier: MIT

#ifndef PAX_RECTS_H
#define PAX_RECTS_H

#include "pax_types.h"

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus


// Draw a rectangle, ignoring matrix transform.
void pax_simple_rect(pax_buf_t *buf, pax_col_t color, float x, float y, float width, float height);

// Draw a rectangle.
void pax_draw_rect(pax_buf_t *buf, pax_col_t color, float x, float y, float width, float height);
// Draw a rounded rectangle.
void pax_draw_round_rect(pax_buf_t *buf, pax_col_t color, float x, float y, float width, float height, float radius);
// Draw a rounded rectangle with different radii per corner.
// The radii start top-left and go clockwise.
void pax_draw_round_rect4(
    pax_buf_t *buf, pax_col_t color, float x, float y, float width, float height, float r0, float r1, float r2, float r3
);

// Draw a rectangle outline.
void pax_outline_rect(pax_buf_t *buf, pax_col_t color, float x, float y, float width, float height);
// Outline a rounded rectangle.
void pax_outline_round_rect(pax_buf_t *buf, pax_col_t color, float x, float y, float width, float height, float radius);
// Outline a rounded rectangle with different radii per corner.
// The radii start top-left and go clockwise.
void pax_outline_round_rect4(
    pax_buf_t *buf, pax_col_t color, float x, float y, float width, float height, float r0, float r1, float r2, float r3
);

// Draw a rectangle with a shader.
// If uvs is NULL, a default will be used (0,0; 1,0; 1,1; 0,1).
void pax_shade_rect(
    pax_buf_t          *buf,
    pax_col_t           color,
    pax_shader_t const *shader,
    pax_quadf const    *uvs,
    float               x,
    float               y,
    float               width,
    float               height
);
// Draw a rectangle outline with a shader.
// If uvs is NULL, a default will be used (0,0; 1,0; 1,1; 0,1).
void pax_shade_outline_rect(
    pax_buf_t          *buf,
    pax_col_t           color,
    pax_shader_t const *shader,
    pax_quadf const    *uvs,
    float               x,
    float               y,
    float               width,
    float               height
);


#ifdef __cplusplus
}
#endif //__cplusplus

#endif // PAX_RECTS_H
