
// SPDX-License-Identifier: MIT

#include "pax_internal.h"
#include "pax_shaders.h"

#include <endian.h>


static inline void __attribute__((always_inline))
getter_setter_bounds_check(pax_buf_t const *buf, int index, int length) {
#if !CONFIG_PAX_BOUNDS_CHECK
    (void)buf;
    (void)index;
    (void)length;
#else
    if (index < 0 || (length > 0 && (index + length < index || index + length > buf->width * buf->height))) {
        PAX_LOGE(
            "pax",
            "Frame buffer access out of bounds: index %d, length %d on a %dx%d buffer",
            index,
            length,
            buf->width,
            buf->height
        );
        abort();
    }
#endif
}



/* ===== GETTERS AND SETTERS ===== */

// Gets the index getters and setters for the given buffer.
void pax_get_setters(
    pax_buf_t const    *buf,
    pax_index_getter_t *getter,
    pax_index_setter_t *setter,
    pax_range_setter_t *range_setter,
    pax_range_setter_t *range_merger
) {
#if CONFIG_PAX_RANGE_MERGER
    switch (buf->type) {
    #define PAX_DEF_BUF_TYPE_PAL(type_bpp, name)                                                                       \
        case name: *range_merger = pax_range_setter_##type_bpp##bpp; break;
    #define PAX_DEF_BUF_TYPE_GREY(type_bpp, name)                                                                      \
        case name: *range_merger = pax_range_merger_##type_bpp##_grey; break;
    #define PAX_DEF_BUF_TYPE_ARGB(bpp, a, r, g, b, name)                                                               \
        case name: *range_merger = pax_range_merger_##a##r##g##b##argb; break;
    #define PAX_DEF_BUF_TYPE_RGB(bpp, r, g, b, name)                                                                   \
        case name: *range_merger = pax_range_merger_##r##g##b##rgb; break;
    #include "helpers/pax_buf_type.inc"
    }
#else
    *range_merger = pax_range_merger_generic;
#endif
#if CONFIG_PAX_RANGE_SETTER
    switch (buf->type_info.bpp) {
        case (1): *range_setter = pax_range_setter_1bpp; break;
        case (2): *range_setter = pax_range_setter_2bpp; break;
        case (4): *range_setter = pax_range_setter_4bpp; break;
        case (8): *range_setter = pax_range_setter_8bpp; break;
        case (16):
            if (buf->reverse_endianness) {
                *range_setter = pax_range_setter_16bpp_rev;
            } else {
                *range_setter = pax_range_setter_16bpp;
            }
            break;
        case (24):
            if (buf->reverse_endianness) {
                *range_setter = pax_range_setter_24bpp_rev;
            } else {
                *range_setter = pax_range_setter_24bpp;
            }
            break;
        case (32):
            if (buf->reverse_endianness) {
                *range_setter = pax_range_setter_32bpp_rev;
            } else {
                *range_setter = pax_range_setter_32bpp;
            }
            break;
    }
#else
    *range_setter = pax_range_setter_generic;
#endif

    switch (buf->type_info.bpp) {
        case (1):
            *getter = pax_index_getter_1bpp;
            *setter = pax_index_setter_1bpp;
            break;

        case (2):
            *getter = pax_index_getter_2bpp;
            *setter = pax_index_setter_2bpp;
            break;

        case (4):
            *getter = pax_index_getter_4bpp;
            *setter = pax_index_setter_4bpp;
            break;

        case (8):
            *getter = pax_index_getter_8bpp;
            *setter = pax_index_setter_8bpp;
            break;

        case (16):
            if (buf->reverse_endianness) {
                *getter = pax_index_getter_16bpp_rev;
                *setter = pax_index_setter_16bpp_rev;
            } else {
                *getter = pax_index_getter_16bpp;
                *setter = pax_index_setter_16bpp;
            }
            break;

        case (24):
            if (buf->reverse_endianness) {
                *getter = pax_index_getter_24bpp_rev;
                *setter = pax_index_setter_24bpp_rev;
            } else {
                *getter = pax_index_getter_24bpp;
                *setter = pax_index_setter_24bpp;
            }
            break;

        case (32):
            if (buf->reverse_endianness) {
                *getter = pax_index_getter_32bpp_rev;
                *setter = pax_index_setter_32bpp_rev;
            } else {
                *getter = pax_index_getter_32bpp;
                *setter = pax_index_setter_32bpp;
            }
            break;
    }
}


#pragma region index_getter
// Gets a raw value from a 1BPP buffer.
pax_col_t pax_index_getter_1bpp(pax_buf_t const *buf, int index) {
    getter_setter_bounds_check(buf, index, 1);
    return (buf->buf_8bpp[index >> 3] >> (index & 7)) & 1;
}

