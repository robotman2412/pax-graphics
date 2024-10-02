
// SPDX-License-Identifier: MIT

#ifndef PAX_CIRCLES_H
#define PAX_CIRCLES_H

#include "pax_types.h"

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus


// Draw a circle, ignoring matrix transform.
void pax_simple_circle(pax_buf_t *buf, pax_col_t color, float x, float y, float r);

// Draw a circle.
void pax_draw_circle(pax_buf_t *buf, pax_col_t color, float x, float y, float r);
// Draw a hollow circle.
void pax_draw_hollow_circle(pax_buf_t *buf, pax_col_t color, float x, float y, float radius0, float radius1);

// Draw a circle outline.
void pax_outline_circle(pax_buf_t *buf, pax_col_t color, float x, float y, float r);
// Draw a hollow circle.
void pax_outline_hollow_circle(pax_buf_t *buf, pax_col_t color, float x, float y, float radius0, float radius1);

// Draw a circle with a shader.
// If uvs is NULL, a default will be used (0,0; 1,0; 1,1; 0,1).
void pax_shade_circle(
    pax_buf_t *buf, pax_col_t color, pax_shader_t const *shader, pax_quadf const *uvs, float x, float y, float r
);
// Draw a circle outline with a shader.
// If uvs is NULL, a default will be used (0,0; 1,0; 1,1; 0,1).
void pax_shade_outline_circle(
    pax_buf_t *buf, pax_col_t color, pax_shader_t const *shader, pax_quadf const *uvs, float x, float y, float r
);

// Vectorise a circle outline.
void pax_vectorise_circle(pax_vec2f *output, size_t num_points, float x, float y, float r);


#ifdef __cplusplus
}
#endif //__cplusplus

#endif // PAX_CIRCLES_H
