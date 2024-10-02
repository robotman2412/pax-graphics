
// SPDX-License-Identifier: MIT

#include "pax_internal.h"



// Set a pixel, merging with alpha.
void pax_merge_pixel(pax_buf_t *buf, pax_col_t color, int x, int y) {
    PAX_BUF_CHECK(buf);

#if PAX_COMPILE_ORIENTATION
    pax_vec2i tmp = pax_orient_det_vec2i(buf, (pax_vec2i){x, y});
    x             = tmp.x;
    y             = tmp.y;
#endif

    if (x < 0 || x >= buf->width || y < 0 || y >= buf->height) {
        // Out of bounds error.
        PAX_ERROR(PAX_ERR_BOUNDS);
    }

    int index = x + y * buf->width;
    if (PAX_IS_PALETTE(buf->type)) {
        // Palette colors don't have conversion.
        if (color & 0xff000000)
            buf->setter(buf, color, index);
    } else if (color >= 0xff000000) {
        // Opaque colors don't need alpha blending.
        buf->setter(buf, buf->col2buf(buf, color), index);
    } else if (color & 0xff000000) {
        // Non-transparent colors will be blended normally.
        pax_col_t base = buf->buf2col(buf, buf->getter(buf, index));
        buf->setter(buf, buf->col2buf(buf, pax_col_merge(base, color)), index);
    }
}

// Set a pixel.
void pax_set_pixel(pax_buf_t *buf, pax_col_t color, int x, int y) {
    PAX_BUF_CHECK(buf);

#if PAX_COMPILE_ORIENTATION
    pax_vec2i tmp = pax_orient_det_vec2i(buf, (pax_vec2i){x, y});
    x             = tmp.x;
    y             = tmp.y;
#endif

    if (x < 0 || x >= buf->width || y < 0 || y >= buf->height) {
        // Out of bounds error.
        PAX_ERROR(PAX_ERR_BOUNDS);
    }

    int index = x + y * buf->width;
    if (PAX_IS_PALETTE(buf->type)) {
        // Palette colors don't have conversion.
        buf->setter(buf, color, index);
    } else {
        // But all other colors do have a conversion.
        buf->setter(buf, buf->col2buf(buf, color), index);
    }
}

// Get a pixel.
pax_col_t pax_get_pixel(pax_buf_t const *buf, int x, int y) {
    PAX_BUF_CHECK(buf, 0);

#if PAX_COMPILE_ORIENTATION
    pax_vec2i tmp = pax_orient_det_vec2i(buf, (pax_vec2i){x, y});
    x             = tmp.x;
    y             = tmp.y;
#endif

    if (x < 0 || x >= buf->width || y < 0 || y >= buf->height) {
        // Out of bounds error.
        PAX_ERROR(PAX_ERR_BOUNDS, 0);
    }
    return buf->buf2col(buf, buf->getter(buf, x + y * buf->width));
}

// Set a pixel without color conversion.
void pax_set_pixel_raw(pax_buf_t *buf, pax_col_t color, int x, int y) {
    PAX_BUF_CHECK(buf);

#if PAX_COMPILE_ORIENTATION
    pax_vec2i tmp = pax_orient_det_vec2i(buf, (pax_vec2i){x, y});
    x             = tmp.x;
    y             = tmp.y;
#endif

    if (x < 0 || x >= buf->width || y < 0 || y >= buf->height) {
        // Out of bounds error.
        PAX_ERROR(PAX_ERR_BOUNDS);
    }

    int index = x + y * buf->width;
    // Don't do any color conversion.
    buf->setter(buf, color, index);
}

// Get a pixel without color conversion.
pax_col_t pax_get_pixel_raw(pax_buf_t const *buf, int x, int y) {
    PAX_BUF_CHECK(buf, 0);

#if PAX_COMPILE_ORIENTATION
    pax_vec2i tmp = pax_orient_det_vec2i(buf, (pax_vec2i){x, y});
    x             = tmp.x;
    y             = tmp.y;
#endif

    if (x < 0 || x >= buf->width || y < 0 || y >= buf->height) {
        // Out of bounds error.
        PAX_ERROR(PAX_ERR_BOUNDS, 0);
    }
    return buf->getter(buf, x + y * buf->width);
}


// Draws an image at the image's normal size.
void pax_draw_image(pax_buf_t *buf, pax_buf_t const *image, float x, float y) {
    PAX_BUF_CHECK(buf);
    PAX_BUF_CHECK(image);
    pax_draw_image_sized(buf, image, x, y, image->width, image->height);
}

// Draw an image with a prespecified size.
void pax_draw_image_sized(pax_buf_t *buf, pax_buf_t const *image, float x, float y, float width, float height) {
    PAX_BUF_CHECK(buf);
    PAX_BUF_CHECK(image);
    if (PAX_IS_ALPHA(image->type)) {
        pax_shade_rect(buf, -1, &PAX_SHADER_TEXTURE(image), NULL, x, y, width, height);
    } else {
        pax_shade_rect(buf, -1, &PAX_SHADER_TEXTURE_OP(image), NULL, x, y, width, height);
    }
}

// Draws an image at the image's normal size.
// Assumes the image is completely opaque, any transparent parts are drawn opaque.
void pax_draw_image_op(pax_buf_t *buf, pax_buf_t const *image, float x, float y) {
    PAX_BUF_CHECK(buf);
    PAX_BUF_CHECK(image);
    pax_draw_image_sized_op(buf, image, x, y, image->width, image->height);
}