// Gets a raw value from a 2BPP buffer.
pax_col_t pax_index_getter_2bpp(pax_buf_t const *buf, int index) {
    getter_setter_bounds_check(buf, index, 1);
    return (buf->buf_8bpp[index >> 2] >> (index & 3) * 2) & 3;
}

// Gets a raw value from a 4BPP buffer.
pax_col_t pax_index_getter_4bpp(pax_buf_t const *buf, int index) {
    getter_setter_bounds_check(buf, index, 1);
    return (buf->buf_8bpp[index >> 1] >> (index & 1) * 4) & 15;
}

// Gets a raw value from a 8BPP buffer.
pax_col_t pax_index_getter_8bpp(pax_buf_t const *buf, int index) {
    getter_setter_bounds_check(buf, index, 1);
    return buf->buf_8bpp[index];
}

// Gets a raw value from a 16BPP buffer.
pax_col_t pax_index_getter_16bpp(pax_buf_t const *buf, int index) {
    getter_setter_bounds_check(buf, index, 1);
    return buf->buf_16bpp[index];
}

// Gets a raw value from a 24BPP buffer.
pax_col_t pax_index_getter_24bpp(pax_buf_t const *buf, int index) {
    getter_setter_bounds_check(buf, index, 1);
    index += 2 * index;
    return (buf->buf_8bpp[index + 0] << 0) | (buf->buf_8bpp[index + 1] << 8) | (buf->buf_8bpp[index + 2] << 16);
}

// Gets a raw value from a 32BPP buffer.
pax_col_t pax_index_getter_32bpp(pax_buf_t const *buf, int index) {
    getter_setter_bounds_check(buf, index, 1);
    return buf->buf_32bpp[index];
}

// Gets a raw value from a 16BPP buffer, reversed endianness.
pax_col_t pax_index_getter_16bpp_rev(pax_buf_t const *buf, int index) {
    getter_setter_bounds_check(buf, index, 1);
    return pax_rev_endian_16(buf->buf_16bpp[index]);
}

// Gets a raw value from a 24BPP buffer, reversed endianness.
pax_col_t pax_index_getter_24bpp_rev(pax_buf_t const *buf, int index) {
    getter_setter_bounds_check(buf, index, 1);
    index += 2 * index;
    return (buf->buf_8bpp[index + 0] << 16) | (buf->buf_8bpp[index + 1] << 8) | (buf->buf_8bpp[index + 2] << 0);
}

// Gets a raw value from a 32BPP buffer, reversed endianness.
pax_col_t pax_index_getter_32bpp_rev(pax_buf_t const *buf, int index) {
    getter_setter_bounds_check(buf, index, 1);
    return pax_rev_endian_32(buf->buf_32bpp[index]);
}
#pragma endregion index_getter


#pragma region index_setter
// Sets a raw value from a 1BPP buffer.
void pax_index_setter_1bpp(pax_buf_t *buf, pax_col_t color, int index) {
    getter_setter_bounds_check(buf, index, 1);
    uint8_t *ptr = &buf->buf_8bpp[index >> 3];
    switch (index & 7) {
        case (0): *ptr = (*ptr & 0xfe) | (color << 0); break;
        case (1): *ptr = (*ptr & 0xfd) | (color << 1); break;
        case (2): *ptr = (*ptr & 0xfb) | (color << 2); break;
        case (3): *ptr = (*ptr & 0xf7) | (color << 3); break;
        case (4): *ptr = (*ptr & 0xef) | (color << 4); break;
        case (5): *ptr = (*ptr & 0xdf) | (color << 5); break;
        case (6): *ptr = (*ptr & 0xbf) | (color << 6); break;
        case (7): *ptr = (*ptr & 0x7f) | (color << 7); break;
    }
}

// Sets a raw value from a 2BPP buffer.
void pax_index_setter_2bpp(pax_buf_t *buf, pax_col_t color, int index) {
    getter_setter_bounds_check(buf, index, 1);
    uint8_t *ptr  = &buf->buf_8bpp[index >> 2];
    color        &= 3;
    switch (index & 3) {
        case (0): *ptr = (*ptr & 0xfc) | (color << 0); break;
        case (1): *ptr = (*ptr & 0xf3) | (color << 2); break;
        case (2): *ptr = (*ptr & 0xcf) | (color << 4); break;
        case (3): *ptr = (*ptr & 0x3f) | (color << 6); break;
    }
}

// Sets a raw value from a 4BPP buffer.
void pax_index_setter_4bpp(pax_buf_t *buf, pax_col_t color, int index) {
    getter_setter_bounds_check(buf, index, 1);
    uint8_t *ptr = &buf->buf_8bpp[index >> 1];
    if (index & 1) {
        *ptr = (*ptr & 0x0f) | (color << 4);
    } else {
        *ptr = (*ptr & 0xf0) | color;
    }
}

