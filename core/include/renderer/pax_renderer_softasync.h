
// SPDX-License-Identifier: MIT

#ifndef PAX_RENDERER_SOFTASYNC_H
#define PAX_RENDERER_SOFTASYNC_H

#include "pax_gfx.h"
#include "pax_renderer.h"
#include "ptq.h"

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

// Args for software async renderer.
typedef struct {
    ptq_queue_t               queue;
    pax_render_funcs_t const *renderfuncs;
    pthread_mutex_t           rendermtx;
} pax_sasr_worker_args_t;


// Worker thread function for software async renderer.
void *pax_sasr_worker(void *renderfuncs);


// Draw a solid-colored line.
void pax_sasr_unshaded_line(pax_buf_t *buf, pax_col_t color, pax_linef shape);
// Draw a solid-colored rectangle.
void pax_sasr_unshaded_rect(pax_buf_t *buf, pax_col_t color, pax_rectf shape);
// Draw a solid-colored quad.
void pax_sasr_unshaded_quad(pax_buf_t *buf, pax_col_t color, pax_quadf shape);
// Draw a solid-colored triangle.
void pax_sasr_unshaded_tri(pax_buf_t *buf, pax_col_t color, pax_trif shape);

// Draw a line with a shader.
void pax_sasr_shaded_line(pax_buf_t *buf, pax_col_t color, pax_linef shape, pax_shader_t const *shader, pax_linef uv);
// Draw a rectangle with a shader.
void pax_sasr_shaded_rect(pax_buf_t *buf, pax_col_t color, pax_rectf shape, pax_shader_t const *shader, pax_quadf uv);
// Draw a quad with a shader.
void pax_sasr_shaded_quad(pax_buf_t *buf, pax_col_t color, pax_quadf shape, pax_shader_t const *shader, pax_quadf uv);
// Draw a triangle with a shader.
void pax_sasr_shaded_tri(pax_buf_t *buf, pax_col_t color, pax_trif shape, pax_shader_t const *shader, pax_trif uv);

// Draw a sprite; like a blit, but use color blending if applicable.
void pax_sasr_sprite(
    pax_buf_t *base, pax_buf_t const *top, pax_recti base_pos, pax_orientation_t top_orientation, pax_vec2i top_pos
);
// Perform a buffer copying operation with a PAX buffer.
void pax_sasr_blit(
    pax_buf_t *base, pax_buf_t const *top, pax_recti base_pos, pax_orientation_t top_orientation, pax_vec2i top_pos
);
// Perform a buffer copying operation with an unmanaged user buffer.
void pax_sasr_blit_raw(
    pax_buf_t        *base,
    void const       *top,
    pax_vec2i         top_dims,
    pax_recti         base_pos,
    pax_orientation_t top_orientation,
    pax_vec2i         top_pos
);
// Blit one or more characters of text in the bitmapped format.
void pax_sasr_blit_char(pax_buf_t *buf, pax_col_t color, pax_vec2i pos, int scale, pax_text_rsdata_t rsdata);

// Wait for all pending draw calls to finish.
void pax_sasr_join();


// Async software rendering functions.
extern pax_render_funcs_t const  pax_render_funcs_softasync;
// Software rendering functions for worker thread 1/2.
extern pax_render_funcs_t const  pax_render_funcs_mcr_thread0;
// Software rendering functions for worker thread 2/2.
extern pax_render_funcs_t const  pax_render_funcs_mcr_thread1;
// Async software rendering engine.
extern pax_render_engine_t const pax_render_engine_softasync;


#ifdef __cplusplus
} // extern "C"
#endif //__cplusplus

#endif // PAX_RENDERER_SOFTASYNC_H
