
// SPDX-License-Identifier: MIT

#ifndef PAX_LINES_H
#define PAX_LINES_H

#include "pax_types.h"

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus


// Draw a line, ignoring matrix transform.
void pax_simple_line(pax_buf_t *buf, pax_col_t color, float x0, float y0, float x1, float y1);

// Draw a line.
void pax_draw_line(pax_buf_t *buf, pax_col_t color, float x0, float y0, float x1, float y1);
// Draw a thick line using a rectangle.
// Note: Will look different than `pax_draw_line` even if `thickness == 1`.
void pax_draw_thick_line(pax_buf_t *buf, pax_col_t color, float x0, float y0, float x1, float y1, float thickness);

// Draw a line with a shader.
// If uvs is NULL, a default will be used (0,0; 1,0).
void pax_shade_line(
    pax_buf_t          *buf,
    pax_col_t           color,
    pax_shader_t const *shader,
    pax_linef const    *uvs,
    float               x0,
    float               y0,
    float               x1,
    float               y1
);


#ifdef __cplusplus
}
#endif //__cplusplus

#endif // PAX_LINES_H
