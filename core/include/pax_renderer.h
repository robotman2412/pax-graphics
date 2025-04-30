
// SPDX-License-Identifier: MIT

#ifndef PAX_RENDERER_H
#define PAX_RENDERER_H

#include "pax_gfx.h"

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus



/* ==== Render engine definitions ==== */

// Render engine function table definition.
// The renderer gets coordinates after orientation and matrix transform, but before the clipping checks.
typedef struct {
    // Draw a solid-colored line.
    void (*unshaded_line)(pax_buf_t *buf, pax_col_t color, pax_linef shape);
    // Draw a solid-colored rectangle.
    void (*unshaded_rect)(pax_buf_t *buf, pax_col_t color, pax_rectf shape);
    // Draw a solid-colored quad.
    void (*unshaded_quad)(pax_buf_t *buf, pax_col_t color, pax_quadf shape);
    // Draw a solid-colored triangle.
    void (*unshaded_tri)(pax_buf_t *buf, pax_col_t color, pax_trif shape);

    // Draw a line with a shader.
    void (*shaded_line)(pax_buf_t *buf, pax_col_t color, pax_linef shape, pax_shader_t const *shader, pax_linef uv);
    // Draw a rectangle with a shader.
    void (*shaded_rect)(pax_buf_t *buf, pax_col_t color, pax_rectf shape, pax_shader_t const *shader, pax_quadf uv);
    // Draw a quad with a shader.
    void (*shaded_quad)(pax_buf_t *buf, pax_col_t color, pax_quadf shape, pax_shader_t const *shader, pax_quadf uv);
    // Draw a triangle with a shader.
    void (*shaded_tri)(pax_buf_t *buf, pax_col_t color, pax_trif shape, pax_shader_t const *shader, pax_trif uv);

    // Draw a sprite; like a blit, but use color blending if applicable.
    void (*sprite)(
        pax_buf_t *base, pax_buf_t const *top, pax_recti base_pos, pax_orientation_t top_orientation, pax_vec2i top_pos
    );
    // Perform a buffer copying operation with a PAX buffer.
    void (*blit)(
        pax_buf_t *base, pax_buf_t const *top, pax_recti base_pos, pax_orientation_t top_orientation, pax_vec2i top_pos
    );
    // Perform a buffer copying operation with an unmanaged user buffer.
    void (*blit_raw)(
        pax_buf_t        *base,
        void const       *top,
        pax_vec2i         top_dims,
        pax_recti         base_pos,
        pax_orientation_t top_orientation,
        pax_vec2i         top_pos
    );

    // Blit one or more characters of text in the bitmapped format.
    void (*blit_char)(pax_buf_t *buf, pax_col_t color, pax_vec2i pos, int scale, pax_text_rsdata_t rsdata);

    // Wait for all pending drawing operations to finish.
    void (*join)();
} pax_render_funcs_t;

// Render engine definition.
typedef struct {
    // Renderer init function; after this returns, the renderer must be ready.
    pax_render_funcs_t const *(*init)(void *init_cookie);
    // Renderer de-init function; clean up any implicitly-allocated resources.
    // Optional.
    void (*deinit)();
    // Have the dispatch run dirty marking on behalf of the renderer.
    bool implicit_dirty;
} pax_render_engine_t;



/* ==== Raw draw call dispatching API ==== */

// Draw a solid-colored line.
void pax_dispatch_unshaded_line(pax_buf_t *buf, pax_col_t color, pax_linef shape);
// Draw a solid-colored rectangle.
void pax_dispatch_unshaded_rect(pax_buf_t *buf, pax_col_t color, pax_rectf shape);
// Draw a solid-colored quad.
void pax_dispatch_unshaded_quad(pax_buf_t *buf, pax_col_t color, pax_quadf shape);
// Draw a solid-colored triangle.
void pax_dispatch_unshaded_tri(pax_buf_t *buf, pax_col_t color, pax_trif shape);

// Draw a line with a shader.
void pax_dispatch_shaded_line(
    pax_buf_t *buf, pax_col_t color, pax_linef shape, pax_shader_t const *shader, pax_linef uv
);
// Draw a rectangle with a shader.
void pax_dispatch_shaded_rect(
    pax_buf_t *buf, pax_col_t color, pax_rectf shape, pax_shader_t const *shader, pax_quadf uv
);
// Draw a quad with a shader.
void pax_dispatch_shaded_quad(
    pax_buf_t *buf, pax_col_t color, pax_quadf shape, pax_shader_t const *shader, pax_quadf uv
);
// Draw a triangle with a shader.
void pax_dispatch_shaded_tri(pax_buf_t *buf, pax_col_t color, pax_trif shape, pax_shader_t const *shader, pax_trif uv);

// Draw a sprite; like a blit, but use color blending if applicable.
void pax_dispatch_sprite(
    pax_buf_t *base, pax_buf_t const *top, pax_recti base_pos, pax_orientation_t top_orientation, pax_vec2i top_pos
);
// Perform a buffer copying operation with a PAX buffer.
void pax_dispatch_blit(
    pax_buf_t *base, pax_buf_t const *top, pax_recti base_pos, pax_orientation_t top_orientation, pax_vec2i top_pos
);
// Perform a buffer copying operation with an unmanaged user buffer.
void pax_dispatch_blit_raw(
    pax_buf_t        *base,
    void const       *top,
    pax_vec2i         top_dims,
    pax_recti         base_pos,
    pax_orientation_t top_orientation,
    pax_vec2i         top_pos
);
// Blit one or more characters of text in the bitmapped format.
void pax_dispatch_blit_char(pax_buf_t *buf, pax_col_t color, pax_vec2i pos, int scale, pax_text_rsdata_t rsdata);



/* ==== Render engine management API ==== */

// Change the active render engine.
void pax_set_renderer(pax_render_engine_t const *engine, void *init_cookie);



#ifdef __cplusplus
} // extern "C"
#endif //__cplusplus

#endif // PAX_RENDERER_H
