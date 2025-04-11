
// SPDX-License-Identifier: MIT

#include "pax_internal.h"



// Dummy UVs used for quad UVs where NULL is provided.
static pax_quadf const dummy_quad_uvs = {.x0 = 0, .y0 = 0, .x1 = 1, .y1 = 0, .x2 = 1, .y2 = 1, .x3 = 0, .y3 = 1};

// Select a number of divisions for an arc.
static int pax_pick_arc_divs(matrix_2d_t const *matrix, float r, float a0, float a1) {
    float c_r = r * sqrtf(matrix->a0 * matrix->a0 + matrix->b0 * matrix->b0)
                * sqrtf(matrix->a1 * matrix->a1 + matrix->b1 * matrix->b1);
    int n_div;
    if (c_r > 30) {
        n_div = (a1 - a0) / M_PI * 24;
    } else if (c_r > 7) {
        n_div = (a1 - a0) / M_PI * 16;
    } else {
        n_div = (a1 - a0) / M_PI * 8;
    }
    return n_div <= 1 ? 1 : n_div;
}



// Draw a arc, ignoring matrix transform.
// Angles in radians.
void pax_simple_arc(pax_buf_t *buf, pax_col_t color, float x, float y, float r, float a0, float a1) {
    PAX_BUF_CHECK(buf);
    if (!pax_do_draw_col(buf, color))
        return;

    // Simplify the angles slightly.
    float a2  = fmodf(a0, M_PI * 2);
    a1       += a2 - a0;
    a0        = a2;
    if (a1 < a0)
        PAX_SWAP(float, a0, a1);
    if (a1 - a0 > M_PI * 2) {
        a1 = M_PI * 2;
        a0 = 0;
    }

    // Pick an appropriate number of divisions.
    int n_div;
    if (r > 30)
        n_div = (a1 - a0) / M_PI * 32 + 1;
    if (r > 20)
        n_div = (a1 - a0) / M_PI * 16 + 1;
    else
        n_div = (a1 - a0) / M_PI * 8 + 1;

    // Get the sine and cosine of one division, used for rotation in the loop.
    float div_angle = (a1 - a0) / n_div;
    float c_sin     = sinf(div_angle);
    float c_cos     = cosf(div_angle);

    // Start with a unit vector according to a0.
    float x0 = cosf(a0);
    float y0 = sinf(a0);

    // Draw it as a series of triangles, rotating with what is essentially matrix multiplication.
    for (int i = 0; i < n_div; i++) {
        // Perform the rotation.
        float x1 = x0 * c_cos - y0 * c_sin;
        float y1 = x0 * c_sin + y0 * c_cos;
        // We subtract y0 and y1 from y because our up is -y.
        pax_simple_tri(buf, color, x, y, x + x0 * r, y - y0 * r, x + x1 * r, y - y1 * r);
        // Assign them yes.
        x0 = x1;
        y0 = y1;
    }
}

// Draw na arc, angles in radians.
void pax_draw_arc(pax_buf_t *buf, pax_col_t color, float x, float y, float r, float a0, float a1) {
    PAX_BUF_CHECK(buf);
    if (!pax_do_draw_col(buf, color))
        return;

    // Simplify the angles slightly.
    float a2  = fmodf(a0, M_PI * 2);
    a1       += a2 - a0;
    a0        = a2;
    if (a1 < a0)
        PAX_SWAP(float, a0, a1);
    if (a1 - a0 > M_PI * 2) {
        a1 = M_PI * 2;
        a0 = 0;
    }

    // Pick an appropriate number of divisions.
    int n_div = pax_pick_arc_divs(&buf->stack_2d.value, r, a0, a1);

    // Get the sine and cosine of one division, used for rotation in the loop.
    float div_angle = (a1 - a0) / n_div;
    float c_sin     = sinf(div_angle);
    float c_cos     = cosf(div_angle);

    // Start with a unit vector according to a0.
    float x0 = cosf(a0);
    float y0 = sinf(a0);

    // Draw it as a series of triangles, rotating with what is essentially matrix multiplication.
    for (int i = 0; i < n_div; i++) {
        // Perform the rotation.
        float x1 = x0 * c_cos - y0 * c_sin;
        float y1 = x0 * c_sin + y0 * c_cos;
        // We subtract y0 and y1 from y because our up is -y.
        pax_draw_tri(buf, color, x, y, x + x0 * r, y - y0 * r, x + x1 * r, y - y1 * r);
        // Assign the newly rotated vectors.
        x0 = x1;
        y0 = y1;
    }
}