// Sets a raw value from a 8BPP buffer.
void pax_index_setter_8bpp(pax_buf_t *buf, pax_col_t color, int index) {
    getter_setter_bounds_check(buf, index, 1);
    buf->buf_8bpp[index] = color;
}

// Sets a raw value from a 16BPP buffer.
void pax_index_setter_16bpp(pax_buf_t *buf, pax_col_t color, int index) {
    getter_setter_bounds_check(buf, index, 1);
    buf->buf_16bpp[index] = color;
}

// Sets a raw value from a 24BPP buffer.
void pax_index_setter_24bpp(pax_buf_t *buf, pax_col_t color, int index) {
    getter_setter_bounds_check(buf, index, 1);
    index += 2 * index;
#if BYTE_ORDER == LITTLE_ENDIAN
    buf->buf_8bpp[index + 0] = color >> 0;
    buf->buf_8bpp[index + 1] = color >> 8;
    buf->buf_8bpp[index + 2] = color >> 16;
#else
    buf->buf_8bpp[index + 0] = color >> 16;
    buf->buf_8bpp[index + 1] = color >> 8;
    buf->buf_8bpp[index + 2] = color >> 0;
#endif
}

// Sets a raw value from a 32BPP buffer.
void pax_index_setter_32bpp(pax_buf_t *buf, pax_col_t color, int index) {
    getter_setter_bounds_check(buf, index, 1);
    buf->buf_32bpp[index] = color;
}

// Sets a raw value from a 16BPP buffer, reversed endianness.
void pax_index_setter_16bpp_rev(pax_buf_t *buf, pax_col_t color, int index) {
    getter_setter_bounds_check(buf, index, 1);
    buf->buf_16bpp[index] = pax_rev_endian_16(color);
}

// Sets a raw value from a 24BPP buffer, reversed endianness.
void pax_index_setter_24bpp_rev(pax_buf_t *buf, pax_col_t color, int index) {
    getter_setter_bounds_check(buf, index, 1);
    index += 2 * index;
#if BYTE_ORDER == LITTLE_ENDIAN
    buf->buf_8bpp[index + 0] = color >> 16;
    buf->buf_8bpp[index + 1] = color >> 8;
    buf->buf_8bpp[index + 2] = color >> 0;
#else
    buf->buf_8bpp[index + 0] = color >> 0;
    buf->buf_8bpp[index + 1] = color >> 8;
    buf->buf_8bpp[index + 2] = color >> 16;
#endif
}

// Sets a raw value from a 32BPP buffer, reversed endianness.
void pax_index_setter_32bpp_rev(pax_buf_t *buf, pax_col_t color, int index) {
    getter_setter_bounds_check(buf, index, 1);
    buf->buf_32bpp[index] = pax_rev_endian_32(color);
}
#pragma endregion index_setter


#pragma region range_setter
#if CONFIG_PAX_RANGE_SETTER
// Sets a raw value range from a 1BPP buffer.
void pax_range_setter_1bpp(pax_buf_t *buf, pax_col_t color, int index, int count) {
    getter_setter_bounds_check(buf, index, count);
    if (!count) {
        return;
    }
    color &= 0x1;
    int i  = index;
    while (i & 7) {
        pax_index_setter_1bpp(buf, color, i);
        i++;
    }
    memset(buf->buf_8bpp + i / 8, color * 0x55, (index + count - i) / 8);
    i += (index + count - i) / 8 * 8;
    while (i < index + count) {
        pax_index_setter_1bpp(buf, color, i);
        i++;
    }
}

// Sets a raw value range from a 2BPP buffer.
void pax_range_setter_2bpp(pax_buf_t *buf, pax_col_t color, int index, int count) {
    getter_setter_bounds_check(buf, index, count);
    if (!count) {
        return;
    }
    color &= 0x3;
    int i  = index;
    while (i & 3) {
        pax_index_setter_2bpp(buf, color, i);
        i++;
    }
    memset(buf->buf_8bpp + i / 4, color * 0x55, (index + count - i) / 4);
    i += (index + count - i) / 4 * 4;
    while (i < index + count) {
        pax_index_setter_2bpp(buf, color, i);
        i++;
    }
}

// Sets a raw value range from a 4BPP buffer.
void pax_range_setter_4bpp(pax_buf_t *buf, pax_col_t color, int index, int count) {
    getter_setter_bounds_check(buf, index, count);
    if (!count) {
        return;
    }
    color &= 0xf;
    int i  = index;
    if (i & 1) {
        pax_index_setter_4bpp(buf, color, i);
        i++;
    }
    memset(buf->buf_8bpp + i / 2, color * 0x55, (index + count - i) / 2);
    i += (index + count - i) / 2 * 2;
    if (i < index + count) {
        pax_index_setter_4bpp(buf, color, i);
    }
}