// Draw an image with a prespecified size.
// Assumes the image is completely opaque, any transparent parts are drawn opaque.
void pax_draw_image_sized_op(pax_buf_t *buf, pax_buf_t const *image, float x, float y, float width, float height) {
    PAX_BUF_CHECK(buf);
    PAX_BUF_CHECK(image);
    pax_shade_rect(buf, -1, &PAX_SHADER_TEXTURE_OP(image), NULL, x, y, width, height);
}


// Fill the background.
PAX_PERF_CRITICAL_ATTR void pax_background(pax_buf_t *buf, pax_col_t color) {
    PAX_BUF_CHECK(buf);
    // TODO: Make into render callback.

#if PAX_COMPILE_MCR
    pax_join();
#endif

    uint32_t value;
    if (PAX_IS_PALETTE(buf->type)) {
        if (color > buf->palette_size)
            value = 0;
        else
            value = color;
    } else {
        value = buf->col2buf(buf, color);
    }

    if (value == 0) {
        memset(buf->buf, 0, PAX_BUF_CALC_SIZE(buf->width, buf->height, buf->type));
    } else if (buf->bpp == 16) {
        if (buf->reverse_endianness) {
            value = pax_rev_endian_16(value);
        }
        // Fill 16bpp parts.
        for (size_t i = 0; i < buf->width * buf->height; i++) {
            buf->buf_16bpp[i] = value;
        }
    } else if (buf->bpp == 32) {
        if (buf->reverse_endianness) {
            value = pax_rev_endian_32(value);
        }
        // Fill 32bpp parts.
        for (size_t i = 0; i < buf->width * buf->height; i++) {
            buf->buf_32bpp[i] = value;
        }
    } else {
        // Fill <=8bpp parts.
        if (buf->bpp == 1)
            value = -value;
        else if (buf->bpp == 2)
            value = value * 0x55;
        else if (buf->bpp == 4)
            value = value * 0x11;
        size_t limit = (7 + buf->width * buf->height * buf->bpp) / 8;
        for (size_t i = 0; i < limit; i++) {
            buf->buf_8bpp[i] = value;
        }
    }

    pax_mark_dirty0(buf);
}

// Scroll the buffer, filling with a placeholder color.
void pax_buf_scroll(pax_buf_t *buf, pax_col_t placeholder, int x, int y) {
    PAX_BUF_CHECK(buf);
#if PAX_COMPILE_MCR
    pax_join();
#endif
    // TODO: Make into render callback.

#if PAX_COMPILE_ORIENTATION
    {
        int x0 = x, y0 = y;
        // Fix direction of scrolling.
        switch (buf->orientation & 3) {
            default:
            case 0: break;
            case 1:
                y = -x0;
                x = y0;
                break;
            case 2:
                x = -x0;
                y = -y0;
                break;
            case 3:
                y = x0;
                x = -y0;
                break;
        }
        if (buf->orientation & 4) {
            x = -x;
        }
    }
#endif

    // Edge case: Scrolls too far.
    if (x >= buf->width || x <= -buf->width || y >= buf->height || y <= -buf->height) {
        pax_background(buf, placeholder);
        return;
    }

    // Pixel index offset for the copy.
    ptrdiff_t off   = x + y * buf->width;
    // Number of pixels that must be copied.
    size_t    count = buf->width * buf->height - labs(off);

    // Bit index version of the offset.
    ptrdiff_t bit_off   = PAX_GET_BPP(buf->type) * off;
    // Number of bits to copy.
    size_t    bit_count = PAX_GET_BPP(buf->type) * count;

    if ((bit_off & 7) == 0) {
        // If bit offset lines up to a byte, use memmove.
        ptrdiff_t byte_off   = bit_off / 8;
        size_t    byte_count = bit_count / 8;

        if (byte_off > 0) {
            memmove(buf->buf_8bpp + byte_off, buf->buf_8bpp, byte_count);
        } else {
            memmove(buf->buf_8bpp, buf->buf_8bpp - byte_off, byte_count);
        }

    } else {
        // If it does not, an expensive copy must be performed.
        if (off > 0) {
            for (ptrdiff_t i = count - 1; i >= 0; i--) {
                pax_col_t value = buf->getter(buf, off + i);
                buf->setter(buf, value, i);
            }
        } else {
            for (ptrdiff_t i = 0; i < count; i++) {
                pax_col_t value = buf->getter(buf, i);
                buf->setter(buf, value, off + i);
            }
        }
    }

#if PAX_COMPILE_ORIENTATION
    // Ignore orientation for a moment.
    int rot          = buf->orientation;
    buf->orientation = 0;
#endif

    // Fill the edges.
    if (x > 0) {
        pax_simple_rect(buf, placeholder, 0, y, x, buf->height - y);
    } else if (x < 0) {
        pax_simple_rect(buf, placeholder, buf->width, y, x, buf->height - y);
    }
    if (y > 0) {
        pax_simple_rect(buf, placeholder, 0, 0, buf->width, y);
    } else if (y < 0) {
        pax_simple_rect(buf, placeholder, 0, buf->height, buf->width, y);
    }

#if PAX_COMPILE_ORIENTATION
    // Restore previous orientation.
    buf->orientation = rot;
#endif
}
