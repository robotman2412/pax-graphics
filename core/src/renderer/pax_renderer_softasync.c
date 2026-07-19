
// SPDX-License-Identifier: MIT

#include "renderer/pax_renderer_softasync.h"

#include "helpers/pax_drawing_helpers.h"
#include "pax_internal.h"
#include "pax_types.h"
#include "ptq.h"
#include "renderer/pax_renderer_soft.h"
#include "semaphore.h"

#include <sched.h>
#include <stdbool.h>

#if CONFIG_PAX_USE_FREERTOS
    #include <freertos/FreeRTOS.h>
#else
    #include <pthread.h>
#endif
#include <stdatomic.h>

#if !CONFIG_PAX_COMPILE_ASYNC_RENDERER

// Enable the asynchronous renderer.
// If `multithreaded` is `true` and `CONFIG_PAX_COMPILE_ASYNC_RENDERER` is set to `2`,
// This will use two threads for rendering instead of just one.
void pax_set_renderer_async(bool multithreaded) {
    PAX_LOGE(
        "pax-sasr",
        "Async renderer is not compiled in, please define CONFIG_PAX_COMPILE_ASYNC_RENDERER to be 1 or 2."
    );
}

#else

// Enable the asynchronous renderer.
// If `multithreaded` is `true` and `CONFIG_PAX_COMPILE_ASYNC_RENDERER` is set to `2`,
// This will use two threads for rendering instead of just one.
void pax_set_renderer_async(bool multithreaded) {
    pax_set_renderer(&pax_render_engine_softasync, (void *)multithreaded);
}

// Args for software async renderer.
typedef struct {
    ptq_queue_t               queue;
    pax_render_funcs_t const *renderfuncs;
    pthread_mutex_t           rendermtx;
} pax_sasr_worker_args_t;



static bool is_multithreaded;

static pax_sasr_worker_args_t args0;

    #if CONFIG_PAX_COMPILE_ASYNC_RENDERER == 2
static pax_sasr_worker_args_t args1;
    #endif

// Queue a draw call.
static void pax_sasr_queue(pax_task_t *task);

    #if CONFIG_PAX_USE_FREERTOS
// Worker thread function for software async renderer.
static void pax_sasr_worker(void *_args);
    #else
// Worker thread function for software async renderer.
static void *pax_sasr_worker(void *_args);
    #endif

// Number of completions that still need to happen.
static atomic_int outstanding;
// Semaphore posted every time a piece of work finishes.
static sem_t      complete_sem;

// Initialize the async renderer.
static pax_render_funcs_t const *pax_sasr_init(void *arg) {
    sem_init(&complete_sem, 0, 0);
    outstanding = 0;

    args0.queue       = ptq_create_max(sizeof(pax_task_t), CONFIG_PAX_QUEUE_SIZE);
    args0.renderfuncs = &pax_render_funcs_soft;

    #if CONFIG_PAX_COMPILE_ASYNC_RENDERER == 2
    is_multithreaded = arg;
    if (is_multithreaded) {
        args0.renderfuncs = &pax_render_funcs_mcr_thread0;
        args1.renderfuncs = &pax_render_funcs_mcr_thread1;
        args1.queue       = ptq_create_max(sizeof(pax_task_t), CONFIG_PAX_QUEUE_SIZE);
        pthread_mutex_init(&args1.rendermtx, NULL);
        #if CONFIG_PAX_USE_FREERTOS
        TaskHandle_t dummy_handle;
        xTaskCreatePinnedToCore(pax_sasr_worker, "MCRW1", 4096, &args1, 1, &dummy_handle, 1);
        #else
        pthread_t handle;
        pthread_create(&handle, NULL, pax_sasr_worker, &args1);
        pthread_detach(handle);
        #endif
    }
    #else
    if (is_multithreaded) {
        PAX_LOGW(
            "pax-sasr",
            "Async renderer is compiled in single-threaded mode; please define CONFIG_PAX_COMPILE_ASYNC_RENDERER to be "
            "2 to use multithreaded mode."
        );
    }
    #endif

    pthread_mutex_init(&args0.rendermtx, NULL);
    #if CONFIG_PAX_USE_FREERTOS
    TaskHandle_t dummy_handle;
    xTaskCreatePinnedToCore(pax_sasr_worker, "MCRW0", 4096, &args0, 1, &dummy_handle, 0);
    #else
    pthread_t handle;
    pthread_create(&handle, NULL, pax_sasr_worker, &args0);
    pthread_detach(handle);
    #endif

    return &pax_render_funcs_softasync;
}