// Draw a hollow arc.
void pax_draw_hollow_arc(
    pax_buf_t *buf, pax_col_t color, float x, float y, float radius0, float radius1, float a0, float a1
) {
    size_t    n_points = 24;
    pax_vec2f points[n_points];
    pax_vectorise_arc(points, n_points, 0, 0, 1, a0, a1);
    for (size_t i = 0; i < n_points - 1; i++) {
        pax_draw_tri(
            buf,
            color,
            x + points[i].x * radius0,
            y + points[i].y * radius0,
            x + points[i].x * radius1,
            y + points[i].y * radius1,
            x + points[i + 1].x * radius1,
            y + points[i + 1].y * radius1
        );
        pax_draw_tri(
            buf,
            color,
            x + points[i].x * radius0,
            y + points[i].y * radius0,
            x + points[i + 1].x * radius0,
            y + points[i + 1].y * radius0,
            x + points[i + 1].x * radius1,
            y + points[i + 1].y * radius1
        );
    }
}

// Draw a hollow arc.
void pax_draw_round_hollow_arc(
    pax_buf_t *buf, pax_col_t color, float x, float y, float radius0, float radius1, float a0, float a1
) {
    pax_draw_hollow_arc(buf, color, x, y, radius0, radius1, a0, a1);
    float radius  = (radius0 + radius1) / 2;
    float dradius = fabsf(radius1 - radius0) / 2;
    float a2      = a0 - M_PI;
    float a3      = a0;
    float a4      = a1;
    float a5      = a1 + M_PI;
    if (a1 < a0) {
        a2 += M_PI;
        a3 += M_PI;
        a4 += M_PI;
        a5 += M_PI;
    }
    pax_draw_arc(buf, color, x + cos(a0) * radius, y - sinf(a0) * radius, dradius, a2, a3);
    pax_draw_arc(buf, color, x + cos(a1) * radius, y - sinf(a1) * radius, dradius, a4, a5);
}


// Draw an arc outline, angle in radians.
void pax_outline_arc(pax_buf_t *buf, pax_col_t color, float x, float y, float r, float a0, float a1) {
    PAX_BUF_CHECK(buf);

    // Simplify the angles slightly.
    float a2  = fmodf(a0, M_PI * 2);
    a1       += a2 - a0;
    a0        = a2;
    if (a1 < a0)
        PAX_SWAP(float, a0, a1);
    if (a1 - a0 > M_PI * 2) {
        a1 = M_PI * 2;
        a0 = 0;
    }

    // Pick an appropriate number of divisions.
    int n_div = pax_pick_arc_divs(&buf->stack_2d.value, r, a0, a1);
    if (!n_div) {
        n_div = 1;
    }

    // Get the sine and cosine of one division, used for rotation in the loop.
    float div_angle = (a1 - a0) / n_div;
    float c_sin     = sinf(div_angle);
    float c_cos     = cosf(div_angle);

    // Start with a unit vector according to a0.
    float x0 = cosf(a0);
    float y0 = sinf(a0);

    // Draw it as a series of triangles, rotating with what is essentially matrix multiplication.
    for (int i = 0; i < n_div; i++) {
        // Perform the rotation.
        float x1 = x0 * c_cos - y0 * c_sin;
        float y1 = x0 * c_sin + y0 * c_cos;
        // We subtract y0 and y1 from y because our up is -y.
        pax_draw_line(buf, color, x + x0 * r, y - y0 * r, x + x1 * r, y - y1 * r);
        // Assign them yes.
        x0 = x1;
        y0 = y1;
    }
}

// Draw a hollow arc.
static void pax_outline_hollow_arc0(
    pax_buf_t *buf, pax_col_t color, float x, float y, float radius0, float radius1, float a0, float a1
) {
    size_t    n_points = 24;
    pax_vec2f points[n_points];
    pax_vectorise_arc(points, n_points, 0, 0, 1, a0, a1);
    for (size_t i = 0; i < n_points - 1; i++) {
        pax_draw_line(
            buf,
            color,
            x + points[i].x * radius0,
            y + points[i].y * radius0,
            x + points[i + 1].x * radius0,
            y + points[i + 1].y * radius0
        );
        pax_draw_line(
            buf,
            color,
            x + points[i].x * radius1,
            y + points[i].y * radius1,
            x + points[i + 1].x * radius1,
            y + points[i + 1].y * radius1
        );
    }
}

