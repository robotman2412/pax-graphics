
// SPDX-License-Identifier: MIT

#ifndef PAX_MISC_H
#define PAX_MISC_H

#include "pax_types.h"

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus


// Set a pixel, merging with alpha.
void      pax_merge_pixel(pax_buf_t *buf, pax_col_t color, int x, int y);
// Set a pixel.
void      pax_set_pixel(pax_buf_t *buf, pax_col_t color, int x, int y);
// Get a pixel (does palette lookup if applicable).
pax_col_t pax_get_pixel(pax_buf_t const *buf, int x, int y);
// Set a pixel without color conversion.
void      pax_set_pixel_raw(pax_buf_t *buf, pax_col_t color, int x, int y);
// Get a pixel without color conversion.
pax_col_t pax_get_pixel_raw(pax_buf_t const *buf, int x, int y);

// Draws an image at the image's normal size.
void pax_draw_image(pax_buf_t *buf, pax_buf_t const *image, float x, float y);
// Draw an image with a prespecified size.
void pax_draw_image_sized(pax_buf_t *buf, pax_buf_t const *image, float x, float y, float width, float height);
// Draws an image at the image's normal size.
// Assumes the image is completely opaque, any transparent parts are drawn opaque.
void pax_draw_image_op(pax_buf_t *buf, pax_buf_t const *image, float x, float y);
// Draw an image with a prespecified size.
// Assumes the image is completely opaque, any transparent parts are drawn opaque.
void pax_draw_image_sized_op(pax_buf_t *buf, pax_buf_t const *image, float x, float y, float width, float height);

// Fill the background.
void pax_background(pax_buf_t *buf, pax_col_t color);
// Scroll the buffer, filling with a placeholder color.
void pax_buf_scroll(pax_buf_t *buf, pax_col_t placeholder, int x, int y);


#ifdef __cplusplus
}
#endif //__cplusplus

#endif // PAX_MISC_H
