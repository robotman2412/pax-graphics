
// SPDX-License-Identifier: MIT

#ifndef PAX_RENDERER_H
#define PAX_RENDERER_H

#include "pax_gfx.h"
#include "pax_matrix.h"

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus



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
// Blit a character of text in the bitmapped format.
void pax_dispatch_blit_char(pax_buf_t *buf, pax_col_t color, pax_vec2i pos, int scale, pax_text_rsdata_t rsdata);
// Draw a string of text in the bitmapped format.
void pax_dispatch_text(
    pax_buf_t        *buf,
    matrix_2d_t       matrix,
    pax_col_t         color,
    pax_font_t const *font,
    float             font_size,
    pax_vec2f         pos,
    char const       *text,
    size_t            text_len,
    pax_align_t       halign,
    pax_align_t       valign,
    ptrdiff_t         cursorpos
);



/* ==== Render engine management API ==== */

// Change the active render engine.
void pax_set_renderer(pax_render_engine_t const *engine, void *init_cookie);



#ifdef __cplusplus
} // extern "C"
#endif //__cplusplus

#endif // PAX_RENDERER_H