// Draw a hollow arc.
void pax_outline_hollow_arc(
    pax_buf_t *buf, pax_col_t color, float x, float y, float radius0, float radius1, float a0, float a1
) {
    pax_outline_hollow_arc0(buf, color, x, y, radius0, radius1, a0, a1);
    float sin0 = sinf(a0);
    float cos0 = cosf(a0);
    float sin1 = sinf(a1);
    float cos1 = cosf(a1);
    pax_draw_line(buf, color, x + cos0 * radius0, y - sin0 * radius0, x + cos0 * radius1, y - sin0 * radius1);
    pax_draw_line(buf, color, x + cos1 * radius0, y - sin1 * radius0, x + cos1 * radius1, y - sin1 * radius1);
}

// Outline a hollow arc.
void pax_outline_round_hollow_arc(
    pax_buf_t *buf, pax_col_t color, float x, float y, float radius0, float radius1, float a0, float a1
) {
    pax_outline_hollow_arc0(buf, color, x, y, radius0, radius1, a0, a1);
    float radius  = (radius0 + radius1) / 2;
    float dradius = fabsf(radius1 - radius0) / 2;
    float a2      = a0 - M_PI;
    float a3      = a0;
    float a4      = a1;
    float a5      = a1 + M_PI;
    if (a1 < a0) {
        a2 += M_PI;
        a3 += M_PI;
        a4 += M_PI;
        a5 += M_PI;
    }
    pax_outline_arc(buf, color, x + cos(a0) * radius, y - sinf(a0) * radius, dradius, a2, a3);
    pax_outline_arc(buf, color, x + cos(a1) * radius, y - sinf(a1) * radius, dradius, a4, a5);
}