// Deinitialize the async renderer.
static void pax_sasr_deinit() {
    pax_task_t task = {
        .type = PAX_TASK_STOP,
    };
    pax_sasr_queue(&task);
    pax_sasr_join();
    #if CONFIG_PAX_COMPILE_ASYNC_RENDERER == 2
    if (is_multithreaded) {
        ptq_destroy(args1.queue);
        pthread_mutex_destroy(&args1.rendermtx);
    }
    #endif
    ptq_destroy(args0.queue);
    pthread_mutex_destroy(&args0.rendermtx);
    sem_destroy(&complete_sem);
}


// Queue a draw call.
static void pax_sasr_queue(pax_task_t *task) {
    atomic_fetch_add_explicit(&outstanding, 1 + is_multithreaded, memory_order_relaxed);
    ptq_send_block(args0.queue, task, NULL);
    #if CONFIG_PAX_COMPILE_ASYNC_RENDERER == 2
    if (is_multithreaded) {
        ptq_send_block(args1.queue, task, NULL);
    }
    #endif
}

    #if CONFIG_PAX_USE_FREERTOS
// Worker thread function for software async renderer.
static void pax_sasr_worker(void *_args)
    #else
// Worker thread function for software async renderer.
static void *pax_sasr_worker(void *_args)
    #endif
{
    pax_sasr_worker_args_t *args = _args;

    while (1) {
        pax_task_t task;
        ptq_receive_block(args->queue, &task, &args->rendermtx);
        atomic_thread_fence(memory_order_acquire);

        if (task.type == PAX_TASK_STOP) {
            sem_post(&complete_sem);
            pthread_mutex_unlock(&args->rendermtx);
    #if CONFIG_PAX_USE_FREERTOS
            vTaskDelete(NULL);
    #else
            return NULL;
    #endif
        } else if (task.type == PAX_TASK_BACKGROUND) {
            args->renderfuncs->background(task.buffer, task.color);
        } else if (task.type == PAX_TASK_QUAD) {
            if (task.use_shader) {
                args->renderfuncs->shaded_quad(task.buffer, task.color, task.quadf.shape, &task.shader, task.quadf.uvs);
            } else {
                args->renderfuncs->unshaded_quad(task.buffer, task.color, task.quadf.shape);
            }
        } else if (task.type == PAX_TASK_RECT) {
            if (task.use_shader) {
                args->renderfuncs->shaded_rect(task.buffer, task.color, task.rectf.shape, &task.shader, task.rectf.uvs);
            } else {
                args->renderfuncs->unshaded_rect(task.buffer, task.color, task.rectf.shape);
            }
        } else if (task.type == PAX_TASK_TRI) {
            if (task.use_shader) {
                args->renderfuncs->shaded_tri(task.buffer, task.color, task.trif.shape, &task.shader, task.trif.uvs);
            } else {
                args->renderfuncs->unshaded_tri(task.buffer, task.color, task.trif.shape);
            }
        } else if (task.type == PAX_TASK_LINE) {
            if (task.use_shader) {
                args->renderfuncs->shaded_line(task.buffer, task.color, task.linef.shape, &task.shader, task.linef.uvs);
            } else {
                args->renderfuncs->unshaded_line(task.buffer, task.color, task.linef.shape);
            }
        } else if (task.type == PAX_TASK_SPRITE) {
            args->renderfuncs
                ->sprite(task.buffer, task.blit.top, task.blit.base_pos, task.blit.top_orientation, task.blit.top_pos);
        } else if (task.type == PAX_TASK_BLIT) {
            args->renderfuncs
                ->blit(task.buffer, task.blit.top, task.blit.base_pos, task.blit.top_orientation, task.blit.top_pos);
        } else if (task.type == PAX_TASK_BLIT_RAW) {
            args->renderfuncs->blit_raw(
                task.buffer,
                task.blit.top,
                task.blit.top_dims,
                task.blit.base_pos,
                task.blit.top_orientation,
                task.blit.top_pos
            );
        } else if (task.type == PAX_TASK_BLIT_CHAR) {
            args->renderfuncs
                ->blit_char(task.buffer, task.color, task.blit_char.pos, task.blit_char.scale, task.blit_char.rsdata);
        } else if (task.type == PAX_TASK_TEXT) {
            args->renderfuncs->text(
                task.buffer,
                task.text_matrix,
                task.color,
                task.text.font,
                task.text.font_size,
                task.text.pos,
                task.text.str.len > PAX_SSO_BUF_LEN ? task.text.str.ptr->data : task.text.str.sso,
                task.text.str.len,
                task.text.halign,
                task.text.valign,
                task.text.cursorpos
            );
            if (task.text.str.len > PAX_SSO_BUF_LEN) {
                if (atomic_fetch_sub(&task.text.str.ptr->refcount, memory_order_relaxed) == 1) {
                    free(task.text.str.ptr);
                }
            }
        } else if (task.type == PAX_TASK_SCALED_IMAGE) {
            args->renderfuncs->scaled_image(
                task.buffer,
                task.scaled_image.top,
                task.scaled_image.base_pos,
                task.scaled_image.top_orientation,
                task.scaled_image.assume_opaque
            );
        }

        sem_post(&complete_sem);
        pthread_mutex_unlock(&args->rendermtx);
    }
}


    #if CONFIG_PAX_COMPILE_ASYNC_RENDERER == 2

