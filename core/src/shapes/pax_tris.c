
// SPDX-License-Identifier: MIT

#include "pax_internal.h"
#include "pax_renderer.h"

// Dummy UVs used for tri UVs where NULL is provided.
static pax_trif const dummy_tri_uvs = {.x0 = 0, .y0 = 0, .x1 = 1, .y1 = 0, .x2 = 0, .y2 = 1};



// Draw a triangle, ignoring matrix transform.
void pax_simple_tri(pax_buf_t *buf, pax_col_t color, float x0, float y0, float x1, float y1, float x2, float y2) {
    PAX_BUF_CHECK(buf);
    if (!pax_do_draw_col(buf, color))
        return;

    if (!isfinite(x0) || !isfinite(y0) || !isfinite(x1) || !isfinite(y1) || !isfinite(x2) || !isfinite(y2)) {
        // We can't draw to infinity.
        PAX_ERROR(PAX_ERR_INF);
    }

    if ((y2 == y0 && y1 == y0) || (x2 == x0 && x1 == x0)) {
        // We can't draw a flat triangle.
        return;
    }

#if CONFIG_PAX_COMPILE_ORIENTATION
    // Rotate points.
    pax_vec1_t tmp = pax_orient_det_vec2f(buf, (pax_vec2f){x0, y0});
    x0             = tmp.x;
    y0             = tmp.y;
    tmp            = pax_orient_det_vec2f(buf, (pax_vec2f){x1, y1});
    x1             = tmp.x;
    y1             = tmp.y;
    tmp            = pax_orient_det_vec2f(buf, (pax_vec2f){x2, y2});
    x2             = tmp.x;
    y2             = tmp.y;
#endif

    pax_dispatch_unshaded_tri(buf, color, (pax_trif){x0, y0, x1, y1, x2, y2});
}


// Draw a triangle.
void pax_draw_tri(pax_buf_t *buf, pax_col_t color, float x0, float y0, float x1, float y1, float x2, float y2) {
    PAX_BUF_CHECK(buf);
    if (!pax_do_draw_col(buf, color))
        return;
    // Apply the transforms.
    matrix_2d_transform(buf->stack_2d.value, &x0, &y0);
    matrix_2d_transform(buf->stack_2d.value, &x1, &y1);
    matrix_2d_transform(buf->stack_2d.value, &x2, &y2);
    // Draw the triangle.
    pax_simple_tri(buf, color, x0, y0, x1, y1, x2, y2);
}

// Draw a triangle outline.
void pax_outline_tri(pax_buf_t *buf, pax_col_t color, float x0, float y0, float x1, float y1, float x2, float y2) {
    pax_draw_line(buf, color, x0, y0, x1, y1);
    pax_draw_line(buf, color, x2, y2, x1, y1);
    pax_draw_line(buf, color, x0, y0, x2, y2);
}


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
) {
    if (!shader) {
        // If shader is NULL, simplify this.
        pax_draw_tri(buf, color, x0, y0, x1, y1, x2, y2);
        return;
    }

    PAX_BUF_CHECK(buf);
    matrix_2d_transform(buf->stack_2d.value, &x0, &y0);
    matrix_2d_transform(buf->stack_2d.value, &x1, &y1);
    matrix_2d_transform(buf->stack_2d.value, &x2, &y2);

    if (!isfinite(x0) || !isfinite(y0) || !isfinite(x1) || !isfinite(y1) || !isfinite(x2) || !isfinite(y2)) {
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
    tmp            = pax_orient_det_vec2f(buf, (pax_vec2f){x2, y2});
    x2             = tmp.x;
    y2             = tmp.y;

    if (!uvs) {
        // Apply default UVs.
        uvs = &dummy_tri_uvs;
    }

    if ((y2 == y0 && y1 == y0) || (x2 == x0 && x1 == x0)) {
        // We can't draw a flat triangle.
        return;
    }

    pax_dispatch_shaded_tri(
        buf,
        color,
        (pax_trif){
            x0,
            y0,
            x1,
            y1,
            x2,
            y2,
        },
        shader,
        (pax_trif){
            uvs->x0,
            uvs->y0,
            uvs->x1,
            uvs->y1,
            uvs->x2,
            uvs->y2,
        }
    );
}

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
) {
    pax_linef tmp;
    if (uvs == NULL) {
        uvs = &dummy_tri_uvs;
    }

    tmp = (pax_linef){uvs->x0, uvs->y0, uvs->x1, uvs->y1};
    pax_shade_line(buf, color, shader, &tmp, x0, y0, x1, y1);

    tmp = (pax_linef){uvs->x1, uvs->y1, uvs->x2, uvs->y2};
    pax_shade_line(buf, color, shader, &tmp, x1, y1, x2, y2);

    tmp = (pax_linef){uvs->x2, uvs->y2, uvs->x0, uvs->y0};
    pax_shade_line(buf, color, shader, &tmp, x2, y2, x0, y0);
}