// Sets a raw value range from a 8BPP buffer.
void pax_range_setter_8bpp(pax_buf_t *buf, pax_col_t color, int index, int count) {
    getter_setter_bounds_check(buf, index, count);
    memset(buf->buf_8bpp + index, color, count);
}

// Sets a raw value range from a 16BPP buffer.
void pax_range_setter_16bpp(pax_buf_t *buf, pax_col_t color, int index, int count) {
    getter_setter_bounds_check(buf, index, count);
    if (!count) {
        return;
    }
    color &= 0xffff;
    int i  = index;
    if (i & 1) {
        pax_index_setter_16bpp(buf, color, i);
        i++;
    }
    uint32_t *ptr = (uint32_t *)(buf->buf_16bpp + i);
    for (; i + 1 < index + count; i += 2) {
        *ptr = color | (color << 16);
        ptr++;
    }
    if (i < index + count) {
        pax_index_setter_16bpp(buf, color, i);
    }
}

// Sets a raw value range from a 24BPP buffer.
void pax_range_setter_24bpp(pax_buf_t *buf, pax_col_t color, int index, int count) {
    getter_setter_bounds_check(buf, index, count);
    if (!count) {
        return;
    }
    color &= 0xffffff;
    int i  = 0;
    if (index & 1) {
        pax_index_setter_24bpp(buf, color, i);
        i++;
    }
    uint16_t *ptr = (uint16_t *)(buf->buf_8bpp + (index + i) * 3);
    for (; i < count - 1; i += 2) {
    #if BYTE_ORDER == LITTLE_ENDIAN
        ptr[0] = color;
        ptr[1] = (color >> 16) | ((color & 255) << 8);
        ptr[2] = color >> 8;
    #else
        ptr[0] = color >> 8;
        ptr[1] = (color >> 16) | ((color & 255) << 8);
        ptr[2] = color;
    #endif
        ptr += 3;
    }
    if (i < count) {
        pax_index_setter_24bpp(buf, color, i);
    }
}

// Sets a raw value range from a 32BPP buffer.
void pax_range_setter_32bpp(pax_buf_t *buf, pax_col_t color, int index, int count) {
    getter_setter_bounds_check(buf, index, count);
    for (int i = index; i < index + count; i++) {
        buf->buf_32bpp[i] = color;
    }
}

// Sets a raw value range from a 16BPP buffer, reversed endianness.
void pax_range_setter_16bpp_rev(pax_buf_t *buf, pax_col_t color, int index, int count) {
    getter_setter_bounds_check(buf, index, count);
    pax_range_setter_16bpp(buf, pax_rev_endian_16(color), index, count);
}

// Sets a raw value range from a 24BPP buffer, reversed endianness.
void pax_range_setter_24bpp_rev(pax_buf_t *buf, pax_col_t color, int index, int count) {
    getter_setter_bounds_check(buf, index, count);
    pax_range_setter_24bpp(buf, pax_rev_endian_24(color), index, count);
}

// Sets a raw value range from a 32BPP buffer, reversed endianness.
void pax_range_setter_32bpp_rev(pax_buf_t *buf, pax_col_t color, int index, int count) {
    getter_setter_bounds_check(buf, index, count);
    pax_range_setter_32bpp(buf, pax_rev_endian_32(color), index, count);
}
#else
// Sets a raw value range from any buffer.
void pax_range_setter_generic(pax_buf_t *buf, pax_col_t color, int index, int count) {
    for (int i = 0; i < count; i++) {
        pax_col_t base = buf->buf2col(buf, buf->getter(buf, i + index));
        buf->setter(buf, color, i + index);
    }
}
#endif
#pragma endregion range_setter


