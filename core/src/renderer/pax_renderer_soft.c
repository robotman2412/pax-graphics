
// SPDX-License-Identifier: MIT

#include "renderer/pax_renderer_soft.h"

#include "endian.h"
#include "helpers/pax_drawing_helpers.h"
#include "pax_internal.h"



// Draw a solid-colored line.
void pax_swr_unshaded_line(pax_buf_t *buf, pax_col_t color, pax_linef shape) {
    pax_line_unshaded(buf, color, shape.x0, shape.y0, shape.x1, shape.y1);
}

// Draw a solid-colored rectangle.
void pax_swr_unshaded_rect(pax_buf_t *buf, pax_col_t color, pax_rectf shape) {
    pax_rect_unshaded(buf, color, shape.x, shape.y, shape.w, shape.h);
}

// Draw a solid-colored quad.
void pax_swr_unshaded_quad(pax_buf_t *buf, pax_col_t color, pax_quadf shape) {
    pax_quad_unshaded(buf, color, shape.x0, shape.y0, shape.x1, shape.y1, shape.x2, shape.y2, shape.x3, shape.y3);
}

// Draw a solid-colored triangle.
void pax_swr_unshaded_tri(pax_buf_t *buf, pax_col_t color, pax_trif shape) {
    pax_tri_unshaded(buf, color, shape.x0, shape.y0, shape.x1, shape.y1, shape.x2, shape.y2);
}


// Draw a line with a shader.
void pax_swr_shaded_line(pax_buf_t *buf, pax_col_t color, pax_linef shape, pax_shader_t const *shader, pax_linef uv) {
    pax_line_shaded(buf, color, shader, shape.x0, shape.y0, shape.x1, shape.y1, uv.x0, uv.y0, uv.x1, uv.y1);
}

// Draw a rectangle with a shader.
void pax_swr_shaded_rect(pax_buf_t *buf, pax_col_t color, pax_rectf shape, pax_shader_t const *shader, pax_quadf uv) {
    // clang-format off
    pax_rect_shaded(
        buf, color, shader,
        shape.x, shape.y, shape.w, shape.h,
        uv.x0, uv.y0, uv.x1, uv.y1, uv.x2, uv.y2, uv.x3, uv.y3
    );
    // clang-format on
}

// Draw a quad with a shader.
void pax_swr_shaded_quad(pax_buf_t *buf, pax_col_t color, pax_quadf shape, pax_shader_t const *shader, pax_quadf uv) {
    // clang-format off
    pax_quad_shaded(
        buf, color, shader,
        shape.x0, shape.y0, shape.x1, shape.y1, shape.x2, shape.y2, shape.x3, shape.y3,
        uv.x0, uv.y0, uv.x1, uv.y1, uv.x2, uv.y2, uv.x3, uv.y3
    );
    // clang-format on
}

// Draw a triangle with a shader.
void pax_swr_shaded_tri(pax_buf_t *buf, pax_col_t color, pax_trif shape, pax_shader_t const *shader, pax_trif uv) {
    // clang-format off
    pax_tri_shaded(
        buf, color, shader,
        shape.x0, shape.y0, shape.x1, shape.y1, shape.x2, shape.y2,
        uv.x0, uv.y0, uv.x1, uv.y1, uv.x2, uv.y2
    );
    // clang-format on
}


// Read a single pixel from a raw buffer type by index.
__attribute__((always_inline)) static inline pax_col_t
    raw_get_pixel(void const *buf, uint8_t bpp, pax_vec2i dims, int index) {
    uint8_t const *buf_8bpp  = buf;
    uint8_t const *buf_16bpp = buf;
    uint8_t const *buf_32bpp = buf;
    switch (bpp) {
        case 1: return (buf_8bpp[index / 8] >> ((index % 8) * 1)) & 0x01;
        case 2: return (buf_8bpp[index / 4] >> ((index % 4) * 2)) & 0x03;
        case 4: return (buf_8bpp[index / 2] >> ((index % 2) * 4)) & 0x0f;
        case 8: return buf_8bpp[index];
        case 16: return buf_16bpp[index];
#if BYTE_ORDER == LITTLE_ENDIAN
        case 24: return buf_8bpp[index] | (buf_8bpp[index + 1] << 8) | (buf_8bpp[index + 2] << 16);
#else
        case 24: return buf_8bpp[index + 2] | (buf_8bpp[index + 1] << 8) | (buf_8bpp[index] << 16);
#endif
        case 32: return buf_32bpp[index];
        default: __builtin_unreachable();
    }
}

