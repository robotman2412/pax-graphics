
// SPDX-License-Identifier: MIT

#include "pax_internal.h"



// Select an appropriate precalculated circle.
static int pax_pick_circle(matrix_2d_t const *matrix, float r, pax_vec2f const **vertex, pax_trif const **uv) {
    float c_r = r * sqrtf(matrix->a0 * matrix->a0 + matrix->b0 * matrix->b0)
                * sqrtf(matrix->a1 * matrix->a1 + matrix->b1 * matrix->b1);
    if (c_r > 30) {
        *vertex = pax_precalc_circle_24;
        *uv     = pax_precalc_uv_circle_24;
        return 24;
    } else if (c_r > 7) {
        *vertex = pax_precalc_circle_16;
        *uv     = pax_precalc_uv_circle_16;
        return 16;
    } else {
        *vertex = pax_precalc_circle_8;
        *uv     = pax_precalc_uv_circle_8;
        return 8;
    }
}


// Draw a circle, ignoring matrix transform.
void pax_simple_circle(pax_buf_t *buf, pax_col_t color, float x, float y, float r) {
    pax_simple_arc(buf, color, x, y, r, 0, M_PI * 2);
}

// Draw a circle.
void pax_draw_circle(pax_buf_t *buf, pax_col_t color, float x, float y, float r) {
    PAX_BUF_CHECK(buf);
    if (!pax_do_draw_col(buf, color))
        return;

    // Use precalcualted circles for speed because the user can't tell anyway.
    pax_vec2f const *preset;
    pax_trif const  *uv_set;
    size_t           size = pax_pick_circle(&buf->stack_2d.value, r, &preset, &uv_set);

    // Use the builtin matrix stuff to our advantage.
    pax_push_2d(buf);
    pax_apply_2d(buf, matrix_2d_translate(x, y));
    pax_apply_2d(buf, matrix_2d_scale(r, r));
    // Plot all the triangles in the ROM.
    for (size_t i = 0; i < size - 1; i++) {
        pax_draw_tri(buf, color, preset[0].x, preset[0].y, preset[i].x, preset[i].y, preset[i + 1].x, preset[i + 1].y);
    }
    pax_pop_2d(buf);
}

// Draw a hollow circle.
void pax_draw_hollow_circle(pax_buf_t *buf, pax_col_t color, float x, float y, float radius0, float radius1) {
    pax_draw_hollow_arc(buf, color, x, y, radius0, radius1, 0, M_PI * 2);
}


// Draw a circle outline.
void pax_outline_circle(pax_buf_t *buf, pax_col_t color, float x, float y, float r) {
    PAX_BUF_CHECK(buf);
    if (!pax_do_draw_col(buf, color))
        return;

    // Use precalcualted circles for speed because the user can't tell anyway.
    pax_vec2f const *preset;
    pax_trif const  *uv_set;
    size_t           size = pax_pick_circle(&buf->stack_2d.value, r, &preset, &uv_set);

    // Use the builtin matrix stuff to our advantage.
    pax_push_2d(buf);
    pax_apply_2d(buf, matrix_2d_translate(x, y));
    pax_apply_2d(buf, matrix_2d_scale(r, r));
    // Plot all the lines in the ROM.
    for (size_t i = 0; i < size; i++) {
        pax_draw_line(buf, color, preset[i].x, preset[i].y, preset[i + 1].x, preset[i + 1].y);
    }
    pax_pop_2d(buf);
}

// Draw a hollow circle.
void pax_outline_hollow_circle(pax_buf_t *buf, pax_col_t color, float x, float y, float radius0, float radius1) {
    pax_outline_circle(buf, color, x, y, radius0);
    pax_outline_circle(buf, color, x, y, radius1);
}


// Draw a circle with a shader.
// If uvs is NULL, a default will be used (0,0; 1,0; 1,1; 0,1).
void pax_shade_circle(
    pax_buf_t *buf, pax_col_t color, pax_shader_t const *shader, pax_quadf const *uvs, float x, float y, float r
) {
    if (!shader) {
        pax_draw_circle(buf, color, x, y, r);
    }

    PAX_BUF_CHECK(buf);

    // Use precalcualted circles for speed because the user can't tell anyway.
    pax_vec2f const *preset;
    pax_trif const  *uv_set;
    size_t           size = pax_pick_circle(&buf->stack_2d.value, r, &preset, &uv_set);

    // Use the builtin matrix stuff to our advantage.
    pax_push_2d(buf);
    pax_apply_2d(buf, matrix_2d_translate(x, y));
    pax_apply_2d(buf, matrix_2d_scale(r, r));
    if (uvs) {
        // UV interpolation required.
        pax_trif uv_res;
        uv_res.x0 = (uvs->x1 + uvs->x2) * 0.5;
        uv_res.y0 = (uvs->y1 + uvs->y2) * 0.5;
        uv_res.x1 = pax_flerp4(preset[1].x, -preset[1].y, uvs->x0, uvs->x1, uvs->x3, uvs->x2);
        uv_res.y1 = pax_flerp4(preset[1].x, -preset[1].y, uvs->y0, uvs->y1, uvs->y3, uvs->y2);
        for (size_t i = 0; i < size - 1; i++) {
            uv_res.x2 = pax_flerp4(preset[i + 1].x, -preset[i + 1].y, uvs->x0, uvs->x1, uvs->x3, uvs->x2);
            uv_res.y2 = pax_flerp4(preset[i + 1].x, -preset[i + 1].y, uvs->y0, uvs->y1, uvs->y3, uvs->y2);
            pax_shade_tri(
                buf,
                color,
                shader,
                &uv_res,
                preset[0].x,
                preset[0].y,
                preset[i].x,
                preset[i].y,
                preset[i + 1].x,
                preset[i + 1].y
            );
            uv_res.x1 = uv_res.x2;
            uv_res.y1 = uv_res.y2;
        }

    } else {
        // No UV interpolation needed.
        for (size_t i = 0; i < size - 1; i++) {
            pax_shade_tri(
                buf,
                color,
                shader,
                &uv_set[i],
                preset[0].x,
                preset[0].y,
                preset[i].x,
                preset[i].y,
                preset[i + 1].x,
                preset[i + 1].y
            );
        }
    }
    pax_pop_2d(buf);
}

// Draw a circle outline with a shader.
// If uvs is NULL, a default will be used (0,0; 1,0; 1,1; 0,1).
void pax_shade_outline_circle(
    pax_buf_t *buf, pax_col_t color, pax_shader_t const *shader, pax_quadf const *uvs, float x, float y, float r
) {
    pax_shade_outline_arc(buf, color, shader, uvs, x, y, r, 0, M_PI * 2);
}


// Vectorise a circle outline.
void pax_vectorise_circle(pax_vec2f *output, size_t num_points, float x, float y, float r) {
    pax_vectorise_arc(output, num_points, x, y, r, 0, M_PI * 2);
}
