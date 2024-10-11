
// SPDX-License-Identifier: MIT

#include "renderer/pax_renderer_softasync.h"

#include "endian.h"
#include "helpers/pax_drawing_helpers.h"
#include "pax_internal.h"
#include "ptq.h"
#include "renderer/pax_renderer_soft.h"

#include <stdatomic.h>

#include <pthread.h>

bool is_multithreaded;



static pthread_t              handle0;
static pax_sasr_worker_args_t args0;

#if PAX_COMPILE_ASYNC_RENDERER == 2
static pthread_t              handle1;
static pax_sasr_worker_args_t args1;
#endif

// Queue a draw call.
static void pax_sasr_queue(pax_task_t *task);


// Initialize the async renderer.
static pax_render_funcs_t const *pax_sasr_init(void *arg) {
    args0.queue = ptq_create_max(sizeof(pax_task_t), PAX_QUEUE_SIZE);
    pthread_mutex_init(&args0.rendermtx, NULL);
    args0.renderfuncs = &pax_render_funcs_soft;

#if PAX_COMPILE_ASYNC_RENDERER == 2
    is_multithreaded = arg;
    if (is_multithreaded) {
        args0.renderfuncs = &pax_render_funcs_mcr_thread0;
        args1.renderfuncs = &pax_render_funcs_mcr_thread1;
        args1.queue       = ptq_create_max(sizeof(pax_task_t), PAX_QUEUE_SIZE);
        pthread_mutex_init(&args1.rendermtx, NULL);
        pthread_create(&handle1, NULL, pax_sasr_worker, &args1);
    }
#endif
    pthread_create(&handle0, NULL, pax_sasr_worker, &args0);

    return &pax_render_funcs_softasync;
}

// Deinitialize the async renderer.
static void pax_sasr_deinit() {
    pax_task_t task = {
        .type = PAX_TASK_STOP,
    };
    pax_sasr_queue(&task);
    pax_sasr_join();
#if PAX_COMPILE_ASYNC_RENDERER == 2
    if (is_multithreaded) {
        ptq_destroy(args1.queue);
        pthread_mutex_destroy(&args1.rendermtx);
    }
#endif
    ptq_destroy(args0.queue);
    pthread_mutex_destroy(&args0.rendermtx);
}


// Queue a draw call.
static void pax_sasr_queue(pax_task_t *task) {
    ptq_send_block(args0.queue, task, NULL);
#if PAX_COMPILE_ASYNC_RENDERER == 2
    if (is_multithreaded) {
        ptq_send_block(args1.queue, task, NULL);
    }
#endif
}

// Worker thread function for software async renderer.
void *pax_sasr_worker(void *_args) {
    pax_sasr_worker_args_t *args = _args;

    while (1) {
        pax_task_t task;
        ptq_receive_block(args->queue, &task, &args->rendermtx);

        if (task.type == PAX_TASK_STOP) {
            pthread_mutex_unlock(&args->rendermtx);
            return NULL;
        } else if (task.type == PAX_TASK_QUAD) {
            if (task.use_shader) {
                args->renderfuncs->shaded_quad(task.buffer, task.color, task.quad_shape, &task.shader, task.quad_uvs);
            } else {
                args->renderfuncs->unshaded_quad(task.buffer, task.color, task.quad_shape);
            }
        } else if (task.type == PAX_TASK_RECT) {
            if (task.use_shader) {
                args->renderfuncs->shaded_rect(task.buffer, task.color, task.rect_shape, &task.shader, task.quad_uvs);
            } else {
                args->renderfuncs->unshaded_rect(task.buffer, task.color, task.rect_shape);
            }
        } else if (task.type == PAX_TASK_TRI) {
            if (task.use_shader) {
                args->renderfuncs->shaded_tri(task.buffer, task.color, task.tri_shape, &task.shader, task.tri_uvs);
            } else {
                args->renderfuncs->unshaded_tri(task.buffer, task.color, task.tri_shape);
            }
        } else if (task.type == PAX_TASK_LINE) {
            if (task.use_shader) {
                args->renderfuncs->shaded_line(task.buffer, task.color, task.line_shape, &task.shader, task.line_uvs);
            } else {
                args->renderfuncs->unshaded_line(task.buffer, task.color, task.line_shape);
            }
        } else if (task.type == PAX_TASK_SPRITE) {
            args->renderfuncs
                ->sprite(task.buffer, task.blit.top, task.blit_base_pos, task.blit.top_orientation, task.blit.top_pos);
        } else if (task.type == PAX_TASK_BLIT) {
            args->renderfuncs
                ->blit(task.buffer, task.blit.top, task.blit_base_pos, task.blit.top_orientation, task.blit.top_pos);
        } else if (task.type == PAX_TASK_BLIT_RAW) {
            args->renderfuncs->blit_raw(
                task.buffer,
                task.blit.top,
                task.blit.top_dims,
                task.blit_base_pos,
                task.blit.top_orientation,
                task.blit.top_pos
            );
        }

        pthread_mutex_unlock(&args->rendermtx);
    }
}


