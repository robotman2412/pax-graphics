
// SPDX-License-Identifier: MIT

#include "pax_renderer.h"

#include "pax_internal.h"
#include "renderer/pax_renderer_soft.h"

#define DEFAULT_RENDERER_ONLY !PAX_COMPILE_ASYNC_RENDERER && !PAX_COMPILE_ESP32P4_PPA_RENDERER



#if DEFAULT_RENDERER_ONLY

// Draw a solid-colored line.
void pax_dispatch_unshaded_line(pax_buf_t *buf, pax_col_t color, pax_linef shape) {
    pax_swr_unshaded_line(buf, color, shape);
}

// Draw a solid-colored rectangle.
void pax_dispatch_unshaded_rect(pax_buf_t *buf, pax_col_t color, pax_rectf shape) {
    pax_swr_unshaded_rect(buf, color, shape);
}

// Draw a solid-colored quad.
void pax_dispatch_unshaded_quad(pax_buf_t *buf, pax_col_t color, pax_quadf shape) {
    pax_swr_unshaded_quad(buf, color, shape);
}

// Draw a solid-colored triangle.
void pax_dispatch_unshaded_tri(pax_buf_t *buf, pax_col_t color, pax_trif shape) {
    pax_swr_unshaded_tri(buf, color, shape);
}


// Draw a line with a shader.
void pax_dispatch_shaded_line(
    pax_buf_t *buf, pax_col_t color, pax_linef shape, pax_shader_t const *shader, pax_linef uv
) {
    pax_swr_shaded_line(buf, color, shape, shader, uv);
}

// Draw a rectangle with a shader.
void pax_dispatch_shaded_rect(
    pax_buf_t *buf, pax_col_t color, pax_rectf shape, pax_shader_t const *shader, pax_quadf uv
) {
    pax_swr_shaded_rect(buf, color, shape, shader, uv);
}

// Draw a quad with a shader.
void pax_dispatch_shaded_quad(
    pax_buf_t *buf, pax_col_t color, pax_quadf shape, pax_shader_t const *shader, pax_quadf uv
) {
    pax_swr_shaded_quad(buf, color, shape, shader, uv);
}

// Draw a triangle with a shader.
void pax_dispatch_shaded_tri(pax_buf_t *buf, pax_col_t color, pax_trif shape, pax_shader_t const *shader, pax_trif uv) {
    pax_swr_shaded_tri(buf, color, shape, shader, uv);
}


// Perform a buffer copying operation with a PAX buffer.
void pax_dispatch_blit(
    pax_buf_t *base, pax_buf_t *top, pax_recti base_pos, pax_orientation_t top_orientation, pax_vec2i top_pos
) {
    pax_swr_blit(base, top, base_pos, top_orientation, top_pos);
}

// Perform a buffer copying operation with an unmanaged user buffer.
void pax_dispatch_blit_raw(
    pax_buf_t        *base,
    void const       *top,
    pax_vec2i         top_dims,
    pax_recti         base_pos,
    pax_orientation_t top_orientation,
    pax_vec2i         top_pos
) {
    pax_swr_blit_raw(base, top, top_dims, base_pos, top_orientation, top_pos);
}


// Wait for all pending drawing operations to finish.
void pax_join() {
    // Synchronous renderer; nothing to do.
}

#else

static bool                       implicit_dirty = true;
static pax_render_engine_t const *renderer       = &pax_render_engine_soft;
static pax_render_funcs_t const  *renderfunc     = &pax_render_funcs_soft;

// Draw a solid-colored line.
void pax_dispatch_unshaded_line(pax_buf_t *buf, pax_col_t color, pax_linef shape) {
    if (implicit_dirty) {
        pax_mark_dirty1(buf, shape.x0, shape.y0);
        pax_mark_dirty1(buf, shape.x1, shape.y1);
    }
    renderfunc->unshaded_line(buf, color, shape);
}

// Draw a solid-colored rectangle.
void pax_dispatch_unshaded_rect(pax_buf_t *buf, pax_col_t color, pax_rectf shape) {
    if (implicit_dirty) {
        pax_mark_dirty2(buf, shape.x, shape.y, shape.w, shape.h);
    }
    renderfunc->unshaded_rect(buf, color, shape);
}