#pragma region range_merger
#if CONFIG_PAX_RANGE_MERGER

    #define pax_rev_endian_1(x) (x)
    #define pax_rev_endian_2(x) (x)
    #define pax_rev_endian_4(x) (x)
    #define pax_rev_endian_8(x) (x)

    #define GENERIC_RANGE_MERGER(type, type_bpp)                                                                       \
        PAX_PERF_CRITICAL_ATTR void pax_range_merger_##type(pax_buf_t *buf, pax_col_t color, int index, int count) {   \
            getter_setter_bounds_check(buf, index, count);                                                             \
            int i = 0;                                                                                                 \
            if (type_bpp > 8 && buf->reverse_endianness) {                                                             \
                while (i < count) {                                                                                    \
                    pax_col_t base = pax_##type##_to_col(                                                              \
                        buf,                                                                                           \
                        pax_rev_endian_##type_bpp(pax_index_getter_##type_bpp##bpp(buf, i + index))                    \
                    );                                                                                                 \
                    pax_col_t merged = pax_col_merge(base, color);                                                     \
                    pax_index_setter_##type_bpp##bpp(                                                                  \
                        buf,                                                                                           \
                        pax_rev_endian_##type_bpp(pax_col_to_##type(buf, merged)),                                     \
                        i + index                                                                                      \
                    );                                                                                                 \
                    i++;                                                                                               \
                }                                                                                                      \
            } else {                                                                                                   \
                while (i < count) {                                                                                    \
                    pax_col_t base   = pax_##type##_to_col(buf, pax_index_getter_##type_bpp##bpp(buf, i + index));     \
                    pax_col_t merged = pax_col_merge(base, color);                                                     \
                    pax_index_setter_##type_bpp##bpp(buf, pax_col_to_##type(buf, merged), i + index);                  \
                    i++;                                                                                               \
                }                                                                                                      \
            }                                                                                                          \
        }
    #define PAX_DEF_BUF_TYPE_PAL(bpp, name)
    #define PAX_DEF_BUF_TYPE_GREY(bpp, name)             GENERIC_RANGE_MERGER(bpp##_grey, bpp)
    #define PAX_DEF_BUF_TYPE_ARGB(bpp, a, r, g, b, name) GENERIC_RANGE_MERGER(a##r##g##b##argb, bpp)
    #define PAX_DEF_BUF_TYPE_RGB(bpp, r, g, b, name)     GENERIC_RANGE_MERGER(r##g##b##rgb, bpp)
    #include "helpers/pax_buf_type.inc"

#else
// Merges a single 32-bit ARGB color into a range of pixels.
void pax_range_merger_generic(pax_buf_t *buf, pax_col_t color, int index, int count) {
    for (int i = 0; i < count; i++) {
        pax_col_t base = buf->buf2col(buf, buf->getter(buf, i + index));
        buf->setter(buf, buf->col2buf(buf, pax_col_merge(base, color)), i + index);
    }
}
#endif
#pragma endregion range_merger


// Gets the most efficient index setter for the occasion.
// Also converts the color, if applicable.
// Returns NULL when setting is not required.
pax_index_setter_t pax_get_setter(pax_buf_t const *buf, pax_col_t *col_ptr, pax_shader_t const *shader) {
    pax_col_t col = *col_ptr;

    if (buf->type_info.fmt_type == PAX_BUF_SUBTYPE_PALETTE) {
        return pax_do_draw_col(buf, col) ? buf->setter : NULL;
    }

    if (shader && (shader->callback == pax_shader_texture || shader->callback == pax_shader_texture_aa)) {
        // We can determine whether to factor in alpha based on buffer type.
        if (((pax_buf_t *)shader->callback_args)->type_info.fmt_type == PAX_BUF_SUBTYPE_PALETTE) {
            // If alpha needs factoring in, return the merging setter.
            return col & 0xff000000 ? pax_merge_index : NULL;
        } else {
            // If alpha doesn't need factoring in, return the converting setter.
            return col & 0xff000000 ? pax_set_index_conv : NULL;
        }

    } else if (shader) {
        // More generic shaders, including text.
        if (!(col & 0xff000000) && shader->alpha_promise_0) {
            // When a shader promises to have 0 alpha on 0 alpha tint, return NULL.
            return NULL;
        } else if ((col & 0xff000000) == 0xff000000 && shader->alpha_promise_255) {
            // When a shader promises to have 255 alpha on 255 alpha tint, return converting setter.
            return pax_set_index_conv;
        } else {
            // When no promises are made, fall back to the merging setter.
            return pax_merge_index;
        }

    } else if (!(col & 0xff000000)) {
        // If no shader and alpha is 0, don't set.
        return NULL;

    } else if ((col & 0xff000000) == 0xff000000) {
        // If no shader and 255 alpha, convert color and return raw setter.
        *col_ptr = buf->col2buf(buf, col);
        return buf->setter;

    } else {
        // If no shader and partial alpha, return merging setter.
        return pax_merge_index;
    }
}

// Gets the most efficient range setter/merger for the occasion.
// Also converts the color, if applicable.
// Returns NULL when setting is not required.
pax_range_setter_t pax_get_range_setter(pax_buf_t const *buf, pax_col_t *col_ptr) {
    pax_col_t col = *col_ptr;

    if (buf->type_info.fmt_type == PAX_BUF_SUBTYPE_PALETTE) {
        return pax_do_draw_col(buf, col) ? buf->range_setter : NULL;
    }

    if (!(col & 0xff000000)) {
        // If no shader and alpha is 0, don't set.
        return NULL;

    } else if ((col & 0xff000000) == 0xff000000) {
        // If no shader and 255 alpha, convert color and return raw setter.
        *col_ptr = buf->col2buf(buf, col);
        return buf->range_setter;

    } else {
        // If no shader and partial alpha, return merging setter.
        return buf->range_merger;
    }
}