#if PAX_COMPILE_ASYNC_RENDERER == 2

// Read a single pixel from a raw buffer type by index.
__attribute__((always_inline)) static inline pax_col_t
    raw_get_pixel(void const *buf, uint8_t bpp, pax_vec2i dims, int index) {
    uint8_t const  *buf_8bpp  = buf;
    uint16_t const *buf_16bpp = buf;
    uint32_t const *buf_32bpp = buf;
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
__attribute__((always_inline)) static inline void sasr_blit_impl_2(
    bool              odd_scanline,
    pax_buf_t        *base,
    void const       *top,
    pax_vec2i         top_dims,
    pax_recti         base_pos,
    pax_orientation_t top_orientation,
    pax_vec2i         top_pos,
    bool              is_merge,
    bool              is_raw_buf,
    bool              is_pal_buf
) {
    // Determine copying parameters for top buffer.
    #if PAX_COMPILE_ORIENTATION
    // clang-format off
    int dx, dy; bool swap;
    int top_dx, top_dy, top_index;
    pax_vec2i top_pos0 = top_pos;
    switch (top_orientation & 7) {
        case PAX_O_UPRIGHT:         top_pos.x =              top_pos0.x; top_pos.y =              top_pos0.y; break;
        case PAX_O_ROT_CCW:         top_pos.y =              top_pos0.y; top_pos.x = top_dims.x-1-top_pos0.x; break;
        case PAX_O_ROT_HALF:        top_pos.x = top_dims.x-1-top_pos0.x; top_pos.y = top_dims.y-1-top_pos0.y; break;
        case PAX_O_ROT_CW:          top_pos.y = top_dims.y-1-top_pos0.y; top_pos.x =              top_pos0.x; break;
        case PAX_O_FLIP_H:          top_pos.x = top_dims.x-1-top_pos0.x; top_pos.y =              top_pos0.y; break;
        case PAX_O_ROT_CCW_FLIP_H:  top_pos.y = top_dims.y-1-top_pos0.y; top_pos.x = top_dims.x-1-top_pos0.x; break;
        case PAX_O_ROT_HALF_FLIP_H: top_pos.x =              top_pos0.x; top_pos.y = top_dims.y-1-top_pos0.y; break;
        case PAX_O_ROT_CW_FLIP_H:   top_pos.y =              top_pos0.y; top_pos.x =              top_pos0.x; break;
    }
    switch (top_orientation & 7) {
        case PAX_O_UPRIGHT:         dx =  1; dy =  1; swap = false; break;
        case PAX_O_ROT_CCW:         dx =  1; dy = -1; swap = true;  break;
        case PAX_O_ROT_HALF:        dx = -1; dy = -1; swap = false; break;
        case PAX_O_ROT_CW:          dx = -1; dy =  1; swap = true;  break;
        case PAX_O_FLIP_H:          dx = -1; dy =  1; swap = false; break;
        case PAX_O_ROT_CCW_FLIP_H:  dx = -1; dy = -1; swap = true;  break;
        case PAX_O_ROT_HALF_FLIP_H: dx =  1; dy = -1; swap = false; break;
        case PAX_O_ROT_CW_FLIP_H:   dx =  1; dy =  1; swap = true;  break;
    }
    // clang-format on
    if (swap) {
        top_dx = top_dims.x * dx;
        top_dy = dy;
    } else {
        top_dx = dx;
        top_dy = top_dims.x * dy;
    }
    top_index = top_pos.x + top_pos.y * top_dims.x;
    #else
    int top_dx    = 1;
    int top_dy    = top_dims.x;
    int top_index = top_pos.x + top_dims.x * top_pos.y;
    #endif

    // Determine copying parameters for bottom buffer.
    int base_dy    = base->width * 2 - base_pos.w;
    int base_index = base_pos.x + base->width * base_pos.y;

    int y = base_pos.y;
    if ((y & 1) != odd_scanline) {
        base_index += base->width;
        top_index  += top_dy;
        y++;
    }
    top_dy *= 2;
    top_dy -= base_pos.w * top_dx;
    for (; y < base_pos.y + base_pos.h; y += 2) {
        for (int x = base_pos.x; x < base_pos.x + base_pos.w; x++) {
            if (is_merge) {
                pax_buf_t const *_top     = top;
                pax_col_t        base_col = base->buf2col(base, base->getter(base, top_index));
                pax_col_t        top_col  = _top->buf2col(_top, _top->getter(_top, top_index));
                base->setter(base, base->col2buf(base, pax_col_merge(base_col, top_col)), base_index);
            } else if (is_raw_buf) {
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


// Draw a sprite; like a blit, but use color blending if applicable.
static void pax_sasr_sprite_impl(
    bool              odd_scanline,
    pax_buf_t        *base,
    pax_buf_t const  *top,
    pax_recti         base_pos,
    pax_orientation_t top_orientation,
    pax_vec2i         top_pos
) {
    sasr_blit_impl_2(
        odd_scanline,
        base,
        top,
        (pax_vec2i){top->width, top->height},
        base_pos,
        top_orientation,
        top_pos,
        1,
        0,
        0
    );
}

// Perform a buffer copying operation with an unmanaged user buffer.
__attribute__((noinline)) static void pax_sasr_blit_raw_impl(
    bool              odd_scanline,
    pax_buf_t        *base,
    void const       *top,
    pax_vec2i         top_dims,
    pax_recti         base_pos,
    pax_orientation_t top_orientation,
    pax_vec2i         top_pos
) {
    sasr_blit_impl_2(odd_scanline, base, top, top_dims, base_pos, top_orientation, top_pos, false, true, false);
}

// Perform a buffer copying operation with a PAX buffer.
static void pax_sasr_blit_impl(
    bool              odd_scanline,
    pax_buf_t        *base,
    pax_buf_t const  *top,
    pax_recti         base_pos,
    pax_orientation_t top_orientation,
    pax_vec2i         top_pos
) {
    if (top->type == base->type && false) {
        // Equal buffer types; no color conversion required.
        pax_sasr_blit_raw_impl(
            odd_scanline,
            base,
            top->buf,
            (pax_vec2i){top->width, top->height},
            base_pos,
            top_orientation,
            top_pos
        );
    } else if (PAX_IS_PALETTE(base->type) && !PAX_IS_PALETTE(top->type)) {
        // Bottom is palette, top is not; do palette special case.
        sasr_blit_impl_2(
            odd_scanline,
            base,
            top,
            (pax_vec2i){top->width, top->height},
            base_pos,
            top_orientation,
            top_pos,
            0,
            0,
            1
        );
    } else {
        // Different buffer types; color conversion required.
        sasr_blit_impl_2(
            odd_scanline,
            base,
            top,
            (pax_vec2i){top->width, top->height},
            base_pos,
            top_orientation,
            top_pos,
            0,
            0,
            0
        );
    }
}



// Draw a solid-colored line.
void pax_mcrw0_unshaded_line(pax_buf_t *buf, pax_col_t color, pax_linef shape) {
    paxmcr_line_unshaded(0, buf, color, shape.x0, shape.y0, shape.x1, shape.y1);
}

// Draw a solid-colored rectangle.
void pax_mcrw0_unshaded_rect(pax_buf_t *buf, pax_col_t color, pax_rectf shape) {
    paxmcr_rect_unshaded(0, buf, color, shape.x, shape.y, shape.w, shape.h);
}

// Draw a solid-colored quad.
void pax_mcrw0_unshaded_quad(pax_buf_t *buf, pax_col_t color, pax_quadf shape) {
    paxmcr_quad_unshaded(0, buf, color, shape.x0, shape.y0, shape.x1, shape.y1, shape.x2, shape.y2, shape.x3, shape.y3);
}

// Draw a solid-colored triangle.
void pax_mcrw0_unshaded_tri(pax_buf_t *buf, pax_col_t color, pax_trif shape) {
    paxmcr_tri_unshaded(0, buf, color, shape.x0, shape.y0, shape.x1, shape.y1, shape.x2, shape.y2);
}

// Draw a line with a shader.
void pax_mcrw0_shaded_line(pax_buf_t *buf, pax_col_t color, pax_linef shape, pax_shader_t const *shader, pax_linef uv) {
    paxmcr_line_shaded(0, buf, color, shader, shape.x0, shape.y0, shape.x1, shape.y1, uv.x0, uv.y0, uv.x1, uv.y1);
}

// Draw a rectangle with a shader.
void pax_mcrw0_shaded_rect(pax_buf_t *buf, pax_col_t color, pax_rectf shape, pax_shader_t const *shader, pax_quadf uv) {
    // clang-format off
    paxmcr_rect_shaded(
        0, buf, color, shader,
        shape.x, shape.y, shape.w, shape.h,
        uv.x0, uv.y0, uv.x1, uv.y1, uv.x2, uv.y2, uv.x3, uv.y3
    );
    // clang-format on
}

// Draw a quad with a shader.
void pax_mcrw0_shaded_quad(pax_buf_t *buf, pax_col_t color, pax_quadf shape, pax_shader_t const *shader, pax_quadf uv) {
    // clang-format off
    paxmcr_quad_shaded(
        0, buf, color, shader,
        shape.x0, shape.y0, shape.x1, shape.y1, shape.x2, shape.y2, shape.x3, shape.y3,
        uv.x0, uv.y0, uv.x1, uv.y1, uv.x2, uv.y2, uv.x3, uv.y3
    );
    // clang-format on
}

// Draw a triangle with a shader.
void pax_mcrw0_shaded_tri(pax_buf_t *buf, pax_col_t color, pax_trif shape, pax_shader_t const *shader, pax_trif uv) {
    // clang-format off
    paxmcr_tri_shaded(
        0, buf, color, shader,
        shape.x0, shape.y0, shape.x1, shape.y1, shape.x2, shape.y2,
        uv.x0, uv.y0, uv.x1, uv.y1, uv.x2, uv.y2
    );
    // clang-format on
}

// Draw a sprite; like a blit, but use color blending if applicable.
void pax_mcrw0_sprite(
    pax_buf_t *base, pax_buf_t const *top, pax_recti base_pos, pax_orientation_t top_orientation, pax_vec2i top_pos
) {
    pax_sasr_sprite_impl(0, base, top, base_pos, top_orientation, top_pos);
}

// Perform a buffer copying operation with a PAX buffer.
void pax_mcrw0_blit(
    pax_buf_t *base, pax_buf_t const *top, pax_recti base_pos, pax_orientation_t top_orientation, pax_vec2i top_pos
) {
    pax_sasr_blit_impl(0, base, top, base_pos, top_orientation, top_pos);
}

// Perform a buffer copying operation with an unmanaged user buffer.
void pax_mcrw0_blit_raw(
    pax_buf_t        *base,
    void const       *top,
    pax_vec2i         top_dims,
    pax_recti         base_pos,
    pax_orientation_t top_orientation,
    pax_vec2i         top_pos
) {
    pax_sasr_blit_raw_impl(0, base, top, top_dims, base_pos, top_orientation, top_pos);
}


// Draw a solid-colored line.
void pax_mcrw1_unshaded_line(pax_buf_t *buf, pax_col_t color, pax_linef shape) {
    paxmcr_line_unshaded(1, buf, color, shape.x0, shape.y0, shape.x1, shape.y1);
}

// Draw a solid-colored rectangle.
void pax_mcrw1_unshaded_rect(pax_buf_t *buf, pax_col_t color, pax_rectf shape) {
    paxmcr_rect_unshaded(1, buf, color, shape.x, shape.y, shape.w, shape.h);
}

// Draw a solid-colored quad.
void pax_mcrw1_unshaded_quad(pax_buf_t *buf, pax_col_t color, pax_quadf shape) {
    paxmcr_quad_unshaded(1, buf, color, shape.x0, shape.y0, shape.x1, shape.y1, shape.x2, shape.y2, shape.x3, shape.y3);
}

// Draw a solid-colored triangle.
void pax_mcrw1_unshaded_tri(pax_buf_t *buf, pax_col_t color, pax_trif shape) {
    paxmcr_tri_unshaded(1, buf, color, shape.x0, shape.y0, shape.x1, shape.y1, shape.x2, shape.y2);
}

// Draw a line with a shader.
void pax_mcrw1_shaded_line(pax_buf_t *buf, pax_col_t color, pax_linef shape, pax_shader_t const *shader, pax_linef uv) {
    paxmcr_line_shaded(1, buf, color, shader, shape.x0, shape.y0, shape.x1, shape.y1, uv.x0, uv.y0, uv.x1, uv.y1);
}

// Draw a rectangle with a shader.
void pax_mcrw1_shaded_rect(pax_buf_t *buf, pax_col_t color, pax_rectf shape, pax_shader_t const *shader, pax_quadf uv) {
    // clang-format off
    paxmcr_rect_shaded(
        1, buf, color, shader,
        shape.x, shape.y, shape.w, shape.h,
        uv.x0, uv.y0, uv.x1, uv.y1, uv.x2, uv.y2, uv.x3, uv.y3
    );
    // clang-format on
}

// Draw a quad with a shader.
void pax_mcrw1_shaded_quad(pax_buf_t *buf, pax_col_t color, pax_quadf shape, pax_shader_t const *shader, pax_quadf uv) {
    // clang-format off
    paxmcr_quad_shaded(
        1, buf, color, shader,
        shape.x0, shape.y0, shape.x1, shape.y1, shape.x2, shape.y2, shape.x3, shape.y3,
        uv.x0, uv.y0, uv.x1, uv.y1, uv.x2, uv.y2, uv.x3, uv.y3
    );
    // clang-format on
}

// Draw a triangle with a shader.
void pax_mcrw1_shaded_tri(pax_buf_t *buf, pax_col_t color, pax_trif shape, pax_shader_t const *shader, pax_trif uv) {
    // clang-format off
    paxmcr_tri_shaded(
        1, buf, color, shader,
        shape.x0, shape.y0, shape.x1, shape.y1, shape.x2, shape.y2,
        uv.x0, uv.y0, uv.x1, uv.y1, uv.x2, uv.y2
    );
    // clang-format on
}

// Draw a sprite; like a blit, but use color blending if applicable.
void pax_mcrw1_sprite(
    pax_buf_t *base, pax_buf_t const *top, pax_recti base_pos, pax_orientation_t top_orientation, pax_vec2i top_pos
) {
    pax_sasr_sprite_impl(1, base, top, base_pos, top_orientation, top_pos);
}

// Perform a buffer copying operation with a PAX buffer.
void pax_mcrw1_blit(
    pax_buf_t *base, pax_buf_t const *top, pax_recti base_pos, pax_orientation_t top_orientation, pax_vec2i top_pos
) {
    pax_sasr_blit_impl(1, base, top, base_pos, top_orientation, top_pos);
}

// Perform a buffer copying operation with an unmanaged user buffer.
void pax_mcrw1_blit_raw(
    pax_buf_t        *base,
    void const       *top,
    pax_vec2i         top_dims,
    pax_recti         base_pos,
    pax_orientation_t top_orientation,
    pax_vec2i         top_pos
) {
    pax_sasr_blit_raw_impl(1, base, top, top_dims, base_pos, top_orientation, top_pos);
}

#endif


// Draw a solid-colored line.
void pax_sasr_unshaded_line(pax_buf_t *buf, pax_col_t color, pax_linef shape) {
    pax_task_t task = {
        .buffer     = buf,
        .type       = PAX_TASK_LINE,
        .color      = color,
        .use_shader = false,
        .line_shape = shape,
    };
    pax_sasr_queue(&task);
}

// Draw a solid-colored rectangle.
void pax_sasr_unshaded_rect(pax_buf_t *buf, pax_col_t color, pax_rectf shape) {
    pax_task_t task = {
        .buffer     = buf,
        .type       = PAX_TASK_RECT,
        .color      = color,
        .use_shader = false,
        .rect_shape = shape,
    };
    pax_sasr_queue(&task);
}

// Draw a solid-colored quad.
void pax_sasr_unshaded_quad(pax_buf_t *buf, pax_col_t color, pax_quadf shape) {
    pax_task_t task = {
        .buffer     = buf,
        .type       = PAX_TASK_QUAD,
        .color      = color,
        .use_shader = false,
        .quad_shape = shape,
    };
    pax_sasr_queue(&task);
}

// Draw a solid-colored triangle.
void pax_sasr_unshaded_tri(pax_buf_t *buf, pax_col_t color, pax_trif shape) {
    pax_task_t task = {
        .buffer     = buf,
        .type       = PAX_TASK_TRI,
        .color      = color,
        .use_shader = false,
        .tri_shape  = shape,
    };
    pax_sasr_queue(&task);
}


// Draw a line with a shader.
void pax_sasr_shaded_line(pax_buf_t *buf, pax_col_t color, pax_linef shape, pax_shader_t const *shader, pax_linef uv) {
    pax_task_t task = {
        .buffer     = buf,
        .type       = PAX_TASK_LINE,
        .color      = color,
        .shader     = *shader,
        .use_shader = true,
        .line_shape = shape,
        .line_uvs   = uv,
    };
    pax_sasr_queue(&task);
}

// Draw a rectangle with a shader.
void pax_sasr_shaded_rect(pax_buf_t *buf, pax_col_t color, pax_rectf shape, pax_shader_t const *shader, pax_quadf uv) {
    pax_task_t task = {
        .buffer     = buf,
        .type       = PAX_TASK_RECT,
        .color      = color,
        .shader     = *shader,
        .use_shader = true,
        .rect_shape = shape,
        .quad_uvs   = uv,
    };
    pax_sasr_queue(&task);
}

// Draw a quad with a shader.
void pax_sasr_shaded_quad(pax_buf_t *buf, pax_col_t color, pax_quadf shape, pax_shader_t const *shader, pax_quadf uv) {
    pax_task_t task = {
        .buffer     = buf,
        .type       = PAX_TASK_QUAD,
        .color      = color,
        .shader     = *shader,
        .use_shader = true,
        .quad_shape = shape,
        .quad_uvs   = uv,
    };
    pax_sasr_queue(&task);
}

// Draw a triangle with a shader.
void pax_sasr_shaded_tri(pax_buf_t *buf, pax_col_t color, pax_trif shape, pax_shader_t const *shader, pax_trif uv) {
    pax_task_t task = {
        .buffer     = buf,
        .type       = PAX_TASK_TRI,
        .color      = color,
        .shader     = *shader,
        .use_shader = true,
        .tri_shape  = shape,
        .tri_uvs    = uv,
    };
    pax_sasr_queue(&task);
}


// Draw a sprite; like a blit, but use color blending if applicable.
void pax_sasr_sprite(
    pax_buf_t *base, pax_buf_t const *top, pax_recti base_pos, pax_orientation_t top_orientation, pax_vec2i top_pos
) {
    pax_task_t task = {
        .buffer = base,
        .type   = PAX_TASK_SPRITE,
        .blit   = {
            .top             = top,
            .top_orientation = top_orientation,
            .top_pos         = top_pos,
        },
        .blit_base_pos = base_pos,
    };
    pax_sasr_queue(&task);
}

// Perform a buffer copying operation with a PAX buffer.
void pax_sasr_blit(
    pax_buf_t *base, pax_buf_t const *top, pax_recti base_pos, pax_orientation_t top_orientation, pax_vec2i top_pos
) {
    pax_task_t task = {
        .buffer = base,
        .type   = PAX_TASK_BLIT,
        .blit   = {
            .top             = top,
            .top_orientation = top_orientation,
            .top_pos         = top_pos,
        },
        .blit_base_pos = base_pos,
    };
    pax_sasr_queue(&task);
}

// Perform a buffer copying operation with an unmanaged user buffer.
void pax_sasr_blit_raw(
    pax_buf_t        *base,
    void const       *top,
    pax_vec2i         top_dims,
    pax_recti         base_pos,
    pax_orientation_t top_orientation,
    pax_vec2i         top_pos
) {
    pax_task_t task = {
        .buffer = base,
        .type   = PAX_TASK_BLIT_RAW,
        .blit   = {
            .top             = top,
            .top_dims        = top_dims,
            .top_orientation = top_orientation,
            .top_pos         = top_pos,
        },
        .blit_base_pos = base_pos,
    };
    pax_sasr_queue(&task);
}


// Wait for all pending draw calls to finish.
void pax_sasr_join() {
    ptq_join(args0.queue, &args0.rendermtx);
    pthread_mutex_unlock(&args0.rendermtx);
#if PAX_COMPILE_ASYNC_RENDERER == 2
    if (is_multithreaded) {
        ptq_join(args1.queue, &args1.rendermtx);
        pthread_mutex_unlock(&args1.rendermtx);
    }
#endif
}



// Async software rendering functions.
pax_render_funcs_t const pax_render_funcs_softasync = {
    .unshaded_line = pax_sasr_unshaded_line,
    .unshaded_rect = pax_sasr_unshaded_rect,
    .unshaded_quad = pax_sasr_unshaded_quad,
    .unshaded_tri  = pax_sasr_unshaded_tri,
    .shaded_line   = pax_sasr_shaded_line,
    .shaded_rect   = pax_sasr_shaded_rect,
    .shaded_quad   = pax_sasr_shaded_quad,
    .shaded_tri    = pax_sasr_shaded_tri,
    .sprite        = pax_sasr_sprite,
    .blit          = pax_sasr_blit,
    .blit_raw      = pax_sasr_blit_raw,
    .join          = pax_sasr_join,
};

// Async software rendering functions.
pax_render_funcs_t const pax_render_funcs_mcr_thread0 = {
    .unshaded_line = pax_mcrw0_unshaded_line,
    .unshaded_rect = pax_mcrw0_unshaded_rect,
    .unshaded_quad = pax_mcrw0_unshaded_quad,
    .unshaded_tri  = pax_mcrw0_unshaded_tri,
    .shaded_line   = pax_mcrw0_shaded_line,
    .shaded_rect   = pax_mcrw0_shaded_rect,
    .shaded_quad   = pax_mcrw0_shaded_quad,
    .shaded_tri    = pax_mcrw0_shaded_tri,
    .sprite        = pax_mcrw0_sprite,
    .blit          = pax_mcrw0_blit,
    .blit_raw      = pax_mcrw0_blit_raw,
};

// Async software rendering functions.
pax_render_funcs_t const pax_render_funcs_mcr_thread1 = {
    .unshaded_line = pax_mcrw1_unshaded_line,
    .unshaded_rect = pax_mcrw1_unshaded_rect,
    .unshaded_quad = pax_mcrw1_unshaded_quad,
    .unshaded_tri  = pax_mcrw1_unshaded_tri,
    .shaded_line   = pax_mcrw1_shaded_line,
    .shaded_rect   = pax_mcrw1_shaded_rect,
    .shaded_quad   = pax_mcrw1_shaded_quad,
    .shaded_tri    = pax_mcrw1_shaded_tri,
    .sprite        = pax_mcrw1_sprite,
    .blit          = pax_mcrw1_blit,
    .blit_raw      = pax_mcrw1_blit_raw,
};

// Async software rendering engine.
pax_render_engine_t const pax_render_engine_softasync = {
    .init           = pax_sasr_init,
    .deinit         = pax_sasr_deinit,
    .implicit_dirty = true,
};
