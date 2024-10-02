
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

        if (task.type == PAX_TASK_QUAD) {
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
        } else if (task.type == PAX_TASK_STOP) {
            pthread_mutex_unlock(&args->rendermtx);
            return NULL;
        }

        pthread_mutex_unlock(&args->rendermtx);
    }
}


#if PAX_COMPILE_ASYNC_RENDERER == 2

// Draw a solid-colored line.
void pax_mcrw0_unshaded_line(pax_buf_t *buf, pax_col_t color, pax_linef shape) {
    // paxmcr_line_unshaded(0, buf, color, shape.x0, shape.y0, shape.x1, shape.y1);
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
    // paxmcr_line_shaded(0, buf, color, shader, shape.x0, shape.y0, shape.x1, shape.y1, uv.x0, uv.y0, uv.x1, uv.y1);
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


// Draw a solid-colored line.
void pax_mcrw1_unshaded_line(pax_buf_t *buf, pax_col_t color, pax_linef shape) {
    // paxmcr_line_unshaded(1, buf, color, shape.x0, shape.y0, shape.x1, shape.y1);
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
    // paxmcr_line_shaded(1, buf, color, shader, shape.x0, shape.y0, shape.x1, shape.y1, uv.x0, uv.y0, uv.x1, uv.y1);
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


// Perform a buffer copying operation with a PAX buffer.
void pax_sasr_blit(
    pax_buf_t *base, pax_buf_t *top, pax_recti base_pos, pax_orientation_t top_orientation, pax_vec2i top_pos
) {
    // TODO.
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
    // TODO.
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
};

// Async software rendering engine.
pax_render_engine_t const pax_render_engine_softasync = {
    .init           = pax_sasr_init,
    .deinit         = pax_sasr_deinit,
    .implicit_dirty = true,
};
