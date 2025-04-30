
// SPDX-License-Identifier: MIT

#ifndef PAX_ARCS_H
#define PAX_ARCS_H

#include "pax_types.h"

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus



// Draw na arc, ignoring matrix transform.
// Angles in radians.
void pax_simple_arc(pax_buf_t *buf, pax_col_t color, float x, float y, float r, float a0, float a1);

// Draw an arc, angles in radians.
void pax_draw_arc(pax_buf_t *buf, pax_col_t color, float x, float y, float r, float a0, float a1);
// Draw a hollow arc.
void pax_draw_hollow_arc(
    pax_buf_t *buf, pax_col_t color, float x, float y, float radius0, float radius1, float a0, float a1
);
// Draw a hollow arc.
void pax_draw_round_hollow_arc(
    pax_buf_t *buf, pax_col_t color, float x, float y, float radius0, float radius1, float a0, float a1
);

// Draw an arc outline, angle in radians.
void pax_outline_arc(pax_buf_t *buf, pax_col_t color, float x, float y, float r, float a0, float a1);
// Draw a hollow arc.
void pax_outline_hollow_arc(
    pax_buf_t *buf, pax_col_t color, float x, float y, float radius0, float radius1, float a0, float a1
);
// Outline a hollow arc.
void pax_outline_round_hollow_arc(
    pax_buf_t *buf, pax_col_t color, float x, float y, float radius0, float radius1, float a0, float a1
);


// Vectorise an arc outline, angle in radians.
void pax_vectorise_arc(pax_vec2f *output, size_t num_points, float x, float y, float r, float a0, float a1);

// Draw an arc with a shader, angles in radians.
// If uvs is NULL, a default will be used (0,0; 1,0; 1,1; 0,1).
void pax_shade_arc(
    pax_buf_t          *buf,
    pax_col_t           color,
    pax_shader_t const *shader,
    pax_quadf const    *uvs,
    float               x,
    float               y,
    float               r,
    float               a0,
    float               a1
);

// Draw an arc outline with a shader, angle in radians.
// If uvs is NULL, a default will be used (0,0; 1,0; 1,1; 0,1).
void pax_shade_outline_arc(
    pax_buf_t          *buf,
    pax_col_t           color,
    pax_shader_t const *shader,
    pax_quadf const    *uvs,
    float               x,
    float               y,
    float               r,
    float               a0,
    float               a1
);



#ifdef __cplusplus
}
#endif //__cplusplus

#endif // PAX_ARCS_H
