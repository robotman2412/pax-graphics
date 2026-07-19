
// SPDX-License-Identifier: MIT

#include "pax_gfx.h"
#include "pax_internal.h"
#include "pax_matrix.h"
#include "pax_orientation.h"
#include "pax_renderer.h"
#include "pax_types.h"
#include "shapes/pax_rects.h"

#include <math.h>



// Set a pixel, merging with alpha.
void pax_merge_pixel(pax_buf_t *buf, pax_col_t color, int x, int y) {
    PAX_BUF_CHECK(buf);

#if CONFIG_PAX_COMPILE_ORIENTATION
    pax_vec2i tmp = pax_orient_det_vec2i(buf, (pax_vec2i){x, y});
    x             = tmp.x;
    y             = tmp.y;
#endif

    if (x < 0 || x >= buf->width || y < 0 || y >= buf->height) {
        // Out of bounds error.
        PAX_ERROR(PAX_ERR_BOUNDS);
    }

    int index = x + y * buf->width;
    if (buf->type_info.fmt_type == PAX_BUF_SUBTYPE_PALETTE) {
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

#if CONFIG_PAX_COMPILE_ORIENTATION
    pax_vec2i tmp = pax_orient_det_vec2i(buf, (pax_vec2i){x, y});
    x             = tmp.x;
    y             = tmp.y;
#endif

    if (x < 0 || x >= buf->width || y < 0 || y >= buf->height) {
        // Out of bounds error.
        PAX_ERROR(PAX_ERR_BOUNDS);
    }

    int index = x + y * buf->width;
    if (buf->type_info.fmt_type == PAX_BUF_SUBTYPE_PALETTE) {
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

#if CONFIG_PAX_COMPILE_ORIENTATION
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

#if CONFIG_PAX_COMPILE_ORIENTATION
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

#if CONFIG_PAX_COMPILE_ORIENTATION
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



// Draw a sprite; like a blit, but use color blending if applicable.
void pax_draw_sprite(pax_buf_t *base, pax_buf_t const *top, int x, int y) {
    PAX_BUF_CHECK(base);
    PAX_BUF_CHECK(top);
    pax_vec2i top_dims = pax_buf_get_dims(top);
    pax_draw_sprite_rot_sized(base, top, x, y, PAX_O_UPRIGHT, 0, 0, top_dims.x, top_dims.y);
}

// Draw a sprite; like a blit, but use color blending if applicable.
void pax_draw_sprite_rot(pax_buf_t *base, pax_buf_t const *top, int x, int y, pax_orientation_t rot) {
    PAX_BUF_CHECK(base);
    PAX_BUF_CHECK(top);
    pax_vec2i top_dims = pax_buf_get_dims(top);
    pax_draw_sprite_rot_sized(base, top, x, y, rot, 0, 0, top_dims.x, top_dims.y);
}

// Draw a sprite; like a blit, but use color blending if applicable.
void pax_draw_sprite_sized(
    pax_buf_t *base, pax_buf_t const *top, int x, int y, int top_x, int top_y, int top_w, int top_h
) {
    pax_draw_sprite_rot_sized(base, top, x, y, PAX_O_UPRIGHT, top_x, top_y, top_w, top_h);
}

// Draw a sprite; like a blit, but use color blending if applicable.
void pax_draw_sprite_rot_sized(
    pax_buf_t        *base,
    pax_buf_t const  *top,
    int               x,
    int               y,
    pax_orientation_t rot,
    int               top_x,
    int               top_y,
    int               top_w,
    int               top_h
) {
#if CONFIG_PAX_COMPILE_ORIENTATION
    if (rot & 4) {
        rot = ((rot - top->orientation) & 3) | 4;
    } else {
        rot = (rot + top->orientation) & 3;
    }
    if ((rot ^ top->orientation) & 1) {
        PAX_SWAP(int, top_x, top_y);
        PAX_SWAP(int, top_w, top_h);
    }
    pax_recti tmp = pax_orient_det_recti(base, (pax_recti){x, y, top_w, top_h});
    x             = tmp.x;
    y             = tmp.y;
    top_w         = tmp.w;
    top_h         = tmp.h;
    if (top_w < 0) {
        x     += top_w;
        top_w  = -top_w;
    }
    if (top_h < 0) {
        y     += top_h;
        top_h  = -top_h;
    }
    // Adjust rotation to account for base buffer's orientation transform
    // This ensures blit reads source in correct order after dimensions were transformed
    rot = (rot + base->orientation) & 3;
#endif
    pax_dispatch_sprite(base, top, (pax_recti){x, y, top_w, top_h}, rot, (pax_vec2i){top_x, top_y});
}



// Perform a buffer copying operation with a PAX buffer.
void pax_blit(pax_buf_t *base, pax_buf_t const *top, int x, int y) {
    PAX_BUF_CHECK(base);
    PAX_BUF_CHECK(top);
    pax_vec2i top_dims = pax_buf_get_dims(top);
    pax_blit_rot_sized(base, top, x, y, PAX_O_UPRIGHT, 0, 0, top_dims.x, top_dims.y);
}

// Perform a buffer copying operation with a PAX buffer.
void pax_blit_rot(pax_buf_t *base, pax_buf_t const *top, int x, int y, pax_orientation_t rot) {
    PAX_BUF_CHECK(base);
    PAX_BUF_CHECK(top);
    pax_vec2i top_dims = pax_buf_get_dims(top);
    pax_blit_rot_sized(base, top, x, y, rot, 0, 0, top_dims.x, top_dims.y);
}

// Perform a buffer copying operation with a PAX buffer.
void pax_blit_sized(pax_buf_t *base, pax_buf_t const *top, int x, int y, int top_x, int top_y, int top_w, int top_h) {
    pax_blit_rot_sized(base, top, x, y, PAX_O_UPRIGHT, top_x, top_y, top_w, top_h);
}

// Perform a buffer copying operation with a PAX buffer.
void pax_blit_rot_sized(
    pax_buf_t        *base,
    pax_buf_t const  *top,
    int               x,
    int               y,
    pax_orientation_t rot,
    int               top_x,
    int               top_y,
    int               top_w,
    int               top_h
) {
#if CONFIG_PAX_COMPILE_ORIENTATION
    if (rot & 4) {
        rot = ((rot - top->orientation) & 3) | 4;
    } else {
        rot = (rot + top->orientation) & 3;
    }
    if ((rot ^ top->orientation) & 1) {
        PAX_SWAP(int, top_x, top_y);
        PAX_SWAP(int, top_w, top_h);
    }
    pax_recti tmp = pax_orient_det_recti(base, (pax_recti){x, y, top_w, top_h});
    x             = tmp.x;
    y             = tmp.y;
    top_w         = tmp.w;
    top_h         = tmp.h;
    if (top_w < 0) {
        x     += top_w;
        top_w  = -top_w;
    }
    if (top_h < 0) {
        y     += top_h;
        top_h  = -top_h;
    }
    // Adjust rotation to account for base buffer's orientation transform
    // This ensures blit reads source in correct order after dimensions were transformed
    rot = (rot + base->orientation) & 3;
#endif
    pax_dispatch_blit(base, top, (pax_recti){x, y, top_w, top_h}, rot, (pax_vec2i){top_x, top_y});
}



// Perform a buffer copying operation with a PAX buffer.
void pax_blit_raw(pax_buf_t *base, void const *top, pax_vec2i top_dims, int x, int y) {
    pax_blit_raw_rot_sized(base, top, top_dims, x, y, PAX_O_UPRIGHT, 0, 0, top_dims.x, top_dims.y);
}

// Perform a buffer copying operation with a PAX buffer.
void pax_blit_raw_rot(pax_buf_t *base, void const *top, pax_vec2i top_dims, int x, int y, pax_orientation_t rot) {
    pax_blit_raw_rot_sized(base, top, top_dims, x, y, rot, 0, 0, top_dims.x, top_dims.y);
}

// Perform a buffer copying operation with a PAX buffer.
void pax_blit_raw_sized(
    pax_buf_t *base, void const *top, pax_vec2i top_dims, int x, int y, int top_x, int top_y, int top_w, int top_h
) {
    pax_blit_raw_rot_sized(base, top, top_dims, x, y, PAX_O_UPRIGHT, top_x, top_y, top_w, top_h);
}

// Perform a buffer copying operation with a PAX buffer.
void pax_blit_raw_rot_sized(
    pax_buf_t        *base,
    void const       *top,
    pax_vec2i         top_dims,
    int               x,
    int               y,
    pax_orientation_t rot,
    int               top_x,
    int               top_y,
    int               top_w,
    int               top_h
) {
#if CONFIG_PAX_COMPILE_ORIENTATION
    if (rot & 1) {
        PAX_SWAP(int, top_x, top_y);
        PAX_SWAP(int, top_w, top_h);
    }
    pax_recti tmp = pax_orient_det_recti(base, (pax_recti){x, y, top_w, top_h});
    x             = tmp.x;
    y             = tmp.y;
    top_w         = tmp.w;
    top_h         = tmp.h;
    if (top_w < 0) {
        x     += top_w;
        top_w  = -top_w;
    }
    if (top_h < 0) {
        y     += top_h;
        top_h  = -top_h;
    }
#endif
    pax_dispatch_blit_raw(base, top, top_dims, (pax_recti){x, y, top_w, top_h}, rot, (pax_vec2i){top_x, top_y});
}



// Detect relative orientation and scale from transformation matrix.
// Scale output given is before orientation is applied, and always positive.
static bool try_detect_orientation(matrix_2d_t const *mtx, pax_orientation_t *rot_out, pax_vec2f *scale_out) {
    for (size_t i = 0; i < 6; i++) {
        if (!isfinite(mtx->arr[i])) {
            return false;
        }
    }
    bool a0_zero = fabsf(mtx->a0) < 0.000001f;
    bool a1_zero = fabsf(mtx->a1) < 0.000001f;
    bool b0_zero = fabsf(mtx->b0) < 0.000001f;
    bool b1_zero = fabsf(mtx->b1) < 0.000001f;

    bool swap_xy, flip_h, flip_v;
    if (a0_zero && !a1_zero && !b0_zero && b1_zero) {
        // Swap x/y.
        swap_xy      = true;
        scale_out->x = fabsf(mtx->b0);
        scale_out->y = fabsf(mtx->a1);
        flip_h       = mtx->a1 < 0;
        flip_v       = mtx->b0 < 0;

    } else if (!a0_zero && a1_zero && b0_zero && !b1_zero) {
        // Normal x/y.
        swap_xy      = false;
        scale_out->x = fabsf(mtx->a0);
        scale_out->y = fabsf(mtx->b1);
        flip_h       = mtx->b1 < 0;
        flip_v       = mtx->a0 < 0;

    } else {
        // Is not a rotation, or not in 90-degree increments.
        return false;
    }

    switch (flip_h * 0b100 + flip_v * 0b010 + swap_xy * 0b001) {
        default: // Unreachable.
        case 0b000: *rot_out = PAX_O_UPRIGHT; break;
        case 0b001: *rot_out = PAX_O_ROT_CW_FLIP_H; break;
        case 0b010: *rot_out = PAX_O_ROT_HALF_FLIP_H; break;
        case 0b011: *rot_out = PAX_O_ROT_CCW; break;
        case 0b100: *rot_out = PAX_O_FLIP_H; break;
        case 0b101: *rot_out = PAX_O_ROT_CW; break;
        case 0b110: *rot_out = PAX_O_ROT_HALF; break;
        case 0b111: *rot_out = PAX_O_ROT_CCW_FLIP_H; break;
    }

    return true;
}

// Optimized draw path for images whose transform is an exact 90-degree orientation
// (rotation and/or flip), at any scale.
// Composes the orientation requested by the matrix with the top image's own storage
// orientation and the base buffer's physical storage orientation, then dispatches
// directly instead of going through the generic shader-based rasterizer.
static void draw_image_oriented(
    pax_buf_t *base, pax_buf_t const *top, pax_rectf screen_pos, pax_orientation_t orientation, bool assume_opaque
) {
    // Orientation requested by the matrix, composed with the top image's own storage
    // orientation (same composition rule as pax_draw_sprite_rot_sized/pax_blit_rot_sized).
    pax_orientation_t rot = orientation;
#if CONFIG_PAX_COMPILE_ORIENTATION
    if (rot & 4) {
        rot = ((rot - top->orientation) & 3) | 4;
    } else {
        rot = (rot + top->orientation) & 3;
    }
#endif

#if CONFIG_PAX_COMPILE_ORIENTATION
    // Account for the base buffer's own physical storage orientation.
    screen_pos = pax_rectf_abs(pax_orient_det_rectf(base, screen_pos));
    rot        = (rot + base->orientation) & 3;
#endif

    pax_recti base_pos = {
        (int)roundf(screen_pos.x),
        (int)roundf(screen_pos.y),
        (int)roundf(screen_pos.w),
        (int)roundf(screen_pos.h),
    };

    pax_dispatch_scaled_image(base, top, base_pos, rot, assume_opaque);
}

// Draw an axis-aligned image with scaling (if you want to draw 1:1, try blit).
void pax_scaled_image(pax_buf_t *base, pax_buf_t const *top, int x, int y, float width, float height) {
    draw_image_oriented(base, top, (pax_rectf){x, y, width, height}, PAX_O_UPRIGHT, false);
}

// Draw an axis-aligned image with scaling (if you want to draw 1:1, try blit).
// Relative orientation is applied after the size/position, i.e. a landscape image rotated will become portrait.
void pax_scaled_image_rot(
    pax_buf_t *base, pax_buf_t const *top, int x, int y, float width, float height, pax_orientation_t rot
) {
    draw_image_oriented(base, top, (pax_rectf){x, y, width, height}, rot, false);
}

// Draw an axis-aligned image with scaling (if you want to draw 1:1, try sprite).
void pax_scaled_image_op(pax_buf_t *base, pax_buf_t const *top, int x, int y, float width, float height) {
    draw_image_oriented(base, top, (pax_rectf){x, y, width, height}, PAX_O_UPRIGHT, true);
}

// Draw an axis-aligned image with scaling (if you want to draw 1:1, try sprite).
// Relative orientation is applied after the size/position, i.e. a landscape image rotated will become portrait.
void pax_scaled_image_rot_op(
    pax_buf_t *base, pax_buf_t const *top, int x, int y, float width, float height, pax_orientation_t rot
) {
    draw_image_oriented(base, top, (pax_rectf){x, y, width, height}, rot, true);
}

// Draw an image with a prespecified size.
static void draw_image_impl(
    pax_buf_t *base, pax_buf_t const *top, float x, float y, float width, float height, bool assume_opaque
) {
    PAX_BUF_CHECK(base);
    PAX_BUF_CHECK(top);

    bool has_alpha = false;
    if (!assume_opaque && top->type_info.fmt_type == PAX_BUF_SUBTYPE_PALETTE) {
        for (size_t i = 0; i < top->palette_size; i++) {
            if ((top->palette[i] & 0xff000000) != 0xff000000) {
                has_alpha = true;
                break;
            }
        }
    } else if (!assume_opaque) {
        has_alpha = top->type_info.a > 0;
    }

    // Fast path: transform is an exact 90-degree orientation (rotation and/or flip), any scale.
    pax_orientation_t rot;
    pax_vec2f         scale;
    if (try_detect_orientation(&base->stack_2d.value, &rot, &scale)) {
        pax_vec2i top_dim = pax_buf_get_dims(top);
        if (fabsf(width * scale.x - top_dim.x) < 0.000001f && fabsf(height * scale.y - top_dim.y) < 0.000001f) {
            x += base->stack_2d.value.a2;
            y += base->stack_2d.value.b2;
            if (has_alpha) {
                pax_draw_sprite_rot_sized(base, top, x, y, rot, 0, 0, top_dim.x, top_dim.y);
            } else {
                pax_blit_rot_sized(base, top, x, y, rot, 0, 0, top_dim.x, top_dim.y);
            }
        } else {
            // Destination rectangle: transform the local (pre-matrix) shape corners through the
            // current 2D matrix to get the on-screen axis-aligned rect; matrix_2d_transform maps
            // corners to corners for any 90-degree-increment orientation, so no separate
            // scale/flip bookkeeping is needed here.
            float x0 = x, y0 = y;
            float x1 = x + width, y1 = y + height;
            matrix_2d_transform(base->stack_2d.value, &x0, &y0);
            matrix_2d_transform(base->stack_2d.value, &x1, &y1);
            pax_rectf screen_pos = pax_rectf_abs((pax_rectf){x0, y0, x1 - x0, y1 - y0});

            draw_image_oriented(base, top, screen_pos, rot, !has_alpha);
        }
        return;
    }

    // Fallback: have to render as transformed rectangles (images that are not axis-aligned).
    if (has_alpha) {
        pax_shade_rect(base, -1, &PAX_SHADER_TEXTURE(top), NULL, x, y, width, height);
    } else {
        pax_shade_rect(base, -1, &PAX_SHADER_TEXTURE_OP(top), NULL, x, y, width, height);
    }
}

// Draws an image at the image's normal size.
void pax_draw_image(pax_buf_t *buf, pax_buf_t const *image, float x, float y) {
    draw_image_impl(buf, image, x, y, image ? image->width : 1, image ? image->height : 1, false);
}

// Draw an image with a prespecified size.
void pax_draw_image_sized(pax_buf_t *buf, pax_buf_t const *image, float x, float y, float width, float height) {
    draw_image_impl(buf, image, x, y, width, height, false);
}

// Draws an image at the image's normal size.
// Assumes the image is completely opaque, any transparent parts are drawn opaque.
void pax_draw_image_op(pax_buf_t *buf, pax_buf_t const *image, float x, float y) {
    draw_image_impl(buf, image, x, y, image ? image->width : 1, image ? image->height : 1, true);
}

// Draw an image with a prespecified size.
// Assumes the image is completely opaque, any transparent parts are drawn opaque.
void pax_draw_image_sized_op(pax_buf_t *buf, pax_buf_t const *image, float x, float y, float width, float height) {
    draw_image_impl(buf, image, x, y, width, height, true);
}


// Fill the background.
void pax_background(pax_buf_t *buf, pax_col_t color) {
    pax_dispatch_background(buf, color);
}

// Scroll the buffer, filling with a placeholder color.
void pax_buf_scroll(pax_buf_t *buf, pax_col_t placeholder, int x, int y) {
    PAX_BUF_CHECK(buf);
#if CONFIG_PAX_COMPILE_ASYNC_RENDERER
    pax_join();
#endif
    // TODO: Make into render callback.

#if CONFIG_PAX_COMPILE_ORIENTATION
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
    ptrdiff_t bit_off   = buf->type_info.bpp * off;
    // Number of bits to copy.
    size_t    bit_count = buf->type_info.bpp * count;

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
            for (size_t i = count - 1; i != SIZE_MAX; i--) {
                pax_col_t value = buf->getter(buf, off + i);
                buf->setter(buf, value, i);
            }
        } else {
            for (size_t i = 0; i < count; i++) {
                pax_col_t value = buf->getter(buf, i);
                buf->setter(buf, value, off + i);
            }
        }
    }

#if CONFIG_PAX_COMPILE_ORIENTATION
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

#if CONFIG_PAX_COMPILE_ORIENTATION
    // Restore previous orientation.
    buf->orientation = rot;
#endif
}
