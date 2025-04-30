
// SPDX-License-Identifier: MIT

#include "pax_internal.h"
#include "pax_renderer.h"



// Dummy UVs used for quad UVs where NULL is provided.
static pax_quadf const dummy_quad_uvs = {.x0 = 0, .y0 = 0, .x1 = 1, .y1 = 0, .x2 = 1, .y2 = 1, .x3 = 0, .y3 = 1};



// Draw a rectangle, ignoring matrix transform.
void pax_simple_rect(pax_buf_t *buf, pax_col_t color, float x, float y, float width, float height) {
    PAX_BUF_CHECK(buf);
    if (!pax_do_draw_col(buf, color))
        return;

#if CONFIG_PAX_COMPILE_ORIENTATION
    // Do rotation.
    pax_rectf tmp = pax_orient_det_rectf(buf, (pax_rectf){x, y, width, height});
    x             = tmp.x;
    y             = tmp.y;
    width         = tmp.w;
    height        = tmp.h;
#endif

    pax_dispatch_unshaded_rect(buf, color, (pax_rectf){x, y, width, height});
}


// Draw a rectangle.
void pax_draw_rect(pax_buf_t *buf, pax_col_t color, float x, float y, float width, float height) {
    PAX_BUF_CHECK(buf);
    if (!pax_do_draw_col(buf, color))
        return;

    if (matrix_2d_is_identity2(buf->stack_2d.value)) {
        // We don't need to use triangles here.
        matrix_2d_transform(buf->stack_2d.value, &x, &y);
        width  *= buf->stack_2d.value.a0;
        height *= buf->stack_2d.value.b1;
        pax_simple_rect(buf, color, x, y, width, height);
    } else {
        // Draw as a quad.
        matrix_2d_t mtx       = buf->stack_2d.value;
        pax_vec2f   vertex[4] = {
            matrix_2d_transform_alt(mtx, (pax_vec2f){x, y}),
            matrix_2d_transform_alt(mtx, (pax_vec2f){x + width, y}),
            matrix_2d_transform_alt(mtx, (pax_vec2f){x + width, y + height}),
            matrix_2d_transform_alt(mtx, (pax_vec2f){x, y + height}),
        };
        pax_dispatch_unshaded_quad(
            buf,
            color,
            (pax_quadf){
                vertex[0].x,
                vertex[0].y,
                vertex[1].x,
                vertex[1].y,
                vertex[2].x,
                vertex[2].y,
                vertex[3].x,
                vertex[3].y,
            }
        );
    }
}

// Draw a rounded rectangle.
void pax_draw_round_rect(pax_buf_t *buf, pax_col_t color, float x, float y, float width, float height, float radius) {
    if (radius <= 0) {
        pax_draw_rect(buf, color, x, y, width, height);
        return;
    } else if (width == height && radius >= width) {
        pax_draw_circle(buf, color, x + width * 0.5, y + width * 0.5, width * 0.5);
        return;
    }

    // Clamp size.
    if (width < 0) {
        x     += width;
        width  = -width;
    }
    if (height < 0) {
        y      += height;
        height  = -height;
    }

    // Clamp radius.
    if (radius < 0) {
        radius = 0;
    } else {
        if (radius > width / 2) {
            radius = width / 2;
        }
        if (radius > height / 2) {
            radius = height / 2;
        }
    }

    // Draw corners.
    pax_draw_arc(buf, color, x + radius, y + radius, radius, M_PI * 0.5, M_PI * 1.0);
    pax_draw_arc(buf, color, x + width - radius, y + radius, radius, M_PI * 0.0, M_PI * 0.5);
    pax_draw_arc(buf, color, x + width - radius, y + height - radius, radius, M_PI * 0.0, M_PI * -0.5);
    pax_draw_arc(buf, color, x + radius, y + height - radius, radius, M_PI * -0.5, M_PI * -1.0);

    // Draw infill.
    if (width > 2 * radius) {
        pax_draw_rect(buf, color, x + radius, y, width - 2 * radius, radius);
        pax_draw_rect(buf, color, x + radius, y + height - radius, width - 2 * radius, radius);
    }
    if (height > 2 * radius) {
        pax_draw_rect(buf, color, x, y + radius, width, height - 2 * radius);
    }
}