// Gets based on index instead of coordinates.
// Does no bounds checking nor color conversion.
pax_col_t pax_get_index(pax_buf_t const *buf, int index) {
    return buf->getter(buf, index);
}

// Gets based on index instead of coordinates.
// Does no bounds checking.
pax_col_t pax_get_index_conv(pax_buf_t const *buf, int index) {
    return buf->buf2col(buf, buf->getter(buf, index));
}

// Sets based on index instead of coordinates.
// Does no bounds checking nor color conversion.
void pax_set_index(pax_buf_t *buf, pax_col_t color, int index) {
    buf->setter(buf, color, index);
}

// Sets based on index instead of coordinates.
// Does no bounds checking.
void pax_set_index_conv(pax_buf_t *buf, pax_col_t col, int index) {
    buf->setter(buf, buf->col2buf(buf, col), index);
}

// Merges based on index instead of coordinates. Does no bounds checking.
void pax_merge_index(pax_buf_t *buf, pax_col_t col, int index) {
    pax_col_t base = buf->buf2col(buf, buf->getter(buf, index));
    pax_col_t res  = buf->col2buf(buf, pax_col_merge(base, col));
    buf->setter(buf, res, index);
}



/* ======= COLOR CONVERSION ====== */

// Get the correct color conversion methods for the buffer type.
void pax_get_col_conv(pax_buf_t const *buf, pax_col_conv_t *col2buf, pax_col_conv_t *buf2col) {
    switch (buf->type) {
        case (PAX_BUF_1_PAL):
            *col2buf = pax_trunc_to_1;
            *buf2col = pax_pal_lookup;
            break;

        case (PAX_BUF_2_PAL):
            *col2buf = pax_trunc_to_2;
            *buf2col = pax_pal_lookup;
            break;

        case (PAX_BUF_4_PAL):
            *col2buf = pax_trunc_to_4;
            *buf2col = pax_pal_lookup;
            break;

        case (PAX_BUF_8_PAL):
            *col2buf = pax_trunc_to_8;
            *buf2col = pax_pal_lookup;
            break;

        case (PAX_BUF_16_PAL):
            *col2buf = pax_trunc_to_16;
            *buf2col = pax_pal_lookup;
            break;


        case (PAX_BUF_1_GREY):
            *col2buf = pax_col_to_1_grey;
            *buf2col = pax_1_grey_to_col;
            break;

        case (PAX_BUF_2_GREY):
            *col2buf = pax_col_to_2_grey;
            *buf2col = pax_2_grey_to_col;
            break;

        case (PAX_BUF_4_GREY):
            *col2buf = pax_col_to_4_grey;
            *buf2col = pax_4_grey_to_col;
            break;

        case (PAX_BUF_8_GREY):
            *col2buf = pax_col_to_8_grey;
            *buf2col = pax_8_grey_to_col;
            break;


        case (PAX_BUF_8_332RGB):
            *col2buf = pax_col_to_332rgb;
            *buf2col = pax_332rgb_to_col;
            break;

        case (PAX_BUF_16_565RGB):
            *col2buf = pax_col_to_565rgb;
            *buf2col = pax_565rgb_to_col;
            break;


        case (PAX_BUF_4_1111ARGB):
            *col2buf = pax_col_to_1111argb;
            *buf2col = pax_1111argb_to_col;
            break;

        case (PAX_BUF_8_2222ARGB):
            *col2buf = pax_col_to_2222argb;
            *buf2col = pax_2222argb_to_col;
            break;

        case (PAX_BUF_16_4444ARGB):
            *col2buf = pax_col_to_4444argb;
            *buf2col = pax_4444argb_to_col;
            break;

        case (PAX_BUF_24_888RGB):
            *col2buf = pax_col_conv_dummy;
            *buf2col = pax_888rgb_to_col;
            break;

        case (PAX_BUF_32_8888ARGB):
            *col2buf = pax_col_conv_dummy;
            *buf2col = pax_col_conv_dummy;
            break;
    }
}


// Dummy color converter, returns color input directly.
pax_col_t pax_col_conv_dummy(pax_buf_t const *buf, pax_col_t color) {
    (void)buf;
    return color;
}

// Truncates input to 1 bit.
pax_col_t pax_trunc_to_1(pax_buf_t const *buf, pax_col_t color) {
    (void)buf;
    return color & 1;
}

// Truncates input to 2 bit.
pax_col_t pax_trunc_to_2(pax_buf_t const *buf, pax_col_t color) {
    (void)buf;
    return color & 3;
}

// Truncates input to 4 bit.
pax_col_t pax_trunc_to_4(pax_buf_t const *buf, pax_col_t color) {
    (void)buf;
    return color & 15;
}