// Background fill.
static void pax_sasr_background_impl(bool odd_scanline, pax_buf_t *buf, pax_col_t color) {
    uint32_t value;
    if (buf->type_info.fmt_type == PAX_BUF_SUBTYPE_PALETTE) {
        if (color > buf->palette_size)
            value = 0;
        else
            value = color;
    } else {
        value = buf->col2buf(buf, color);
    }

    pax_range_setter_t setter = buf->range_setter;
    int                width  = buf->width;
    int                height = buf->height;
    for (int row = odd_scanline; row < height; row += 2) {
        setter(buf, value, row * width, width);
    }
}

// Read a single pixel from a raw buffer type by index.
__attribute__((always_inline)) static inline pax_col_t raw_get_pixel(void const *buf, uint8_t bpp, int index) {
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
        case 24: return buf_8bpp[index * 3] | (buf_8bpp[index * 3 + 1] << 8) | (buf_8bpp[index * 3 + 2] << 16);
        #else
        case 24: return buf_8bpp[index * 3 + 2] | (buf_8bpp[index * 3 + 1] << 8) | (buf_8bpp[index * 3] << 16);
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
        // Clipping: dimensions of top buffer.
        #if CONFIG_PAX_COMPILE_ORIENTATION
    if (top_orientation & 1) {
        // Flip X and Y before size check.
        if (base_pos.w > top_dims.y) {
            base_pos.w = top_dims.y;
        }
        if (base_pos.h > top_dims.x) {
            base_pos.h = top_dims.x;
        }
    } else
        #endif
    {
        if (base_pos.w > top_dims.x) {
            base_pos.w = top_dims.x;
        }
        if (base_pos.h > top_dims.y) {
            base_pos.h = top_dims.y;
        }
    }

    // Clipping: clip rect of bottom buffer.
    if (base_pos.x < base->clip.x) {
        base_pos.w -= base->clip.x - base_pos.x;
        base_pos.x  = base->clip.x;
    }
    if (base_pos.x + base_pos.w > base->clip.x + base->clip.w) {
        base_pos.w = base->clip.x + base->clip.w - base_pos.x;
    }
    if (base_pos.y < base->clip.y) {
        base_pos.h -= base->clip.y - base_pos.y;
        base_pos.y  = base->clip.y;
    }
    if (base_pos.y + base_pos.h > base->clip.y + base->clip.h) {
        base_pos.h = base->clip.y + base->clip.h - base_pos.y;
    }

    if (base_pos.h <= 0 || base_pos.w <= 0) {
        return;
    }

        // Determine copying parameters for top buffer.
        #if CONFIG_PAX_COMPILE_ORIENTATION
    // clang-format off
    int dx = 0, dy = 0;
    bool swap = false;
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
                pax_col_t        base_col = base->buf2col(base, base->getter(base, base_index));
                pax_col_t        top_col  = _top->buf2col(_top, _top->getter(_top, top_index));
                base->setter(base, base->col2buf(base, pax_col_merge(base_col, top_col)), base_index);
            } else if (is_raw_buf) {
                pax_col_t col = raw_get_pixel(top, base->type_info.bpp, top_index);
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
    } else if (
        base->type_info.fmt_type == PAX_BUF_SUBTYPE_PALETTE && top->type_info.fmt_type != PAX_BUF_SUBTYPE_PALETTE
    ) {
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

// Clipping routine for text characters.
// Returns `true` if the character should be drawn at all.
__attribute__((always_inline)) static inline bool
    blit_char_clip(pax_recti clip, pax_vec2i pos, int scale, pax_recti *dims_out, pax_text_rsdata_t rsdata) {
    pax_recti dims = {0, 0, rsdata.w * scale, rsdata.h * scale};

    // Offset to calculate clipping.
    dims.x += pos.x;
    dims.y += pos.y;

    // Actual clipping calculation.
    if (dims.x < clip.x) {
        dims.w -= clip.x - dims.x;
        dims.x  = clip.x;
    }
    if (dims.y < clip.y) {
        dims.h -= clip.y - dims.y;
        dims.y  = clip.y;
    }
    if (dims.x + dims.w > clip.x + clip.w) {
        dims.w = clip.x + clip.w - dims.x;
    }
    if (dims.y + dims.h > clip.y + clip.h) {
        dims.h = clip.y + clip.h - dims.y;
    }

    // Undo the offset.
    dims.x -= pos.x;
    dims.y -= pos.y;

    *dims_out = dims;
    return dims.w > 0 && dims.h > 0;
}

// Blit one or more characters of text in the bitmapped format.
__attribute__((always_inline)) static inline void pax_sasr_blit_char_impl_2(
    bool              odd_scanline,
    pax_buf_t        *buf,
    pax_col_t         color,
    pax_vec2i         pos,
    int               scale,
    pax_text_rsdata_t rsdata,
    bool              direct_set
) {
    // clang-format off
    int dx = 0, dy = 0;
    pax_recti effective_clip;
#if CONFIG_PAX_COMPILE_ORIENTATION
    effective_clip = pax_get_clip(buf);
    switch (buf->orientation) {
        case PAX_O_UPRIGHT:         dx =  1;          dy =  buf->width; break;
        case PAX_O_ROT_CCW:         dx = -buf->width; dy =  1;          break;
        case PAX_O_ROT_HALF:        dx = -1;          dy = -buf->width; break;
        case PAX_O_ROT_CW:          dx =  buf->width; dy = -1;          break;
        case PAX_O_FLIP_H:          dx = -1;          dy =  buf->width; break;
        case PAX_O_ROT_CCW_FLIP_H:  dx = -buf->width; dy = -1;          break;
        case PAX_O_ROT_HALF_FLIP_H: dx =  1;          dy = -buf->width; break;
        case PAX_O_ROT_CW_FLIP_H:   dx =  buf->width; dy =  1;          break;
    }
#else
    dx = 1;
    dy = buf->width;
    effective_clip = buf->clip;
#endif
    // clang-format on

    // Calculate correct multiplier for alpha.
    uint8_t  bitmask   = (1 << rsdata.bpp) - 1;
    uint16_t alpha_mul = (0xff00 / bitmask);
    if (!direct_set) {
        // Premultiply the color's alpha.
        alpha_mul  = alpha_mul * ((color >> 24) + (color >> 31)) / 256;
        color     &= 0x00ffffff;
    } else {
        // Pre-convert the color.
        color = buf->col2buf(buf, color);
    }

    // Clip char.
    pax_recti dims;
    if (blit_char_clip(effective_clip, pos, scale, &dims, rsdata)) {
        // Char is not (entirely) outside framebuffer.
        int x_increment = 1, y_increment = 2;
        #if CONFIG_PAX_COMPILE_ORIENTATION
        pos = pax_orient_det_vec2i(buf, pos);

        if (buf->orientation & 1) {
            x_increment = 2, y_increment = 1;

            if (((pos.y + dims.x) & 1) != odd_scanline) {
                dims.x++;
                dims.w--;
            }
        } else
        #endif
            if (((pos.y + dims.y) & 1) != odd_scanline) {
            dims.y++;
            dims.h--;
        }


        // Calculate drawing parameters.
        int bits_dy = rsdata.row_stride << 3;
        int offset  = (pos.x + pos.y * buf->width) + (dims.x * dx + dims.y * dy);

        // Actual blit loop.
        for (int y = dims.y; y < dims.y + dims.h; y += y_increment) {
            int next_y_offset = offset + y_increment * dy;
            for (int x = dims.x; x < dims.x + dims.w; x += x_increment) {
                // Extract value from character bitmap.
                int     bit   = x / scale * rsdata.bpp + y / scale * bits_dy;
                uint8_t value = (rsdata.bitmap[bit >> 3] >> (bit & 7)) & bitmask;
                // Multiply value into 0-255 range.
                value         = (value * alpha_mul) >> 8;

                if (direct_set) {
                    // Directly set the pixel.
                    if (value >= 128) {
                        buf->setter(buf, color, offset);
                    }
                } else {
                    // Perform correct alpha-blending.
                    pax_col_t top    = color | (value << 24);
                    pax_col_t base   = buf->buf2col(buf, buf->getter(buf, offset));
                    pax_col_t merged = pax_col_merge(base, top);
                    buf->setter(buf, buf->col2buf(buf, merged), offset);
                }

                offset += x_increment * dx;
            }
            offset = next_y_offset;
        }
    }
}

// Blit one or more characters of text in the bitmapped format.
__attribute__((noinline)) static void pax_sasr_blit_char_direct_set(
    bool odd_scanline, pax_buf_t *buf, pax_col_t color, pax_vec2i pos, int scale, pax_text_rsdata_t rsdata
) {
    pax_sasr_blit_char_impl_2(odd_scanline, buf, color, pos, scale, rsdata, true);
}

// Blit one or more characters of text in the bitmapped format.
__attribute__((noinline)) static void pax_sasr_blit_char_alpha_blend(
    bool odd_scanline, pax_buf_t *buf, pax_col_t color, pax_vec2i pos, int scale, pax_text_rsdata_t rsdata
) {
    pax_sasr_blit_char_impl_2(odd_scanline, buf, color, pos, scale, rsdata, false);
}

// Blit one or more characters of text in the bitmapped format.
static void pax_sasr_blit_char_impl(
    bool odd_scanline, pax_buf_t *buf, pax_col_t color, pax_vec2i pos, int scale, pax_text_rsdata_t rsdata
) {
    if ((rsdata.bpp == 1 && color >> 24 == 255) || buf->type_info.fmt_type == PAX_BUF_SUBTYPE_PALETTE) {
        // If the BPP is 1 and the color is fully opaque OR the buffer is of palette type, no alpha blending happens.
        pax_sasr_blit_char_direct_set(odd_scanline, buf, color, pos, scale, rsdata);
    } else {
        // Otherwise, alpha blending is necessary.
        pax_sasr_blit_char_alpha_blend(odd_scanline, buf, color, pos, scale, rsdata);
    }
}

// Image reading helper for `pax_swr_scaled_image`.
static inline __attribute__((always_inline)) pax_col_t
    scaled_image_get_pixel(pax_buf_t const *buf, pax_vec2f pos, pax_index_getter_t get, pax_col_conv_t conv) {
    int tex_x = floorf(pos.x);
    int tex_y = floorf(pos.y);

    int       idx0 = pax_clamped_index(buf, tex_x, tex_y);
    int       idx1 = pax_clamped_index(buf, tex_x + 1, tex_y);
    int       idx2 = pax_clamped_index(buf, tex_x + 1, tex_y + 1);
    int       idx3 = pax_clamped_index(buf, tex_x, tex_y + 1);
    uint32_t  raw0 = get(buf, idx0);
    uint32_t  raw1 = get(buf, idx1);
    uint32_t  raw2 = get(buf, idx2);
    uint32_t  raw3 = get(buf, idx3);
    pax_col_t col0 = conv(buf, raw0);
    pax_col_t col1 = conv(buf, raw1);
    pax_col_t col2 = conv(buf, raw2);
    pax_col_t col3 = conv(buf, raw3);

    uint32_t coeffx = (pos.x - tex_x) * 255;
    uint32_t coeffy = (pos.y - tex_y) * 255;

    pax_col_t col01_a = pax_lerp_mask(0xff00ff00, coeffx, col0, col1);
    pax_col_t col32_a = pax_lerp_mask(0xff00ff00, coeffx, col3, col2);
    pax_col_t color_a = pax_lerp_mask(0xff00ff00, coeffy, col01_a, col32_a);
    pax_col_t col01_b = pax_lerp_mask(0x00ff00ff, coeffx, col0, col1);
    pax_col_t col32_b = pax_lerp_mask(0x00ff00ff, coeffx, col3, col2);
    pax_col_t color_b = pax_lerp_mask(0x00ff00ff, coeffy, col01_b, col32_b);

    return color_a | color_b;
}

// Draw an axis-aligned image with fractional scaling.
void pax_sasr_scaled_image_impl(
    bool              odd_scanline,
    pax_buf_t        *base,
    pax_buf_t const  *top,
    pax_recti         base_pos,
    pax_orientation_t top_orientation,
    bool              assume_opaque
) {
    pax_index_getter_t tget     = top->getter;
    pax_col_conv_t     tbuf2col = top->buf2col;
    pax_index_getter_t bget     = base->getter;
    pax_index_setter_t bset     = base->setter;
    pax_col_conv_t     bcol2buf = base->col2buf;
    pax_col_conv_t     bbuf2col = base->buf2col;

    pax_vec2f tex_start, tex_end;
    bool      swap_xy;

    switch (top_orientation & 3) {
        case PAX_O_UPRIGHT:
            tex_start = (pax_vec2f){0, 0};
            tex_end   = (pax_vec2f){1, 1};
            swap_xy   = false;
            break;
        case PAX_O_ROT_CCW:
            tex_start = (pax_vec2f){1, 0};
            tex_end   = (pax_vec2f){0, 1};
            swap_xy   = true;
            break;
        case PAX_O_ROT_HALF:
            tex_start = (pax_vec2f){1, 1};
            tex_end   = (pax_vec2f){0, 0};
            swap_xy   = false;
            break;
        case PAX_O_ROT_CW:
            tex_start = (pax_vec2f){0, 1};
            tex_end   = (pax_vec2f){1, 0};
            swap_xy   = true;
            break;
    }
    if (top_orientation & 4) {
        tex_start = (pax_vec2f){1 - tex_start.x, tex_start.y};
        tex_end   = (pax_vec2f){1 - tex_end.x, tex_end.y};
    }
    tex_start.x *= top->width;
    tex_start.y *= top->height;
    tex_end.x   *= top->width;
    tex_end.y   *= top->height;

    {
        float dx = (tex_end.x - tex_start.x) / base_pos.w;
        float dy = (tex_end.y - tex_start.y) / base_pos.h;

        if (base_pos.x < base->clip.x) {
            int diff     = base->clip.x - base_pos.x;
            tex_start.x += diff * dx;
            base_pos.w  += diff;
            base_pos.x  += diff;
        }
        if (base_pos.x + base_pos.w > base->clip.x + base->clip.w) {
            int diff    = (base_pos.x + base_pos.w) - (base->clip.x + base->clip.w);
            tex_end.x  -= diff * dx;
            base_pos.w -= diff;
        }
        if (base_pos.y < base->clip.y) {
            int diff     = base->clip.y - base_pos.y;
            tex_start.y += diff * dy;
            base_pos.w  += diff;
            base_pos.y  += diff;
        }
        if (base_pos.y + base_pos.h > base->clip.y + base->clip.h) {
            int diff    = (base_pos.y + base_pos.h) - (base->clip.y + base->clip.h);
            tex_end.y  -= diff * dy;
            base_pos.h -= diff;
        }
        if (base_pos.w < 0 || base_pos.h < 0) {
            return;
        }
    }

    pax_vec2f tex_dx = {0};
    pax_vec2f tex_dy = {0};
    if (!swap_xy) {
        tex_dx.x = (tex_end.x - tex_start.x) / base_pos.w;
        tex_dy.y = (tex_end.y - tex_start.y) / base_pos.h;
    } else {
        tex_dy.x = (tex_end.x - tex_start.x) / base_pos.h;
        tex_dx.y = (tex_end.y - tex_start.y) / base_pos.w;
    }

    int       bindex  = base_pos.x + base->width * base_pos.y;
    pax_vec2f tex_pos = tex_start;
    int       y       = base_pos.y;
    if (odd_scanline != (y & 1)) {
        bindex    += base->width;
        tex_pos.x += tex_dy.x;
        tex_pos.y += tex_dy.y;
    }
    for (; y < base_pos.y + base_pos.h; y += 2) {
        for (int x = base_pos.x; x < base_pos.x + base_pos.w; x++) {
            pax_col_t col = scaled_image_get_pixel(top, tex_pos, tget, tbuf2col);
            if (!assume_opaque) {
                col = pax_col_merge_inlined(bbuf2col(base, bget(base, bindex)), col);
            }
            bset(base, bcol2buf(base, col), bindex);
            bindex++;

            tex_pos.x += tex_dx.x;
            tex_pos.y += tex_dx.y;
        }
        bindex    += 2 * base->width - base_pos.w;
        tex_pos.x += 2 * tex_dy.x - base_pos.w * tex_dx.x;
        tex_pos.y += 2 * tex_dy.y - base_pos.w * tex_dx.y;
    }
}



// Background fill.
void pax_mcrw0_background(pax_buf_t *buf, pax_col_t color) {
    pax_sasr_background_impl(0, buf, color);
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


// Draw an axis-aligned image with fractional scaling.
void pax_mcrw0_scaled_image(
    pax_buf_t *base, pax_buf_t const *top, pax_recti base_pos, pax_orientation_t top_orientation, bool assume_opaque
) {
    pax_sasr_scaled_image_impl(0, base, top, base_pos, top_orientation, assume_opaque);
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

// Blit one or more characters of text in the bitmapped format.
void pax_mcrw0_blit_char(pax_buf_t *buf, pax_col_t color, pax_vec2i pos, int scale, pax_text_rsdata_t rsdata) {
    pax_sasr_blit_char_impl(0, buf, color, pos, scale, rsdata);
}

// Draw a string of text in the bitmapped format.
void pax_mcrw0_text(
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
) {
    pax_text_render_t ctx = {
        .do_render   = true,
        .renderfuncs = &pax_render_funcs_mcr_thread0,
        .buf         = buf,
        .color       = color,
        .font        = font,
        .font_size   = font_size,
        .matrix      = matrix,
    };
    pax_internal_text_generic(&ctx, pos, text, text_len, cursorpos, halign, valign);
}


// Background fill.
void pax_mcrw1_background(pax_buf_t *buf, pax_col_t color) {
    pax_sasr_background_impl(1, buf, color);
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

// Draw an axis-aligned image with fractional scaling.
void pax_mcrw1_scaled_image(
    pax_buf_t *base, pax_buf_t const *top, pax_recti base_pos, pax_orientation_t top_orientation, bool assume_opaque
) {
    pax_sasr_scaled_image_impl(1, base, top, base_pos, top_orientation, assume_opaque);
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

// Blit one or more characters of text in the bitmapped format.
void pax_mcrw1_blit_char(pax_buf_t *buf, pax_col_t color, pax_vec2i pos, int scale, pax_text_rsdata_t rsdata) {
    pax_sasr_blit_char_impl(1, buf, color, pos, scale, rsdata);
}

// Draw a string of text in the bitmapped format.
void pax_mcrw1_text(
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
) {
    pax_text_render_t ctx = {
        .do_render   = true,
        .renderfuncs = &pax_render_funcs_mcr_thread1,
        .buf         = buf,
        .color       = color,
        .font        = font,
        .font_size   = font_size,
        .matrix      = matrix,
    };
    pax_internal_text_generic(&ctx, pos, text, text_len, cursorpos, halign, valign);
}

    #endif


// Background fill.
void pax_sasr_background(pax_buf_t *buf, pax_col_t color) {
    pax_task_t task = {
        .buffer = buf,
        .type   = PAX_TASK_BACKGROUND,
        .color  = color,
    };
    pax_sasr_queue(&task);
}

// Draw a solid-colored line.
void pax_sasr_unshaded_line(pax_buf_t *buf, pax_col_t color, pax_linef shape) {
    pax_task_t task = {
        .buffer      = buf,
        .type        = PAX_TASK_LINE,
        .color       = color,
        .use_shader  = false,
        .linef.shape = shape,
    };
    pax_sasr_queue(&task);
}

// Draw a solid-colored rectangle.
void pax_sasr_unshaded_rect(pax_buf_t *buf, pax_col_t color, pax_rectf shape) {
    pax_task_t task = {
        .buffer      = buf,
        .type        = PAX_TASK_RECT,
        .color       = color,
        .use_shader  = false,
        .rectf.shape = shape,
    };
    pax_sasr_queue(&task);
}

// Draw a solid-colored quad.
void pax_sasr_unshaded_quad(pax_buf_t *buf, pax_col_t color, pax_quadf shape) {
    pax_task_t task = {
        .buffer      = buf,
        .type        = PAX_TASK_QUAD,
        .color       = color,
        .use_shader  = false,
        .quadf.shape = shape,
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
        .trif.shape = shape,
    };
    pax_sasr_queue(&task);
}


// Draw a line with a shader.
void pax_sasr_shaded_line(pax_buf_t *buf, pax_col_t color, pax_linef shape, pax_shader_t const *shader, pax_linef uv) {
    pax_task_t task = {
        .buffer      = buf,
        .type        = PAX_TASK_LINE,
        .color       = color,
        .shader      = *shader,
        .use_shader  = true,
        .linef.shape = shape,
        .linef.uvs   = uv,
    };
    pax_sasr_queue(&task);
}

// Draw a rectangle with a shader.
void pax_sasr_shaded_rect(pax_buf_t *buf, pax_col_t color, pax_rectf shape, pax_shader_t const *shader, pax_quadf uv) {
    pax_task_t task = {
        .buffer      = buf,
        .type        = PAX_TASK_RECT,
        .color       = color,
        .shader      = *shader,
        .use_shader  = true,
        .rectf.shape = shape,
        .rectf.uvs   = uv,
    };
    pax_sasr_queue(&task);
}

// Draw a quad with a shader.
void pax_sasr_shaded_quad(pax_buf_t *buf, pax_col_t color, pax_quadf shape, pax_shader_t const *shader, pax_quadf uv) {
    pax_task_t task = {
        .buffer      = buf,
        .type        = PAX_TASK_QUAD,
        .color       = color,
        .shader      = *shader,
        .use_shader  = true,
        .quadf.shape = shape,
        .quadf.uvs   = uv,
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
        .trif.shape = shape,
        .trif.uvs   = uv,
    };
    pax_sasr_queue(&task);
}



// Draw an axis-aligned image with fractional scaling.
void pax_sasr_scaled_image(
    pax_buf_t *base, pax_buf_t const *top, pax_recti base_pos, pax_orientation_t top_orientation, bool assume_opaque
) {
    pax_task_t task = {
        .buffer       = base,
        .type         = PAX_TASK_SCALED_IMAGE,
        .scaled_image = {
            .top             = top,
            .base_pos        = base_pos,
            .top_orientation = top_orientation,
            .assume_opaque   = assume_opaque,
        },
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
        .blit.base_pos = base_pos,
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
        .blit.base_pos = base_pos,
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
        .blit.base_pos = base_pos,
    };
    pax_sasr_queue(&task);
}

// Blit one or more characters of text in the bitmapped format.
void pax_sasr_blit_char(pax_buf_t *buf, pax_col_t color, pax_vec2i pos, int scale, pax_text_rsdata_t rsdata) {
    pax_task_t task = {
        .buffer    = buf,
        .type      = PAX_TASK_BLIT_CHAR,
        .color     = color,
        .blit_char = {
            .rsdata = rsdata,
            .pos    = pos,
            .scale  = scale,
        },
    };
    pax_sasr_queue(&task);
}

// Draw a string of text in the bitmapped format.
void pax_sasr_text(
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
) {
    pax_task_str_t str;
    str.len = text_len;
    if (text_len <= PAX_SSO_BUF_LEN) {
        memcpy(str.sso, text, text_len);
    } else {
        str.ptr = malloc(sizeof(pax_rcstr_t) + text_len);
        memcpy(str.ptr->data, text, text_len);
        atomic_store(&str.ptr->refcount, is_multithreaded + 1);
    }
    pax_task_t task = {
        .buffer         = buf,
        .type           = PAX_TASK_TEXT,
        .color          = color,
        .text_matrix    = matrix,
        .text.font      = font,
        .text.font_size = font_size,
        .text.pos       = pos,
        .text.halign    = halign,
        .text.valign    = valign,
        .text.cursorpos = cursorpos,
        .text.str       = str,
    };
    pax_sasr_queue(&task);
}



// Wait for all pending draw calls to finish.
void pax_sasr_join() {
    int count = atomic_exchange_explicit(&outstanding, 0, memory_order_relaxed);
    while (count--) {
        sem_wait(&complete_sem);
    }
}



// Async software rendering functions.
pax_render_funcs_t const pax_render_funcs_softasync = {
    .background    = pax_sasr_background,
    .unshaded_line = pax_sasr_unshaded_line,
    .unshaded_rect = pax_sasr_unshaded_rect,
    .unshaded_quad = pax_sasr_unshaded_quad,
    .unshaded_tri  = pax_sasr_unshaded_tri,
    .shaded_line   = pax_sasr_shaded_line,
    .shaded_rect   = pax_sasr_shaded_rect,
    .shaded_quad   = pax_sasr_shaded_quad,
    .shaded_tri    = pax_sasr_shaded_tri,
    .scaled_image  = pax_sasr_scaled_image,
    .sprite        = pax_sasr_sprite,
    .blit          = pax_sasr_blit,
    .blit_raw      = pax_sasr_blit_raw,
    .blit_char     = pax_sasr_blit_char,
    .join          = pax_sasr_join,
    .text          = pax_sasr_text,
};

    #if CONFIG_PAX_COMPILE_ASYNC_RENDERER == 2
// Async software rendering functions.
pax_render_funcs_t const pax_render_funcs_mcr_thread0 = {
    .background    = pax_mcrw0_background,
    .unshaded_line = pax_mcrw0_unshaded_line,
    .unshaded_rect = pax_mcrw0_unshaded_rect,
    .unshaded_quad = pax_mcrw0_unshaded_quad,
    .unshaded_tri  = pax_mcrw0_unshaded_tri,
    .shaded_line   = pax_mcrw0_shaded_line,
    .shaded_rect   = pax_mcrw0_shaded_rect,
    .shaded_quad   = pax_mcrw0_shaded_quad,
    .shaded_tri    = pax_mcrw0_shaded_tri,
    .scaled_image  = pax_mcrw0_scaled_image,
    .sprite        = pax_mcrw0_sprite,
    .blit          = pax_mcrw0_blit,
    .blit_raw      = pax_mcrw0_blit_raw,
    .blit_char     = pax_mcrw0_blit_char,
    .text          = pax_mcrw0_text,
};

// Async software rendering functions.
pax_render_funcs_t const pax_render_funcs_mcr_thread1 = {
    .background    = pax_mcrw1_background,
    .unshaded_line = pax_mcrw1_unshaded_line,
    .unshaded_rect = pax_mcrw1_unshaded_rect,
    .unshaded_quad = pax_mcrw1_unshaded_quad,
    .unshaded_tri  = pax_mcrw1_unshaded_tri,
    .shaded_line   = pax_mcrw1_shaded_line,
    .shaded_rect   = pax_mcrw1_shaded_rect,
    .shaded_quad   = pax_mcrw1_shaded_quad,
    .shaded_tri    = pax_mcrw1_shaded_tri,
    .scaled_image  = pax_mcrw1_scaled_image,
    .sprite        = pax_mcrw1_sprite,
    .blit          = pax_mcrw1_blit,
    .blit_raw      = pax_mcrw1_blit_raw,
    .blit_char     = pax_mcrw1_blit_char,
    .text          = pax_mcrw1_text,
};
    #endif

// Async software rendering engine.
pax_render_engine_t const pax_render_engine_softasync = {
    .init           = pax_sasr_init,
    .deinit         = pax_sasr_deinit,
    .implicit_dirty = true,
};

#endif // CONFIG_PAX_COMPILE_ASYNC_RENDERER