// Draw a rounded rectangle with different radii per corner.
// The radii start top-left and go clockwise.
void pax_draw_round_rect4(
    pax_buf_t *buf, pax_col_t color, float x, float y, float width, float height, float r0, float r1, float r2, float r3
) {
    if (r0 <= 0) {
        r0 = 0;
    }
    if (r1 <= 0) {
        r1 = 0;
    }
    if (r2 <= 0) {
        r2 = 0;
    }
    if (r3 <= 0) {
        r3 = 0;
    }
    if (r0 == r1 && r1 == r2 && r2 == r3) {
        pax_draw_round_rect(buf, color, x, y, width, height, r0);
        return;
    }

    // Clamp size.
    if (width < 0) {
        PAX_SWAP(float, r0, r1);
        PAX_SWAP(float, r3, r2);
        x     += width;
        width  = -width;
    }
    if (height < 0) {
        PAX_SWAP(float, r0, r3);
        PAX_SWAP(float, r1, r2);
        y      += height;
        height  = -height;
    }

    // Clamp radius.
    if (r0 > width / 2) {
        r0 = width / 2;
    }
    if (r1 > width / 2) {
        r1 = width / 2;
    }
    if (r2 > width / 2) {
        r2 = width / 2;
    }
    if (r3 > width / 2) {
        r3 = width / 2;
    }
    if (r0 > height / 2) {
        r0 = height / 2;
    }
    if (r1 > height / 2) {
        r1 = height / 2;
    }
    if (r2 > height / 2) {
        r2 = height / 2;
    }
    if (r3 > height / 2) {
        r3 = height / 2;
    }

    // Draw corners.
    if (r0) {
        pax_draw_arc(buf, color, x + r0, y + r0, r0, M_PI * 0.5, M_PI * 1.0);
    }
    if (r1) {
        pax_draw_arc(buf, color, x + width - r1, y + r1, r1, M_PI * 0.0, M_PI * 0.5);
    }
    if (r2) {
        pax_draw_arc(buf, color, x + width - r2, y + height - r2, r2, M_PI * 0.0, M_PI * -0.5);
    }
    if (r3) {
        pax_draw_arc(buf, color, x + r3, y + height - r3, r3, M_PI * -0.5, M_PI * -1.0);
    }

    // Draw infill.
    if (r0 && r1) {
        pax_draw_rect(buf, color, x + r0, y, width - r0 - r1, fminf(r0, r1));
    }
    if (r0 != r1) {
        pax_draw_rect(buf, color, r0 < r1 ? x : x + r0, y + fminf(r0, r1), width - fmaxf(r0, r1), fabsf(r0 - r1));
    }
    if (fmaxf(r0, r1) + fmaxf(r2, r3) < height) {
        pax_draw_rect(buf, color, x, y + fmaxf(r0, r1), width, height - fmaxf(r0, r1) - fmaxf(r2, r3));
    }
    if (r3 != r2) {
        pax_draw_rect(
            buf,
            color,
            r3 < r2 ? x : x + r3,
            y + height - fmaxf(r3, r2),
            height - fmaxf(r3, r2),
            fabsf(r3 - r2)
        );
    }
    if (r2 && r3) {
        pax_draw_rect(buf, color, x + r3, y + height - fminf(r3, r2), width - r3 - r2, fminf(r3, r2));
    }
}


// Draw a rectangle outline.
void pax_outline_rect(pax_buf_t *buf, pax_col_t color, float x, float y, float width, float height) {
    pax_push_2d(buf);
    pax_apply_2d(buf, matrix_2d_translate(x, y));
    pax_draw_line(buf, color, 0, 0, width, 0);
    pax_draw_line(buf, color, 0, height, width, height);
    pax_draw_line(buf, color, 0, 0, 0, height);
    pax_draw_line(buf, color, width, 0, width, height);
    pax_pop_2d(buf);
}