// Truncates input to 8 bit.
pax_col_t pax_trunc_to_8(pax_buf_t const *buf, pax_col_t color) {
    (void)buf;
    return color & 255;
}

// Truncates input to 16 bit.
pax_col_t pax_trunc_to_16(pax_buf_t const *buf, pax_col_t color) {
    (void)buf;
    return color & 65535;
}



// Converts ARGB to 1-bit greyscale (AKA black/white).
pax_col_t pax_col_to_1_grey(pax_buf_t const *buf, pax_col_t color) {
    (void)buf;
    uint_fast16_t total = (color & 0x0000ff) + ((color & 0x00ff00) >> 8) + ((color & 0xff0000) >> 16);
    return total > 128 * 3;
}

// Converts ARGB to 2-bit greyscale.
pax_col_t pax_col_to_2_grey(pax_buf_t const *buf, pax_col_t color) {
    (void)buf;
    uint_fast8_t total = ((color & 0x0000c0) >> 6) + ((color & 0x00c000) >> 14) + ((color & 0xc00000) >> 22);
    return total / 3;
}

// Converts ARGB to 4-bit greyscale.
pax_col_t pax_col_to_4_grey(pax_buf_t const *buf, pax_col_t color) {
    (void)buf;
    uint_fast8_t total = ((color & 0x0000f0) >> 4) + ((color & 0x00f000) >> 12) + ((color & 0xf00000) >> 20);
    return total / 3;
}

// Converts ARGB to 8-bit greyscale.
pax_col_t pax_col_to_8_grey(pax_buf_t const *buf, pax_col_t color) {
    (void)buf;
    uint_fast16_t total = (color & 0x0000ff) + ((color & 0x00ff00) >> 8) + ((color & 0xff0000) >> 16);
    return total / 3;
}



// Converts ARGB to 3, 3, 2 bit RGB.
pax_col_t pax_col_to_332rgb(pax_buf_t const *buf, pax_col_t color) {
    (void)buf;
    // 8BPP 332-RGB
    // From: Aaaa aaaa Rrrr rrrr Gggg gggg Bbbb bbbb
    // To:                                 RrrG ggBb
    uint16_t value = ((color >> 16) & 0xe0) | ((color >> 11) & 0x1c) | ((color >> 6) & 0x03);
    return value;
}

// Converts ARGB to 5, 6, 5 bit RGB.
pax_col_t pax_col_to_565rgb(pax_buf_t const *buf, pax_col_t color) {
    (void)buf;
    // 16BPP 565-RGB
    // From: Aaaa aaaa Rrrr rrrr Gggg gggg Bbbb bbbb
    // To:                       Rrrr rGgg gggB bbbb
    return ((color >> 8) & 0xf800) | ((color >> 5) & 0x07e0) | ((color >> 3) & 0x001f);
}



// Converts ARGB to 1 bit per channel ARGB.
pax_col_t pax_col_to_1111argb(pax_buf_t const *buf, pax_col_t color) {
    (void)buf;
    // 4BPP 1111-ARGB
    // From: Aaaa aaaa Rrrr rrrr Gggg gggg Bbbb bbbb
    // To:                                      ARGB
    uint16_t value = ((color >> 28) & 0x8) | ((color >> 21) & 0x4) | ((color >> 14) & 0x2) | ((color >> 7) & 0x1);
    return value;
}

// Converts ARGB to 2 bit per channel ARGB.
pax_col_t pax_col_to_2222argb(pax_buf_t const *buf, pax_col_t color) {
    (void)buf;
    // 8BPP 2222-ARGB
    // From: Aaaa aaaa Rrrr rrrr Gggg gggg Bbbb bbbb
    // To:                                 AaRr GgBb
    uint16_t value = ((color >> 24) & 0xc0) | ((color >> 18) & 0x30) | ((color >> 12) & 0x0c) | ((color >> 6) & 0x03);
    return value;
}

// Converts ARGB to 4 bit per channel ARGB.
pax_col_t pax_col_to_4444argb(pax_buf_t const *buf, pax_col_t color) {
    (void)buf;
    // 16BPP 4444-ARGB
    // From: Aaaa aaaa Rrrr rrrr Gggg gggg Bbbb bbbb
    // To:                       Aaaa Rrrr Gggg Bbbb
    return ((color >> 16) & 0xf000) | ((color >> 12) & 0x0f00) | ((color >> 8) & 0x00f0) | ((color >> 4) & 0x000f);
}


// Performs a palette lookup based on the input.
pax_col_t pax_pal_lookup(pax_buf_t const *buf, pax_col_t index) {
    return (index >= buf->palette_size) ? *buf->palette : buf->palette[index];
}



// Converts 1-bit greyscale (AKA black/white) to ARGB.
pax_col_t pax_1_grey_to_col(pax_buf_t const *buf, pax_col_t color) {
    (void)buf;
    return color ? 0xffffffff : 0xff000000;
}

