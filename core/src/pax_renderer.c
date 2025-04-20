
// SPDX-License-Identifier: MIT

#include "pax_renderer.h"

#include "renderer/pax_renderer_soft.h"

#define DEFAULT_RENDERER_ONLY !CONFIG_PAX_COMPILE_ASYNC_RENDERER && !CONFIG_PAX_COMPILE_ESP32P4_PPA_RENDERER



// Applies clipping before calling `pax_mark_dirty1`.
static inline void clipped_mark_dirty1(pax_buf_t *buf, int x, int y) {
    pax_recti clip = pax_get_clip(buf);
    if (x >= clip.x && x < clip.x + clip.h && y >= clip.y && y <= clip.y + clip.h) {
        pax_mark_dirty1(buf, x, y);
    }
}

// Applies clipping before calling `pax_mark_dirty2`.
static inline void clipped_mark_dirty2(pax_buf_t *buf, int x, int y, int w, int h) {
    pax_recti clip = pax_get_clip(buf);
    if (x < clip.x) {
        w += clip.x - x;
        x  = clip.x;
    }
    if (y < clip.y) {
        h += clip.y - y;
        y  = clip.y;
    }
    if (x + w > clip.x + clip.w) {
        w = clip.x + clip.w - x;
    }
    if (y + h > clip.y + clip.h) {
        h = clip.y + clip.h - y;
    }
    if (w > 0 && h > 0) {
        pax_mark_dirty2(buf, x, y, w, h);
    }
}



#if DEFAULT_RENDERER_ONLY
    #define implicit_dirty       true
    #define RENDERFUNC(function) pax_swr_##function
#else
static bool                       implicit_dirty = true;
static pax_render_engine_t const *renderer       = &pax_render_engine_soft;
static pax_render_funcs_t const  *renderfunc     = &pax_render_funcs_soft;
    #define RENDERFUNC(function) renderfunc->function
#endif

// Draw a solid-colored line.
void pax_dispatch_unshaded_line(pax_buf_t *buf, pax_col_t color, pax_linef shape) {
    if (implicit_dirty) {
        clipped_mark_dirty1(buf, shape.x0, shape.y0);
        clipped_mark_dirty1(buf, shape.x1, shape.y1);
    }
    RENDERFUNC(unshaded_line)(buf, color, shape);
}

// Draw a solid-colored rectangle.
void pax_dispatch_unshaded_rect(pax_buf_t *buf, pax_col_t color, pax_rectf shape) {
    if (implicit_dirty) {
        clipped_mark_dirty2(buf, shape.x, shape.y, shape.w, shape.h);
    }
    RENDERFUNC(unshaded_rect)(buf, color, shape);
}

// Draw a solid-colored quad.
void pax_dispatch_unshaded_quad(pax_buf_t *buf, pax_col_t color, pax_quadf shape) {
    if (implicit_dirty) {
        clipped_mark_dirty1(buf, shape.x0, shape.y0);
        clipped_mark_dirty1(buf, shape.x1, shape.y1);
        clipped_mark_dirty1(buf, shape.x2, shape.y2);
        clipped_mark_dirty1(buf, shape.x3, shape.y3);
    }
    RENDERFUNC(unshaded_quad)(buf, color, shape);
}

// Draw a solid-colored triangle.
void pax_dispatch_unshaded_tri(pax_buf_t *buf, pax_col_t color, pax_trif shape) {
    if (implicit_dirty) {
        clipped_mark_dirty1(buf, shape.x0, shape.y0);
        clipped_mark_dirty1(buf, shape.x1, shape.y1);
        clipped_mark_dirty1(buf, shape.x2, shape.y2);
    }
    RENDERFUNC(unshaded_tri)(buf, color, shape);
}