// Outline a rounded rectangle.
void pax_outline_round_rect(
    pax_buf_t *buf, pax_col_t color, float x, float y, float width, float height, float radius
) {
    if (radius <= 0) {
        pax_outline_rect(buf, color, x, y, width, height);
        return;
    } else if (width == height && radius >= width) {
        pax_outline_circle(buf, color, x + width * 0.5, y + width * 0.5, width * 0.5);
        return;
    }

    // Clamp size.
    if (width < 0) {
        x     += width;
        width  = -width;
    }
    if (height < 0) {
        y      += height;
        height  = -height;
    }

    // Clamp radius.
    if (radius < 0) {
        radius = 0;
    } else {
        if (radius > width / 2) {
            radius = width / 2;
        }
        if (radius > height / 2) {
            radius = height / 2;
        }
    }

    // Draw corners.
    pax_outline_arc(buf, color, x + radius, y + radius, radius, M_PI * 0.5, M_PI * 1.0);
    pax_outline_arc(buf, color, x + width - radius, y + radius, radius, M_PI * 0.0, M_PI * 0.5);
    pax_outline_arc(buf, color, x + width - radius, y + height - radius, radius, M_PI * 0.0, M_PI * -0.5);
    pax_outline_arc(buf, color, x + radius, y + height - radius, radius, M_PI * -0.5, M_PI * -1.0);

    // Draw infill.
    pax_draw_line(buf, color, x + radius, y, x + width - radius, y);
    pax_draw_line(buf, color, x + radius, y + height, x + width - radius, y + height);
    pax_draw_line(buf, color, x, y + radius, x, y + height - radius);
    pax_draw_line(buf, color, x + width, y + radius, x + width, y + height - radius);
}

