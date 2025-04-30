
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

#if CONFIG_PAX_COMPILE_ORIENTATION
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

// Draw a thick line using a rectangle.
// Note: Will look different than `pax_draw_line` even if `thickness == 1`.
void pax_draw_thick_line(pax_buf_t *buf, pax_col_t color, float x0, float y0, float x1, float y1, float thickness) {
    pax_vec2f direction  = {x1 - x0, y1 - y0};
    pax_vec2f tangent    = pax_vec2f_unify((pax_vec2f){-direction.y, direction.x});
    tangent.x           *= thickness * 0.5f;
    tangent.y           *= thickness * 0.5f;
    pax_vec2f vert[4]    = {
        {x0 + tangent.x, y0 + tangent.y},
        {x1 + tangent.x, y1 + tangent.y},
        {x1 - tangent.x, y1 - tangent.y},
        {x0 - tangent.x, y0 - tangent.y},
    };
    for (int i = 0; i < 4; i++) {
        vert[i] = matrix_2d_transform_alt(buf->stack_2d.value, vert[i]);
    }
    pax_dispatch_unshaded_quad(
        buf,
        color,
        (pax_quadf){
            vert[0].x,
            vert[0].y,
            vert[1].x,
            vert[1].y,
            vert[2].x,
            vert[2].y,
            vert[3].x,
            vert[3].y,
        }
    );
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
