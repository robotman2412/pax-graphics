
// SPDX-License-Identifier: MIT

#include "pax_internal.h"
#include "pax_renderer.h"



// Draw a line, ignoring matrix transform.
void pax_simple_line(pax_buf_t *buf, pax_col_t color, float x0, float y0, float x1, float y1) {
    PAX_BUF_CHECK(buf);
    if (!pax_do_draw_col(buf, color))
        return;

    if (!isfinite(x0) || !isfinite(y0) || !isfinite(x1) || !isfinite(y1)) {
        // We can't draw to infinity.
        PAX_ERROR(PAX_ERR_INF);
    }

#if PAX_COMPILE_ORIENTATION
    // Rotate points.
    pax_vec1_t tmp = pax_orient_det_vec2f(buf, (pax_vec2f){x0, y0});
    x0             = tmp.x;
    y0             = tmp.y;
    tmp            = pax_orient_det_vec2f(buf, (pax_vec2f){x1, y1});
    x1             = tmp.x;
    y1             = tmp.y;
#endif

    pax_dispatch_unshaded_line(buf, color, (pax_linef){x0, y0, x1, y1});
}


// Draw a line.
void pax_draw_line(pax_buf_t *buf, pax_col_t color, float x0, float y0, float x1, float y1) {
    PAX_BUF_CHECK(buf);
    if (!pax_do_draw_col(buf, color))
        return;
    // Apply transforms.
    matrix_2d_transform(buf->stack_2d.value, &x0, &y0);
    matrix_2d_transform(buf->stack_2d.value, &x1, &y1);
    // Draw the line.
    pax_simple_line(buf, color, x0, y0, x1, y1);
}


// Draw a line with a shader.
// Beta feature: UVs are not currently available.
void pax_shade_line(
    pax_buf_t          *buf,
    pax_col_t           color,
    pax_shader_t const *shader,
    pax_linef const    *uvs,
    float               x0,
    float               y0,
    float               x1,
    float               y1
) {
    if (!shader) {
        pax_draw_line(buf, color, x0, y0, x1, y1);
        return;
    }

    PAX_BUF_CHECK(buf);

    float u0, v0, u1, v1;

    if (uvs) {
        u0 = uvs->x0;
        v0 = uvs->y0;
        u1 = uvs->x1;
        v1 = uvs->y1;
    } else {
        u0 = 0;
        v0 = 0;
        u1 = 1;
        v1 = 0;
    }

    // Apply transforms.
    matrix_2d_transform(buf->stack_2d.value, &x0, &y0);
    matrix_2d_transform(buf->stack_2d.value, &x1, &y1);

    if (!isfinite(x0) || !isfinite(y0) || !isfinite(x1) || !isfinite(y1)) {
        // We can't draw to infinity.
        PAX_ERROR(PAX_ERR_INF);
    }

    // Rotate points.
    pax_vec1_t tmp = pax_orient_det_vec2f(buf, (pax_vec2f){x0, y0});
    x0             = tmp.x;
    y0             = tmp.y;
    tmp            = pax_orient_det_vec2f(buf, (pax_vec2f){x1, y1});
    x1             = tmp.x;
    y1             = tmp.y;

    // If any point is outside clip now, we don't draw a line.
    if (y0 < buf->clip.y || y1 > buf->clip.y + buf->clip.h - 1)
        return;

    pax_dispatch_shaded_line(buf, color, (pax_linef){x0, y0, x1, y1}, shader, (pax_linef){u0, v0, u1, v1});
}