// Converts 2-bit greyscale to ARGB.
pax_col_t pax_2_grey_to_col(pax_buf_t const *buf, pax_col_t color) {
    (void)buf;
    static pax_col_t const arr[] = {
        0xff000000,
        0xff555555,
        0xffaaaaaa,
        0xffffffff,
    };
    return arr[color];
}

// Converts 4-bit greyscale to ARGB.
pax_col_t pax_4_grey_to_col(pax_buf_t const *buf, pax_col_t color) {
    (void)buf;
    return 0xff000000 | (color * 0x00111111);
}

// Converts 8-bit greyscale to ARGB.
pax_col_t pax_8_grey_to_col(pax_buf_t const *buf, pax_col_t color) {
    (void)buf;
    return 0xff000000 | (color * 0x00010101);
}



// Converts 3, 3, 2 bit RGB to ARGB.
pax_col_t pax_332rgb_to_col(pax_buf_t const *buf, pax_col_t value) {
    (void)buf;
    // 8BPP 332-RGB
    // From:                               RrrG ggBb
    // To:   .... .... Rrr. .... Ggg. .... .... ....
    // Add:  .... .... ...R rrRr ...Gg gGg .... ....
    // Add:  .... .... .... .... .... .... BbBb BbBb
    pax_col_t color  = ((value << 16) & 0x00e00000) | ((value << 11) & 0x0000e000);
    color           |= (color >> 3) | ((color >> 6) & 0x000f0f00);
    pax_col_t temp   = (value & 0x03);
    temp            |= temp << 2;
    color           |= temp | (temp << 4);
    return color | 0xff000000;
}

// Converts 5, 6, 5 bit RGB to ARGB.
pax_col_t pax_565rgb_to_col(pax_buf_t const *buf, pax_col_t value) {
    (void)buf;
    // 16BPP 565-RGB
    // From:                     Rrrr rGgg gggB bbbb
    // To:   .... .... Rrrr r... Gggg gg.. Bbbb b...
    // Add:  .... .... .... .Rrr .... ..Gg .... .Bbb
    // Take the existing information.
    pax_col_t color  = ((value << 8) & 0x00f80000) | ((value << 5) & 0x0000fc00) | ((value << 3) & 0x000000f8);
    // Now, fill in some missing bits.
    color           |= ((value << 3) & 0x00070000) | ((value >> 1) & 0x00000300) | ((value >> 2) & 0x00000007);
    return color | 0xff000000;
}



// Converts 1 bit per channel ARGB to ARGB.
pax_col_t pax_1111argb_to_col(pax_buf_t const *buf, pax_col_t value) {
    (void)buf;
    // 4BPP 1111-ARGB
    // From:                                    ARGB
    // To:   Aaaa aaaa Rrrr rrrr Gggg gggg Bbbb bbbb
    pax_col_t color = ((value << 28) & 0x80000000) | ((value << 21) & 0x00800000) | ((value << 14) & 0x00008000)
                      | ((value << 7) & 0x00000080);
    color |= color >> 1;
    color |= color >> 2;
    color |= color >> 4;
    return color;
}

// Converts 2 bit per channel ARGB to ARGB.
pax_col_t pax_2222argb_to_col(pax_buf_t const *buf, pax_col_t value) {
    (void)buf;
    // 8BPP 2222-ARGB
    // From:                               AaRr GgBb
    // To:   Aaaa aaaa Rrrr rrrr Gggg gggg Bbbb bbbb
    pax_col_t color = ((value << 24) & 0xc0000000) | ((value << 18) & 0x00c00000) | ((value << 12) & 0x0000c000)
                      | ((value << 6) & 0x000000c0);
    color |= color >> 2;
    color |= color >> 4;
    return color;
}

// Converts 4 bit per channel ARGB to ARGB.
pax_col_t pax_4444argb_to_col(pax_buf_t const *buf, pax_col_t value) {
    (void)buf;
    // 16BPP 4444-ARGB
    // From:                     Aaaa Rrrr Gggg Bbbb
    // To:   Aaaa .... Rrrr .... Gggg .... Bbbb ....
    // Add:  .... Aaaa .... Rrrr .... Gggg .... Bbbb
    pax_col_t color = ((value << 16) & 0xf0000000) | ((value << 12) & 0x00f00000) | ((value << 8) & 0x0000f000)
                      | ((value << 4) & 0x000000f0);
    // Now fill in some missing bits.
    color |= color >> 4;
    return color;
}

// Converts 8 bit per channel RGB to ARGB.
pax_col_t pax_888rgb_to_col(pax_buf_t const *buf, pax_col_t color) {
    (void)buf;
    return color | 0xff000000;
}