// Perform a buffer copying operation.
__attribute__((always_inline)) static inline void swr_blit_impl(
    pax_buf_t        *base,
    void const       *top,
    pax_vec2i         top_dims,
    pax_recti         base_pos,
    pax_orientation_t top_orientation,
    pax_vec2i         top_pos,
    bool              is_raw_buf,
    bool              is_pal_buf
) {
    // Determine copying parameters for top buffer.
#if PAX_COMPILE_ORIENTATION
    // clang-format off
    int dx, dy; bool swap;
    int top_dx, top_dy, top_index;
    switch (top_orientation) {
        case PAX_O_UPRIGHT:         dx =  1; dy =  1; swap = false; break;
        case PAX_O_ROT_CCW:         dx =  1; dy = -1; swap = true;  break;
        case PAX_O_ROT_HALF:        dx = -1; dy = -1; swap = false; break;
        case PAX_O_ROT_CW:          dx = -1; dy =  1; swap = true;  break;
        case PAX_O_FLIP_H:          dx = -1; dy =  1; swap = false; break;
        case PAX_O_ROT_CCW_FLIP_H:  dx = -1; dy = -1; swap = true;  break;
        case PAX_O_ROT_HALF_FLIP_H: dx =  1; dy = -1; swap = false; break;
        case PAX_O_ROT_CW_FLIP_H:   dx =  1; dy =  1; swap = true;  break;
        default: __builtin_unreachable();
    }
    // clang-format on
    if (swap) {
        top_dy    = dx;
        top_dx    = top_dims.x * dy - base_pos.w * dx;
        top_index = dx * top_pos.y + dy * top_pos.x * top_dims.x;
    } else {
        top_dx    = dx;
        top_dy    = top_dims.x * dy - base_pos.w * dx;
        top_index = dx * top_pos.x + dy * top_pos.y * top_dims.x;
    }
#else
    int top_dx    = 1;
    int top_dy    = top_dims.x - base_pos.w;
    int top_index = top_pos.x + top_dims.x * top_pos.y;
#endif

    // Determine copying parameters for bottom buffer.
    int base_dy    = base->width - base_pos.w;
    int base_index = base_pos.x + base->width * base_pos.y;

    for (int y = base_pos.y; y < base_pos.y + base_pos.h; y++) {
        for (int x = base_pos.x; x < base_pos.x + base_pos.w; x++) {
            if (is_raw_buf) {
                pax_col_t col = raw_get_pixel(top, base->bpp, top_dims, top_index);
                base->setter(base, col, base_index);
            } else if (is_pal_buf) {
                pax_buf_t const *_top = top;
                pax_col_t        col  = _top->getter(_top, top_index);
                base->setter(base, pax_closest_in_palette(base->palette, base->palette_size, col), base_index);
            } else {
                pax_buf_t const *_top = top;
                pax_col_t        col  = _top->buf2col(_top, _top->getter(_top, top_index));
                base->setter(base, base->col2buf(base, col), base_index);
            }
            base_index += 1;
            top_index  += top_dx;
        }
        base_index += base_dy;
        top_index  += top_dy;
    }
}


// Perform a buffer copying operation with a PAX buffer.
void pax_swr_blit(
    pax_buf_t *base, pax_buf_t *top, pax_recti base_pos, pax_orientation_t top_orientation, pax_vec2i top_pos
) {
    if (top->type == base->type) {
        // Equal buffer types; no color conversion required.
        pax_swr_blit_raw(base, top->buf, (pax_vec2i){top->width, top->height}, base_pos, top_orientation, top_pos);
    } else if (PAX_IS_PALETTE(base->type) && !PAX_IS_PALETTE(top->type)) {
        // Bottom is palette, top is not; do palette special case.
        swr_blit_impl(base, top, (pax_vec2i){top->width, top->height}, base_pos, top_orientation, top_pos, 0, 1);
    } else {
        // Different buffer types; color conversion required.
        swr_blit_impl(base, top, (pax_vec2i){top->width, top->height}, base_pos, top_orientation, top_pos, 0, 0);
    }
}

// Perform a buffer copying operation with an unmanaged user buffer.
__attribute__((noinline)) void pax_swr_blit_raw(
    pax_buf_t        *base,
    void const       *top,
    pax_vec2i         top_dims,
    pax_recti         base_pos,
    pax_orientation_t top_orientation,
    pax_vec2i         top_pos
) {
    swr_blit_impl(base, top, top_dims, base_pos, top_orientation, top_pos, true, false);
}



// Software rendering functions.
pax_render_funcs_t const pax_render_funcs_soft = {
    .unshaded_line = pax_swr_unshaded_line,
    .unshaded_rect = pax_swr_unshaded_rect,
    .unshaded_quad = pax_swr_unshaded_quad,
    .unshaded_tri  = pax_swr_unshaded_tri,
    .shaded_line   = pax_swr_shaded_line,
    .shaded_rect   = pax_swr_shaded_rect,
    .shaded_quad   = pax_swr_shaded_quad,
    .shaded_tri    = pax_swr_shaded_tri,
    .blit          = pax_swr_blit,
    .blit_raw      = pax_swr_blit_raw,
    .join          = NULL,
};

static pax_render_funcs_t const *init(void *ignored) {
    (void)ignored;
    return &pax_render_funcs_soft;
}

// Software rendering engine.
pax_render_engine_t const pax_render_engine_soft = {
    .init           = init,
    .implicit_dirty = true,
};