// Draw a solid-colored quad.
void pax_dispatch_unshaded_quad(pax_buf_t *buf, pax_col_t color, pax_quadf shape) {
    if (implicit_dirty) {
        pax_mark_dirty1(buf, shape.x0, shape.y0);
        pax_mark_dirty1(buf, shape.x1, shape.y1);
        pax_mark_dirty1(buf, shape.x2, shape.y2);
        pax_mark_dirty1(buf, shape.x3, shape.y3);
    }
    renderfunc->unshaded_quad(buf, color, shape);
}

// Draw a solid-colored triangle.
void pax_dispatch_unshaded_tri(pax_buf_t *buf, pax_col_t color, pax_trif shape) {
    if (implicit_dirty) {
        pax_mark_dirty1(buf, shape.x0, shape.y0);
        pax_mark_dirty1(buf, shape.x1, shape.y1);
        pax_mark_dirty1(buf, shape.x2, shape.y2);
    }
    renderfunc->unshaded_tri(buf, color, shape);
}


// Draw a line with a shader.
void pax_dispatch_shaded_line(
    pax_buf_t *buf, pax_col_t color, pax_linef shape, pax_shader_t const *shader, pax_linef uv
) {
    if (implicit_dirty) {
        pax_mark_dirty1(buf, shape.x0, shape.y0);
        pax_mark_dirty1(buf, shape.x1, shape.y1);
    }
    renderfunc->shaded_line(buf, color, shape, shader, uv);
}

// Draw a rectangle with a shader.
void pax_dispatch_shaded_rect(
    pax_buf_t *buf, pax_col_t color, pax_rectf shape, pax_shader_t const *shader, pax_quadf uv
) {
    if (implicit_dirty) {
        pax_mark_dirty2(buf, shape.x, shape.y, shape.w, shape.h);
    }
    renderfunc->shaded_rect(buf, color, shape, shader, uv);
}

// Draw a quad with a shader.
void pax_dispatch_shaded_quad(
    pax_buf_t *buf, pax_col_t color, pax_quadf shape, pax_shader_t const *shader, pax_quadf uv
) {
    if (implicit_dirty) {
        pax_mark_dirty1(buf, shape.x0, shape.y0);
        pax_mark_dirty1(buf, shape.x1, shape.y1);
        pax_mark_dirty1(buf, shape.x2, shape.y2);
        pax_mark_dirty1(buf, shape.x3, shape.y3);
    }
    renderfunc->shaded_quad(buf, color, shape, shader, uv);
}

// Draw a triangle with a shader.
void pax_dispatch_shaded_tri(pax_buf_t *buf, pax_col_t color, pax_trif shape, pax_shader_t const *shader, pax_trif uv) {
    if (implicit_dirty) {
        pax_mark_dirty1(buf, shape.x0, shape.y0);
        pax_mark_dirty1(buf, shape.x1, shape.y1);
        pax_mark_dirty1(buf, shape.x2, shape.y2);
    }
    renderfunc->shaded_tri(buf, color, shape, shader, uv);
}


// Perform a buffer copying operation with a PAX buffer.
void pax_dispatch_blit(
    pax_buf_t *base, pax_buf_t *top, pax_recti base_pos, pax_orientation_t top_orientation, pax_vec2i top_pos
) {
    if (implicit_dirty) {
        pax_mark_dirty2(base, base_pos.x, base_pos.y, base_pos.w, base_pos.h);
    }
    renderfunc->blit(base, top, base_pos, top_orientation, top_pos);
}

// Perform a buffer copying operation with an unmanaged user buffer.
void pax_dispatch_blit_raw(
    pax_buf_t        *base,
    void const       *top,
    pax_vec2i         top_dims,
    pax_recti         base_pos,
    pax_orientation_t top_orientation,
    pax_vec2i         top_pos
) {
    if (implicit_dirty) {
        pax_mark_dirty2(base, base_pos.x, base_pos.y, base_pos.w, base_pos.h);
    }
    renderfunc->blit_raw(base, top, top_dims, base_pos, top_orientation, top_pos);
}


// Wait for all pending drawing operations to finish.
void pax_join() {
    if (renderfunc->join) {
        renderfunc->join();
    }
}


// Change the active render engine.
void pax_set_renderer(pax_render_engine_t const *new_renderer, void *init_cookie) {
    if (renderer->deinit) {
        renderer->deinit();
    }
    implicit_dirty = new_renderer->implicit_dirty;
    renderfunc     = new_renderer->init(init_cookie);
    renderer       = new_renderer;
}

#endif