// Outline a rounded rectangle with different radii per corner.
// The radii start top-left and go clockwise.
void pax_outline_round_rect4(
    pax_buf_t *buf, pax_col_t color, float x, float y, float width, float height, float r0, float r1, float r2, float r3
) {
    if (r0 <= 0) {
        r0 = 0;
    }
    if (r1 <= 0) {
        r1 = 0;
    }
    if (r2 <= 0) {
        r2 = 0;
    }
    if (r3 <= 0) {
        r3 = 0;
    }
    if (r0 == r1 && r1 == r2 && r2 == r3) {
        pax_outline_round_rect(buf, color, x, y, width, height, r0);
        return;
    }

    // Clamp size.
    if (width < 0) {
        PAX_SWAP(float, r0, r1);
        PAX_SWAP(float, r3, r2);
        x     += width;
        width  = -width;
    }
    if (height < 0) {
        PAX_SWAP(float, r0, r3);
        PAX_SWAP(float, r1, r2);
        y      += height;
        height  = -height;
    }

    // Clamp radius.
    if (r0 > width / 2) {
        r0 = width / 2;
    }
    if (r1 > width / 2) {
        r1 = width / 2;
    }
    if (r2 > width / 2) {
        r2 = width / 2;
    }
    if (r3 > width / 2) {
        r3 = width / 2;
    }
    if (r0 > height / 2) {
        r0 = height / 2;
    }
    if (r1 > height / 2) {
        r1 = height / 2;
    }
    if (r2 > height / 2) {
        r2 = height / 2;
    }
    if (r3 > height / 2) {
        r3 = height / 2;
    }

    // Draw corners.
    if (r0) {
        pax_outline_arc(buf, color, x + r0, y + r0, r0, M_PI * 0.5, M_PI * 1.0);
    }
    if (r1) {
        pax_outline_arc(buf, color, x + width - r1, y + r1, r1, M_PI * 0.0, M_PI * 0.5);
    }
    if (r2) {
        pax_outline_arc(buf, color, x + width - r2, y + height - r2, r2, M_PI * 0.0, M_PI * -0.5);
    }
    if (r3) {
        pax_outline_arc(buf, color, x + r3, y + height - r3, r3, M_PI * -0.5, M_PI * -1.0);
    }

    // Draw edges.
    if (r0 + r1 < width) {
        pax_draw_line(buf, color, x + r0, y, x + width - r1, y);
    }
    if (r3 + r2 < width) {
        pax_draw_line(buf, color, x + r3, y + height, x + width - r2, y + height);
    }
    if (r3 + r0 < height) {
        pax_draw_line(buf, color, x, y + r0, x, y + height - r3);
    }
    if (r1 + r2 < height) {
        pax_draw_line(buf, color, x + width, y + r1, x + width, y + height - r2);
    }
}


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
) {
    if (!shader) {
        // If shader is NULL, simplify this.
        pax_draw_rect(buf, color, x, y, width, height);
        return;
    }

    PAX_BUF_CHECK(buf);

    if (!uvs) {
        // Apply default UVs.
        uvs = &dummy_quad_uvs;
    }

    if (matrix_2d_is_identity2(buf->stack_2d.value)) {
        // We don't need to use triangles here.
        matrix_2d_transform(buf->stack_2d.value, &x, &y);
        width  *= buf->stack_2d.value.a0;
        height *= buf->stack_2d.value.b1;

// Perform rotation.
#if CONFIG_PAX_COMPILE_ORIENTATION
        pax_rectf tmp = pax_orient_det_rectf(buf, (pax_rectf){x, y, width, height});
        x             = tmp.x;
        y             = tmp.y;
        width         = tmp.w;
        height        = tmp.h;

        pax_quadf uvs_rotated;
        if (buf->orientation & 1) {
            uvs_rotated = (pax_quadf){
                uvs->x0,
                uvs->y0,
                uvs->x3,
                uvs->y3,
                uvs->x2,
                uvs->y2,
                uvs->x1,
                uvs->y1,
            };
            uvs = &uvs_rotated;
        }
#endif

        pax_dispatch_shaded_rect(
            buf,
            color,
            (pax_rectf){
                x,
                y,
                width,
                height,
            },
            shader,
            (pax_quadf){
                uvs->x0,
                uvs->y0,
                uvs->x1,
                uvs->y1,
                uvs->x2,
                uvs->y2,
                uvs->x3,
                uvs->y3,
            }
        );
    } else {
        // Draw as a quad.
        matrix_2d_t mtx       = buf->stack_2d.value;
        pax_vec2f   vertex[4] = {
            matrix_2d_transform_alt(mtx, (pax_vec2f){x, y}),
            matrix_2d_transform_alt(mtx, (pax_vec2f){x + width, y}),
            matrix_2d_transform_alt(mtx, (pax_vec2f){x + width, y + height}),
            matrix_2d_transform_alt(mtx, (pax_vec2f){x, y + height}),
        };
        pax_dispatch_shaded_quad(
            buf,
            color,
            (pax_quadf){
                vertex[0].x,
                vertex[0].y,
                vertex[1].x,
                vertex[1].y,
                vertex[2].x,
                vertex[2].y,
                vertex[3].x,
                vertex[3].y,
            },
            shader,
            (pax_quadf){
                uvs->x0,
                uvs->y0,
                uvs->x1,
                uvs->y1,
                uvs->x2,
                uvs->y2,
                uvs->x3,
                uvs->y3,
            }
        );
    }
}

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
) {
    pax_linef tmp;
    if (uvs == NULL) {
        uvs = &dummy_quad_uvs;
    }

    pax_push_2d(buf);
    pax_apply_2d(buf, matrix_2d_translate(x, y));

    tmp = (pax_linef){uvs->x0, uvs->y0, uvs->x1, uvs->y1};
    pax_shade_line(buf, color, shader, &tmp, 0, 0, width, 0);

    tmp = (pax_linef){uvs->x1, uvs->y1, uvs->x2, uvs->y2};
    pax_shade_line(buf, color, shader, &tmp, width, 0, width, height);

    tmp = (pax_linef){uvs->x2, uvs->y2, uvs->x3, uvs->y3};
    pax_shade_line(buf, color, shader, &tmp, width, height, 0, height);

    tmp = (pax_linef){uvs->x3, uvs->y3, uvs->x0, uvs->y0};
    pax_shade_line(buf, color, shader, &tmp, 0, height, 0, 0);

    pax_pop_2d(buf);
}