// Vectorise an arc outline, angle in radians.
void pax_vectorise_arc(pax_vec2f *ptr, size_t n_div, float x, float y, float r, float a0, float a1) {
    // Allocate output array.
    if (!ptr) {
        PAX_ERROR(PAX_ERR_PARAM);
    }

    // Simplify the angles slightly.
    float a2  = fmodf(a0, M_PI * 2);
    a1       += a2 - a0;
    a0        = a2;
    if (a1 < a0)
        PAX_SWAP(float, a0, a1);
    if (a1 - a0 > M_PI * 2) {
        a1 = M_PI * 2;
        a0 = 0;
    }

    // Get the sine and cosine of one division, used for rotation in the loop.
    float div_angle = (a1 - a0) / (n_div - 1);
    float c_sin     = sinf(div_angle);
    float c_cos     = cosf(div_angle);

    // Start with a unit vector according to a0.
    float x0 = cosf(a0);
    float y0 = sinf(a0);

    // Draw it as a series of triangles, rotating with what is essentially matrix multiplication.
    for (size_t i = 0; i < n_div; i++) {
        // Perform the rotation.
        float x1 = x0 * c_cos - y0 * c_sin;
        float y1 = x0 * c_sin + y0 * c_cos;
        // Store to array.
        // We subtract y0 and y1 from y because our up is -y.
        ptr[i]   = (pax_vec2f){.x = x + x0 * r, .y = y - y0 * r};
        // Assign them yes.
        x0       = x1;
        y0       = y1;
    }
}


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
) {
    if (!shader) {
        // If shader is NULL, simplify this.
        pax_draw_arc(buf, color, x, y, r, a0, a1);
        return;
    }

    PAX_BUF_CHECK(buf);

    if (!uvs) {
        // Assign default UVs.
        uvs = &dummy_quad_uvs;
    }

    // Simplify the angles slightly.
    float a2  = fmodf(a0, M_PI * 2);
    a1       += a2 - a0;
    a0        = a2;
    if (a1 < a0)
        PAX_SWAP(float, a0, a1);
    if (a1 - a0 > M_PI * 2) {
        a1 = M_PI * 2;
        a0 = 0;
    }

    // Pick an appropriate number of divisions.
    int n_div = pax_pick_arc_divs(&buf->stack_2d.value, r, a0, a1);

    // Get the sine and cosine of one division, used for rotation in the loop.
    float div_angle = (a1 - a0) / n_div;
    float c_sin     = sinf(div_angle);
    float c_cos     = cosf(div_angle);

    // Start with a unit vector according to a0.
    float x0 = cosf(a0);
    float y0 = sinf(a0);

    // Prepare some UVs to apply to the triangle.
    pax_trif tri_uvs;
    tri_uvs.x0 = (uvs->x0 + uvs->x1 + uvs->x2 + uvs->x3) * 0.25;
    tri_uvs.y0 = (uvs->y0 + uvs->y1 + uvs->y2 + uvs->y3) * 0.25;

    tri_uvs.x1 = pax_flerp4(x0, y0, uvs->x0, uvs->x1, uvs->x3, uvs->x2);
    tri_uvs.y1 = pax_flerp4(x0, y0, uvs->y0, uvs->y1, uvs->y3, uvs->y2);

    // Draw it as a series of triangles, rotating with what is essentially matrix multiplication.
    for (int i = 0; i < n_div; i++) {
        // Perform the rotation.
        float x1   = x0 * c_cos - y0 * c_sin;
        float y1   = x0 * c_sin + y0 * c_cos;
        // And UV interpolation.
        tri_uvs.x2 = pax_flerp4(x1, y1, uvs->x0, uvs->x1, uvs->x3, uvs->x2);
        tri_uvs.y2 = pax_flerp4(x1, y1, uvs->y0, uvs->y1, uvs->y3, uvs->y2);
        // We subtract y0 and y1 from y because our up is -y.
        pax_shade_tri(buf, color, shader, &tri_uvs, x, y, x + x0 * r, y - y0 * r, x + x1 * r, y - y1 * r);
        // Assign the newly rotated vectors.
        x0         = x1;
        y0         = y1;
        tri_uvs.x1 = tri_uvs.x2;
        tri_uvs.y1 = tri_uvs.y2;
    }
}


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
) {
    PAX_BUF_CHECK(buf);
    pax_linef tmp;
    if (uvs == NULL) {
        uvs = &dummy_quad_uvs;
    }

    // Simplify the angles slightly.
    float a2  = fmodf(a0, M_PI * 2);
    a1       += a2 - a0;
    a0        = a2;
    if (a1 < a0)
        PAX_SWAP(float, a0, a1);
    if (a1 - a0 > M_PI * 2) {
        a1 = M_PI * 2;
        a0 = 0;
    }

    // Pick an appropriate number of divisions.
    int n_div = pax_pick_arc_divs(&buf->stack_2d.value, r, a0, a1);

    // Get the sine and cosine of one division, used for rotation in the loop.
    float div_angle = (a1 - a0) / n_div;
    float c_sin     = sinf(div_angle);
    float c_cos     = cosf(div_angle);

    // Start with a unit vector according to a0.
    float x0 = cosf(a0);
    float y0 = sinf(a0);
    // Transform UV coords.
    tmp.x0   = pax_flerp4(x0, y0, uvs->x0, uvs->x1, uvs->x3, uvs->x2);
    tmp.y0   = pax_flerp4(x0, y0, uvs->y0, uvs->y1, uvs->y3, uvs->y2);

    // Draw it as a series of triangles, rotating with what is essentially matrix multiplication.
    for (int i = 0; i < n_div; i++) {
        // Perform the rotation.
        float x1 = x0 * c_cos - y0 * c_sin;
        float y1 = x0 * c_sin + y0 * c_cos;
        // Transform UV coords.
        tmp.x1   = pax_flerp4(x1, y1, uvs->x0, uvs->x1, uvs->x3, uvs->x2);
        tmp.y1   = pax_flerp4(x1, y1, uvs->y0, uvs->y1, uvs->y3, uvs->y2);
        // We subtract y0 and y1 from y because our up is -y.
        pax_shade_line(buf, color, shader, &tmp, x + x0 * r, y - y0 * r, x + x1 * r, y - y1 * r);
        // Assign them yes.
        x0     = x1;
        y0     = y1;
        tmp.x0 = tmp.x1;
        tmp.y0 = tmp.y1;
    }
}