// Draw a line with a shader.
void pax_dispatch_shaded_line(
    pax_buf_t *buf, pax_col_t color, pax_linef shape, pax_shader_t const *shader, pax_linef uv
) {
    if (implicit_dirty) {
        clipped_mark_dirty1(buf, shape.x0, shape.y0);
        clipped_mark_dirty1(buf, shape.x1, shape.y1);
    }
    RENDERFUNC(shaded_line)(buf, color, shape, shader, uv);
}

// Draw a rectangle with a shader.
void pax_dispatch_shaded_rect(
    pax_buf_t *buf, pax_col_t color, pax_rectf shape, pax_shader_t const *shader, pax_quadf uv
) {
    if (implicit_dirty) {
        clipped_mark_dirty2(buf, shape.x, shape.y, shape.w, shape.h);
    }
    RENDERFUNC(shaded_rect)(buf, color, shape, shader, uv);
}

// Draw a quad with a shader.
void pax_dispatch_shaded_quad(
    pax_buf_t *buf, pax_col_t color, pax_quadf shape, pax_shader_t const *shader, pax_quadf uv
) {
    if (implicit_dirty) {
        clipped_mark_dirty1(buf, shape.x0, shape.y0);
        clipped_mark_dirty1(buf, shape.x1, shape.y1);
        clipped_mark_dirty1(buf, shape.x2, shape.y2);
        clipped_mark_dirty1(buf, shape.x3, shape.y3);
    }
    RENDERFUNC(shaded_quad)(buf, color, shape, shader, uv);
}

// Draw a triangle with a shader.
void pax_dispatch_shaded_tri(pax_buf_t *buf, pax_col_t color, pax_trif shape, pax_shader_t const *shader, pax_trif uv) {
    if (implicit_dirty) {
        clipped_mark_dirty1(buf, shape.x0, shape.y0);
        clipped_mark_dirty1(buf, shape.x1, shape.y1);
        clipped_mark_dirty1(buf, shape.x2, shape.y2);
    }
    RENDERFUNC(shaded_tri)(buf, color, shape, shader, uv);
}


// Draw a sprite; like a blit, but use color blending if applicable.
void pax_dispatch_sprite(
    pax_buf_t *base, pax_buf_t const *top, pax_recti base_pos, pax_orientation_t top_orientation, pax_vec2i top_pos
) {
    if (implicit_dirty) {
        clipped_mark_dirty2(base, base_pos.x, base_pos.y, base_pos.w, base_pos.h);
    }
    RENDERFUNC(sprite)(base, top, base_pos, top_orientation, top_pos);
}

// Perform a buffer copying operation with a PAX buffer.
void pax_dispatch_blit(
    pax_buf_t *base, pax_buf_t const *top, pax_recti base_pos, pax_orientation_t top_orientation, pax_vec2i top_pos
) {
    if (implicit_dirty) {
        clipped_mark_dirty2(base, base_pos.x, base_pos.y, base_pos.w, base_pos.h);
    }
    RENDERFUNC(blit)(base, top, base_pos, top_orientation, top_pos);
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
        clipped_mark_dirty2(base, base_pos.x, base_pos.y, base_pos.w, base_pos.h);
    }
    RENDERFUNC(blit_raw)(base, top, top_dims, base_pos, top_orientation, top_pos);
}

// Blit one or more characters of text in the bitmapped format.
void pax_dispatch_blit_char(pax_buf_t *buf, pax_col_t color, pax_vec2i pos, int scale, pax_text_rsdata_t rsdata) {
    if (implicit_dirty) {
        clipped_mark_dirty2(buf, pos.x, pos.y, rsdata.w, rsdata.h);
    }
    RENDERFUNC(blit_char)(buf, color, pos, scale, rsdata);
}



#if DEFAULT_RENDERER_ONLY

// Wait for all pending drawing operations to finish.
void pax_join() {
    // Nothing to do; default renderer is completely synchronous.
}


// Change the active render engine.
void pax_set_renderer(pax_render_engine_t const *new_renderer, void *init_cookie) {
    PAX_LOGW("pax", "Only default renderer is compiled; pax_set_renderer call ignored");
}

#else

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
