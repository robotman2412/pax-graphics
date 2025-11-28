
// SPDX-License-Identifier: MIT

#include "pax_matrix.h"
#include "pax_orientation.h"
#include "pax_types.h"

#include <math.h>
static char const *TAG = "pax_text";

#include "pax_internal.h"
#include "pax_renderer.h"
#include "string.h"



/* ====== UTF-8 UTILITIES ====== */

// Extracts an UTF-8 code from a string.
// Returns how many bytes were read.
// Sets the decoded UTF-8 using a pointer.
// If the string terminates early or contains invalid unicode, U+FFFD is returned.
size_t pax_utf8_getch_l(char const *cstr, size_t cstr_len, uint32_t *out) {
    char len, mask;
    if (!*cstr || !cstr_len) {
        // Null pointer.
        *out = 0xfffd; // Something something invalid UTF8.
        return 0;
    } else if (!(*cstr & 0x80)) {
        // ASCII point.
        *out = *cstr;
        return 1;
    } else if ((*cstr & 0xe0) == 0xc0) {
        // Two byte point.
        len  = 2;
        mask = 0x1f;
    } else if ((*cstr & 0xf0) == 0xe0) {
        // Three byte point.
        len  = 3;
        mask = 0x0f;
    } else if ((*cstr & 0xf8) == 0xf0) {
        // Four byte point.
        len  = 4;
        mask = 0x07;
    } else {
        // There are no points over four bytes long.
        *out = 0xfffd; // Something something invalid UTF8.
        return 0;
    }

    *out = 0;
    for (int i = 0; i < len; i++) {
        if (!cstr_len || !*cstr) {
            *out = 0xfffd; // Something something invalid UTF8.
            return 0;
        }
        *out <<= 6;
        *out  |= *cstr & mask;
        mask   = 0x3f;
        cstr++;
        cstr_len--;
    }
    return len;
}

// Returns how many UTF-8 characters a given c-string contains.
size_t pax_utf8_strlen_l(char const *cstr, size_t len) {
    uint32_t dummy    = 0;
    size_t   utf8_len = 0;
    while (len) {
        utf8_len++;
        size_t used  = pax_utf8_getch_l(cstr, len, &dummy);
        cstr        += used ?: 1;
        len         -= used ?: 1;
    }
    return utf8_len;
}

// Seek to the next UTF-8 character in a string.
size_t pax_utf8_seeknext_l(char const *cstr, size_t cstr_len, size_t cursor) {
    if (cursor >= cstr_len) {
        return cursor;
    }

    do {
        cursor++;
    } while (cursor < cstr_len && (cstr[cursor] & 0xc0) == 0x80);

    return cursor;
}

// Seek to the previous UTF-8 character in a string.
size_t pax_utf8_seekprev_l(char const *cstr, size_t len, size_t cursor) {
    (void)len;
    if (!cursor) {
        return 0;
    }

    do {
        cursor--;
    } while (cursor && (cstr[cursor] & 0xc0) == 0x80);

    return cursor;
}



/* ======= DRAWING: TEXT ======= */

static uint64_t text_promise_callback_cutout(pax_buf_t *buf, pax_col_t tint, void *args0) {
    (void)buf;
    (void)tint;
    (void)args0;
    return PAX_PROMISE_CUTOUT;
}

static uint64_t text_promise_callback_none(pax_buf_t *buf, pax_col_t tint, void *args0) {
    (void)buf;
    (void)tint;
    (void)args0;
    return 0;
}

// Pixel-aligned optimisation of pax_shade_rect, used for text.
static void pixel_aligned_render(
    pax_text_render_t  *ctx,
    pax_shader_t const *shader,
    pax_quadf const    *uvs,
    float               x,
    float               y,
    float               width,
    float               height
) {
    // Offset and pixel-align co-ordinates.
    x = floorf(0.5 + x + ctx->buf->stack_2d.value.a2);
    y = floorf(0.5 + y + ctx->buf->stack_2d.value.b2);
    pax_mark_dirty2(ctx->buf, x, y, width, height);

#if CONFIG_PAX_COMPILE_ORIENTATION
    pax_rectf tmp = pax_orient_det_rectf(ctx->buf, (pax_rectf){x, y, width, height});
    x             = tmp.x;
    y             = tmp.y;
    width         = tmp.w;
    height        = tmp.h;

    pax_quadf uvs_rotated;
    if (ctx->buf->orientation & 1) {
        uvs_rotated = (pax_quadf){
            uvs->x0,
            uvs->y0,
            uvs->x3,
            uvs->y3,
            uvs->x2,
            uvs->y2,
            uvs->x1,
            uvs->y1,
        };
        uvs = &uvs_rotated;
    }
#endif

    // Single core option.
    ctx->renderfuncs->shaded_rect(
        ctx->buf,
        ctx->color,
        (pax_rectf){
            x,
            y,
            width,
            height,
        },
        shader,
        (pax_quadf){
            uvs->x0,
            uvs->y0,
            uvs->x1,
            uvs->y1,
            uvs->x2,
            uvs->y2,
            uvs->x3,
            uvs->y3,
        }
    );
}

// Dispatch the correct draw call for a glyph.
static void dispatch_glyph(
    pax_text_render_t *ctx, pax_vec2f pos, float scale, pax_font_range_t const *range, pax_text_rsdata_t rsdata
) {
    float mat_scale = ctx->matrix.a0 * scale;
    if (ctx->matrix.a0 > 0 && fabsf(ctx->matrix.a0 - ctx->matrix.b1) < 0.01 && fabsf(mat_scale - (int)mat_scale) < 0.01
        && matrix_2d_is_identity2(ctx->matrix)) {
        // This can be optimized to the special text blitting function.
        ctx->renderfuncs->blit_char(ctx->buf, ctx->color, (pax_vec2i){pos.x, pos.y}, floorf(mat_scale + 0.5), rsdata);
        return;
    }

    // Set up shader.
    pax_shader_t shader = {
        .schema_version    = 1,
        .schema_complement = ~1,
        .renderer_id       = PAX_RENDERER_ID_SWR,
        .callback_args     = (void *)&rsdata,
        .alpha_promise_0   = true,
        .alpha_promise_255 = false,
    };

    // Select correct shader function.
    if ((ctx->buf->type_info.fmt_type == PAX_BUF_SUBTYPE_PALETTE)
        || (range->bitmap_mono.bpp == 1 && ctx->color >> 24 == 255)) {
        shader.promise_callback = text_promise_callback_cutout;
        shader.callback         = pax_shader_font_bmp_pal;
    } else if (ctx->font->recommend_aa) {
        shader.promise_callback = text_promise_callback_none;
        shader.callback         = pax_shader_font_bmp_aa;
    } else {
        shader.promise_callback = text_promise_callback_none;
        shader.callback         = pax_shader_font_bmp;
    }

    // Set UVs to pixel coordinates for the glyph.
    pax_quadf uvs = {
        .x0 = 0,
        .y0 = 0,
        .x1 = rsdata.w,
        .y1 = 0,
        .x2 = rsdata.w,
        .y2 = rsdata.h,
        .x3 = 0,
        .y3 = rsdata.h,
    };

    // Start drawing, boy!
    if (matrix_2d_is_identity2(ctx->buf->stack_2d.value)) {
        // Pixel-aligned optimisation.
        pixel_aligned_render(ctx, &shader, &uvs, pos.x, pos.y, scale * rsdata.w, scale * rsdata.h);
    } else {
        // Generic shader draw required.
        pax_vec2f p0 = matrix_2d_transform_alt(ctx->matrix, (pax_vec2f){pos.x, pos.y});
        pax_vec2f p1 = matrix_2d_transform_alt(ctx->matrix, (pax_vec2f){pos.x + rsdata.w, pos.y});
        pax_vec2f p2 = matrix_2d_transform_alt(ctx->matrix, (pax_vec2f){pos.x + rsdata.w, pos.y + rsdata.h});
        pax_vec2f p3 = matrix_2d_transform_alt(ctx->matrix, (pax_vec2f){pos.x, pos.y + rsdata.h});
#if CONFIG_PAX_COMPILE_ORIENTATION
        p0 = pax_orient_det_vec2f(ctx->buf, p0);
        p1 = pax_orient_det_vec2f(ctx->buf, p1);
        p2 = pax_orient_det_vec2f(ctx->buf, p2);
        p3 = pax_orient_det_vec2f(ctx->buf, p3);
#endif
        pax_quadf shape
            = {.x0 = p0.x, .y0 = p0.y, .x1 = p1.x, .y1 = p1.y, .x2 = p2.x, .y2 = p2.y, .x3 = p3.x, .y3 = p3.y};
        ctx->renderfuncs->shaded_quad(ctx->buf, ctx->color, shape, &shader, uvs);
        // pax_shade_rect(ctx->buf, ctx->color, &shader, &uvs, pos.x, pos.y, scale * rsdata.w, scale * rsdata.h);
    }
}

// Internal method for monospace bitmapped characters.
static pax_vec2f text_bitmap_mono(
    pax_text_render_t *ctx, bool do_render, pax_vec2f pos, float scale, pax_font_range_t const *range, uint32_t glyph
) {
    if (do_render && glyph != 0x20) {
        // Set up glyph rendering information.
        pax_text_rsdata_t rsdata = {
            .bpp = range->bitmap_mono.bpp,
            .w   = range->bitmap_mono.width,
            .h   = range->bitmap_mono.height,
        };
        rsdata.row_stride = (rsdata.w * rsdata.bpp + 7) / 8;
        rsdata.bitmap     = range->bitmap_mono.glyphs + rsdata.row_stride * rsdata.h * (glyph - range->start);

        dispatch_glyph(ctx, pos, scale, range, rsdata);
    }

    // Size calculation is very simple.
    return (pax_vec2f){.x = range->bitmap_mono.width, .y = range->bitmap_mono.height};
}

// Internal method for variable pitch bitmapped characters.
static pax_vec2f text_bitmap_var(
    pax_text_render_t *ctx, bool do_render, pax_vec2f pos, float scale, pax_font_range_t const *range, uint32_t glyph
) {
    size_t            index = (glyph - range->start);
    pax_bmpv_t const *dims  = &range->bitmap_var.dims[index];

    if (do_render && glyph != 0x20) {
        // Set up glyph rendering information.
        pax_text_rsdata_t rsdata = {
            .bpp = range->bitmap_var.bpp,
            .w   = dims->draw_w,
            .h   = dims->draw_h,
        };
        rsdata.row_stride = (rsdata.w * rsdata.bpp + 7) / 8;
        rsdata.bitmap     = range->bitmap_var.glyphs + dims->index;

        pos.x += dims->draw_x * scale;
        pos.y += dims->draw_y * scale;
        dispatch_glyph(ctx, pos, scale, range, rsdata);
    }

    // Size calculation is very simple.
    return (pax_vec2f){.x = dims->measured_width, .y = range->bitmap_var.height};
}

// Determines whether a character lies in a given range.
static inline bool text_range_includes(pax_font_range_t const *range, uint32_t c) {
    return c >= range->start && c <= range->end;
}

// Internal method for determining the font range to use.
// Returns NULL if not in any range.
static pax_font_range_t const *text_get_range(pax_font_t const *font, uint32_t c) {
    // Iterate over ranges.
    for (size_t i = 0; i < font->n_ranges; i++) {
        // Look for the first including the point.
        if (text_range_includes(&font->ranges[i], c)) {
            return &font->ranges[i];
        }
    }
    return NULL;
}

// Internal method for rendering text and calculating text size.
static pax_vec2f text_line_generic_impl(
    pax_text_render_t *ctx, bool do_render, pax_vec2f pos, char const *text, size_t len, ptrdiff_t cursorpos
) {
    // Apply matrix transformation for size.
    float const scale    = ctx->font_size / ctx->font->default_size;
    float       x        = 0;
    float       max_x    = 0;
    float       cursor_x = NAN;

    // Simply loop over all characters.
    size_t                  i     = 0;
    pax_font_range_t const *range = NULL;
    while (i < len) {
        // Draw cursor.
        if ((size_t)cursorpos == i) {
            if (do_render) {
                pax_vec2f p0 = pos;
                pax_vec2f p1
                    = matrix_2d_transform_alt(ctx->matrix, (pax_vec2f){pos.x, pos.y + scale * ctx->font->default_size});
                pax_linef shape = {p0.x, p0.y, p1.x, p1.y};
                ctx->renderfuncs->unshaded_line(ctx->buf, ctx->color, shape);
            }
            cursor_x = x;
        }

        // Get a character.
        uint32_t glyph       = 0;
        size_t   glyph_size  = pax_utf8_getch_l(text + i, len - i, &glyph);
        i                   += glyph_size ?: 1;

        if (glyph == 0xa0) {
            // Non-breaking space is implicitly converted to space.
            glyph = 0x20;
        }

        // Try to find a range the glyph is in.
        if (!range || !text_range_includes(range, glyph)) {
            range = text_get_range(ctx->font, glyph);
        }

        pax_vec2f dims = {0, 0};
        if (range) {
            // Handle the character.
            switch (range->type) {
                case PAX_FONT_TYPE_BITMAP_MONO:
                    dims = text_bitmap_mono(ctx, do_render, pos, scale, range, glyph);
                    break;
                case PAX_FONT_TYPE_BITMAP_VAR: dims = text_bitmap_var(ctx, do_render, pos, scale, range, glyph); break;
            }
        } else {
            // Ignore it for now.
        }
        x     += dims.x;
        pos.x += dims.x * scale;
    }

    // Edge case: Cursor at the end.
    if ((size_t)cursorpos == i) {
        if (do_render) {
            pax_vec2f p0 = pos;
            pax_vec2f p1
                = matrix_2d_transform_alt(ctx->matrix, (pax_vec2f){pos.x, pos.y + scale * ctx->font->default_size});
            pax_linef shape = {p0.x, p0.y, p1.x, p1.y};
            ctx->renderfuncs->unshaded_line(ctx->buf, ctx->color, shape);
        }
        cursor_x = x;
    }

    if (x > max_x) {
        max_x = x;
    }
    return (pax_vec2f){
        scale * max_x,
        scale * cursor_x,
    };
}

// Count the number of newlines in a string.
static inline size_t count_newlines(char const *str, size_t len) {
    size_t found = 0;
    char   prev  = 0;
    while (len) {
        if (*str == '\r') {
            found++;
        } else if (*str == '\n') {
            found += prev != '\r';
        }
        prev = *str;
        str++;
        len--;
    }
    return found;
}

// Count how many bytes of data there is in the current line.
static inline void get_line_length(char const *str, size_t len, size_t *line_len_out, size_t *next_line_out) {
    for (size_t i = 0; i < len; i++) {
        if (str[i] == '\r') {
            *line_len_out = i;
            if (i < len - 1 && str[i + 1] == '\n') {
                *next_line_out = i + 2;
            } else {
                *next_line_out = i + 1;
            }
            return;
        } else if (str[i] == '\n') {
            *line_len_out  = i;
            *next_line_out = i + 1;
            return;
        }
    }
    *line_len_out  = len;
    *next_line_out = len;
}

// Internal method for rendering text and calculating text size.
static inline pax_vec2f text_line_generic(
    pax_text_render_t *ctx, pax_vec2f pos, char const *text, size_t len, pax_align_t halign, ptrdiff_t cursorpos
) {
    if (halign == PAX_ALIGN_BEGIN) {
        return text_line_generic_impl(ctx, ctx->do_render, pos, text, len, cursorpos);
    } else if (halign == PAX_ALIGN_CENTER) {
        pax_vec2f size = text_line_generic_impl(ctx, false, pos, text, len, cursorpos);
        if (ctx->do_render) {
            text_line_generic_impl(ctx, true, (pax_vec2f){pos.x - size.x * 0.5f, pos.y}, text, len, cursorpos);
        }
        return (pax_vec2f){size.x, size.y - size.x * 0.5f};
    } else if (halign == PAX_ALIGN_END) {
        pax_vec2f size = text_line_generic_impl(ctx, false, pos, text, len, cursorpos);
        if (ctx->do_render) {
            text_line_generic_impl(ctx, true, (pax_vec2f){pos.x - size.x, pos.y}, text, len, cursorpos);
        }
        return (pax_vec2f){size.x, size.y - size.x};
    } else {
        return (pax_vec2f){0, NAN};
    }
}

// Internal method for rendering text and calculating text size.
pax_2vec2f pax_internal_text_generic(
    pax_text_render_t *ctx,
    pax_vec2f          pos,
    char const        *text,
    size_t             len,
    ptrdiff_t          cursorpos,
    pax_align_t        halign,
    pax_align_t        valign
) {
    pax_2vec2f size = {0};
    size.y0         = ctx->font_size * (1 + count_newlines(text, len));
    if (valign == PAX_ALIGN_CENTER) {
        pos.y -= size.y0 * 0.5f;
    } else if (valign == PAX_ALIGN_END) {
        pos.y -= size.y0;
    }

    while (len) {
        size_t line_len = len, next_line = len;
        get_line_length(text, len, &line_len, &next_line);
        pax_vec2f line_size = text_line_generic(ctx, pos, text, line_len, halign, cursorpos);
        size.x0             = fmaxf(size.x0, line_size.x);
        if (cursorpos >= 0 && cursorpos < (ptrdiff_t)next_line) {
            size.x1 = line_size.y;
            size.y1 = pos.y;
        }
        text      += next_line;
        len       -= next_line;
        cursorpos -= next_line;
        pos.y     += ctx->font_size;
    }

    return size;
}

// Draw a string with given font, size, alignment and optional cursor index.
// Returns the text size and relative cursor position in a pax_2vec2f.
pax_2vec2f pax_draw_text_adv(
    pax_buf_t        *buf,
    pax_col_t         color,
    pax_font_t const *font,
    float             font_size,
    float             x,
    float             y,
    char const       *text,
    size_t            len,
    pax_align_t       halign,
    pax_align_t       valign,
    ptrdiff_t         cursorpos
) {
    if (!text) {
        return (pax_2vec2f){0, 0, NAN, NAN};
    }
    pax_dispatch_text(
        buf,
        buf->stack_2d.value,
        color,
        font,
        font_size,
        (pax_vec2f){x, y},
        text,
        len,
        halign,
        valign,
        cursorpos
    );
    pax_text_render_t ctx = {
        .do_render = false,
        .font      = font,
        .font_size = font_size,
    };
    return pax_internal_text_generic(&ctx, (pax_vec2f){0, 0}, text, len, cursorpos, halign, valign);
}

// Measure the size of a string with given font, size, alignment and optional cursor index.
// Returns the text size and relative cursor position in a pax_2vec2f.
pax_2vec2f pax_text_size_adv(
    pax_font_t const *font,
    float             font_size,
    char const       *text,
    size_t            len,
    pax_align_t       halign,
    pax_align_t       valign,
    ptrdiff_t         cursorpos
) {
    if (!text) {
        return (pax_2vec2f){0, 0, NAN, NAN};
    }
    pax_text_render_t ctx = {
        .do_render = false,
        .font      = font,
        .font_size = font_size,
    };
    return pax_internal_text_generic(&ctx, (pax_vec2f){0, 0}, text, len, cursorpos, halign, valign);
}



#if 1
// Calculates the size of the region's raw data.
static size_t pax_calc_range_size(pax_font_range_t const *range, bool include_structs) {
    size_t range_size = range->end - range->start + 1;
    size_t size       = include_structs ? sizeof(pax_font_range_t) : 0;
    if (range->type == PAX_FONT_TYPE_BITMAP_MONO) {
        // Based on array CALCULATION.
        size_t bytes_per_line  = (range->bitmap_mono.width * range->bitmap_mono.bpp + 7) / 8;
        size                  += range_size * range->bitmap_mono.height * bytes_per_line;
        return size;

    } else {
        // More complex; based on last index.
        pax_bmpv_t max_index = {.index = 0};

        // Find glyph with highest index.
        for (size_t i = 0; i < range_size; i++) {
            size_t index = range->bitmap_var.dims[i].index;
            if (index > max_index.index)
                max_index = range->bitmap_var.dims[i];
        }

        // Calculate length.
        size_t bytes_per_line  = (max_index.draw_w * range->bitmap_var.bpp + 7) / 8;
        size                  += max_index.index + bytes_per_line * max_index.draw_h;

        if (include_structs) {
            // Calculate size of pax_bmpv_t included.
            size += sizeof(pax_bmpv_t) * range_size;
        }

        return size;
    }
}

// Calculates the size of the region's bitmap data.
static inline size_t pax_calc_range_bitmap_size(pax_font_range_t const *range) {
    return pax_calc_range_size(range, false);
}

// Reads a number from the file (little-endian).
static bool xreadnum(uint64_t *number, size_t bytes, FILE *fd) {
    uint64_t out  = 0;
    size_t   read = 0;
    for (size_t i = 0; i < bytes; i++) {
        uint8_t tmp  = 0;
        read        += fread(&tmp, 1, 1, fd);
        out         |= tmp << (i * 8);
    }
    *number = out;
    return read == bytes;
}

// Writes a number to the file (little-endian).
static bool xwritenum(uint64_t number, size_t bytes, FILE *fd) {
    size_t written = 0;
    for (size_t i = 0; i < bytes; i++) {
        uint8_t tmp   = number;
        written      += fwrite(&tmp, 1, 1, fd);
        number      >>= 8;
    }
    return written == bytes;
}

static inline bool xfwrite(void const *restrict __ptr, size_t __size, size_t __n, FILE *restrict __s) {
    return fwrite(__ptr, __size, __n, __s) == __n;
}

static inline bool xfread(void *restrict __ptr, size_t __size, size_t __n, FILE *restrict __s) {
    return fread(__ptr, __size, __n, __s) == __n;
}

    // Args: size_t *number, size_t bytes, FILE *fd
    #define xreadnum_assert(...)                                                                                       \
        do {                                                                                                           \
            if (!xreadnum(__VA_ARGS__))                                                                                \
                goto fd_error;                                                                                         \
        } while (0)

    // Args: size_t number, size_t bytes, FILE *fd
    #define xwritenum_assert(...)                                                                                      \
        do {                                                                                                           \
            if (!xwritenum(__VA_ARGS__))                                                                               \
                goto fd_error;                                                                                         \
        } while (0)

    // Args: void *ptr, size_t size, size_t n, FILE *fd
    #define fread_assert(...)                                                                                          \
        do {                                                                                                           \
            if (!xfread(__VA_ARGS__))                                                                                  \
                goto fd_error;                                                                                         \
        } while (0)

    // Args: void *ptr, size_t size, size_t n, FILE *fd
    #define fwrite_assert(...)                                                                                         \
        do {                                                                                                           \
            if (!xfwrite(__VA_ARGS__))                                                                                 \
                goto fd_error;                                                                                         \
        } while (0)
#endif

// Loads a font using a file descriptor.
// Allocates the entire font in one go, such that only free(pax_font_t*) is required.
pax_font_t *pax_load_font(FILE *fd) {
    PAX_NULL_CHECK(fd, NULL);
    pax_font_t *out = NULL;
    size_t      out_addr;
    uint64_t    tmpint;

    /* ==== DETERMINE COMPATIBILITY ==== */
    // Validate magic.
    char magic_temp[11] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    fread_assert(magic_temp, 1, 11, fd);
    if (strcmp(magic_temp, "pax_font_t")) {
        // Invalid magic.
        PAX_LOGE(TAG, "Invalid magic in font file");
        pax_set_err(PAX_ERR_CORRUPT);
        return NULL;
    }

    // Validate loader version.
    uint64_t font_version;
    xreadnum_assert(&font_version, sizeof(uint16_t), fd);
    if (font_version != PAX_FONT_LOADER_VERSION) {
        // Different font loader version; unsupported.
        PAX_LOGE(TAG, "Unsupported font version %hu (supported: %hu)", (uint16_t)font_version, PAX_FONT_LOADER_VERSION);
        return NULL;
    }

    /* ==== READ METADATA ==== */
    // Number of stored pax_bmpv_t.
    uint64_t n_bmpv;
    xreadnum_assert(&n_bmpv, sizeof(uint64_t), fd);

    // Size of the combined bitmaps.
    uint64_t n_bitmap;
    xreadnum_assert(&n_bitmap, sizeof(uint64_t), fd);

    // Size of the font name.
    uint64_t n_name;
    xreadnum_assert(&n_name, sizeof(uint64_t), fd);

    // Number of ranges in the font.
    uint64_t n_ranges;
    xreadnum_assert(&n_ranges, sizeof(uint64_t), fd);

    // Calculate required size.
    size_t required_size = sizeof(pax_font_t) + n_ranges * sizeof(pax_font_range_t) + n_bmpv * sizeof(pax_bmpv_t)
                           + n_bitmap + n_name + 1;

    // Validate required size.
    if (required_size < PAX_FONT_LOADER_MINUMUM_SIZE) {
        // The size is suspiciously small.
        PAX_LOGE(
            TAG,
            "File corruption: Font size reported is too small (metadata; %zu < %zu)",
            required_size,
            PAX_FONT_LOADER_MINUMUM_SIZE
        );
        PAX_ERROR(PAX_ERR_UNSUPPORTED, NULL);
    }

    // Allocate memory.
    out      = malloc(required_size);
    out_addr = (size_t)out;
    if (!out) {
        PAX_LOGE(TAG, "Out of memory for loading font (%zu required)", required_size);
        PAX_ERROR(PAX_ERR_NOMEM, NULL);
    }

    out->n_ranges = n_ranges;

    // Default point size.
    xreadnum_assert(&tmpint, sizeof(uint16_t), fd);
    out->default_size = tmpint;

    // Whether antialiassing is recommended.
    xreadnum_assert(&tmpint, sizeof(bool), fd);
    out->recommend_aa = !!tmpint;

    // Validate again whether the memory is enough.
    size_t minimum_size = sizeof(pax_font_t) + out->n_ranges * sizeof(pax_font_range_t) + 3;
    if (required_size < minimum_size) {
        PAX_LOGE(
            TAG,
            "File corruption: Font size reported is too small (range metadata; %zu < %zu)",
            required_size,
            minimum_size
        );
        pax_set_err(PAX_ERR_CORRUPT);
        free(out);
        return NULL;
    }

    /* ==== READ RANGES ==== */
    // Calculate addresses.
    size_t output_offset = sizeof(pax_font_t) + out->n_ranges * sizeof(pax_font_range_t);
    out->ranges          = (void *)(out_addr + sizeof(pax_font_t));

    // Read range data.
    for (size_t i = 0; i < out->n_ranges; i++) {
        pax_font_range_t *range = (pax_font_range_t *)&out->ranges[i];

        // Range type.
        xreadnum_assert(&tmpint, sizeof(uint8_t), fd);
        range->type = tmpint;

        // Range start glyph.
        xreadnum_assert(&tmpint, sizeof(uint32_t), fd);
        range->start = tmpint;

        // Range end glyph.
        xreadnum_assert(&tmpint, sizeof(uint32_t), fd);
        range->end = tmpint;

        size_t range_size = range->end - range->start + 1;

        if (range->type == PAX_FONT_TYPE_BITMAP_MONO) {
            // Glyph width.
            xreadnum_assert(&tmpint, sizeof(uint8_t), fd);
            range->bitmap_mono.width = tmpint;

            // Glyph height.
            xreadnum_assert(&tmpint, sizeof(uint8_t), fd);
            range->bitmap_mono.height = tmpint;

            // Glyph bits per pixel.
            xreadnum_assert(&tmpint, sizeof(uint8_t), fd);
            range->bitmap_mono.bpp = tmpint;

        } else if (range->type == PAX_FONT_TYPE_BITMAP_VAR) {
            // Read later: Additional glyph dimensions.
            // Calculate the address.
            range->bitmap_var.dims = (void *)(out_addr + output_offset);

            // Glyph height.
            xreadnum_assert(&tmpint, sizeof(uint8_t), fd);
            range->bitmap_var.height = tmpint;

            // Glyph bits per pixel.
            xreadnum_assert(&tmpint, sizeof(uint8_t), fd);
            range->bitmap_var.bpp = tmpint;

            // Reassert size requirements.
            minimum_size  += range_size * sizeof(pax_bmpv_t);
            output_offset += range_size * sizeof(pax_bmpv_t);
            if (required_size < minimum_size) {
                PAX_LOGE(
                    TAG,
                    "File corruption: Font size reported is too small (bitmap metadata; %zu < %zu)",
                    required_size,
                    minimum_size
                );
                pax_set_err(PAX_ERR_CORRUPT);
                free(out);
                return NULL;
            }

            // Additional glyph dimensions.
            for (size_t x = 0; x < range_size; x++) {
                pax_bmpv_t *bmpv = (pax_bmpv_t *)&range->bitmap_var.dims[x];

                // Bitmap draw X offset.
                xreadnum_assert(&tmpint, sizeof(uint8_t), fd);
                bmpv->draw_x = tmpint;

                // Bitmap draw Y offset.
                xreadnum_assert(&tmpint, sizeof(uint8_t), fd);
                bmpv->draw_y = tmpint;

                // Bitmap drawn width.
                xreadnum_assert(&tmpint, sizeof(uint8_t), fd);
                bmpv->draw_w = tmpint;

                // Bitmap drawn height.
                xreadnum_assert(&tmpint, sizeof(uint8_t), fd);
                bmpv->draw_h = tmpint;

                // Bitmap measured width.
                xreadnum_assert(&tmpint, sizeof(uint8_t), fd);
                bmpv->measured_width = tmpint;

                // Bitmap index.
                xreadnum_assert(&tmpint, sizeof(uint64_t), fd);
                bmpv->index = tmpint;
            }

        } else {
            // Invalid type.
            PAX_LOGE(TAG, "File corruption: Font type invalid (%u in range %zu)", range->type, i);
            pax_set_err(PAX_ERR_CORRUPT);
            free(out);
            return NULL;
        }
    }

    /* ==== RAW BITMAP DATA ==== */
    fread_assert((void *)(out_addr + output_offset), 1, required_size - output_offset, fd);
    for (size_t i = 0; i < out->n_ranges; i++) {
        pax_font_range_t *range = (pax_font_range_t *)&out->ranges[i];

        if (range->type == PAX_FONT_TYPE_BITMAP_MONO) {
            // Calculate range glyphs address.
            range->bitmap_mono.glyphs  = (void *)(out_addr + output_offset);
            output_offset             += pax_calc_range_bitmap_size(range);

        } else {
            // Calculate range glyphs address.
            range->bitmap_var.glyphs  = (void *)(out_addr + output_offset);
            output_offset            += pax_calc_range_bitmap_size(range);
        }
    }


    return out;


fd_error:
    pax_set_err(PAX_ERR_NODATA);
    if (out)
        free(out);
    return NULL;
}

// Stores a font to a file descriptor.
void pax_store_font(FILE *fd, pax_font_t const *font) {
    PAX_NULL_CHECK(fd);

    /* ==== MAGIC BYTES ==== */
    fwrite_assert("pax_font_t", 1, 11, fd);

    /* ==== PLATFORM METADATA ==== */
    // Font loader version.
    // Files written for another version are assumed incompatible.
    xwritenum_assert(PAX_FONT_LOADER_VERSION, sizeof(uint16_t), fd);

    /* ==== DETERMINE TOTAL SIZE ==== */
    // Calculate total bitmap size.
    size_t total_bitmap = 0;
    for (size_t i = 0; i < font->n_ranges; i++) {
        total_bitmap += pax_calc_range_bitmap_size(&font->ranges[i]);
    }
    // Calculate number of pax_bmpv_t stored.
    size_t total_bmpv = 0;
    for (size_t i = 0; i < font->n_ranges; i++) {
        pax_font_range_t const *range = &font->ranges[i];

        if (range->type == PAX_FONT_TYPE_BITMAP_VAR) {
            total_bmpv += range->end - range->start + 1;
        }
    }

    /* ==== FONT METADATA ==== */
    // Total number of pax_bmpv_t.
    xwritenum_assert(total_bmpv, sizeof(uint64_t), fd);
    // Total size of the bitmap data.
    xwritenum_assert(total_bitmap, sizeof(uint64_t), fd);
    // Length excluding null terminator of the name.
    xwritenum_assert(strlen(font->name), sizeof(uint64_t), fd);
    // Number of ranges in the font.
    xwritenum_assert(font->n_ranges, sizeof(uint64_t), fd);
    // Default size of the font in pixels.
    xwritenum_assert(font->default_size, sizeof(uint16_t), fd);
    // Whether usage of antialiasing is recommended.
    xwritenum_assert(font->recommend_aa, 1, fd);

    /* ==== RANGE DATA ==== */
    for (size_t i = 0; i < font->n_ranges; i++) {
        pax_font_range_t const *range      = &font->ranges[i];
        size_t                  range_size = range->end - range->start + 1;

        // Range type.
        xwritenum_assert(range->type, sizeof(uint8_t), fd);
        // Range start.
        xwritenum_assert(range->start, sizeof(uint32_t), fd);
        // Range end.
        xwritenum_assert(range->end, sizeof(uint32_t), fd);

        if (range->type == PAX_FONT_TYPE_BITMAP_MONO) {
            // Range width.
            xwritenum_assert(range->bitmap_mono.width, sizeof(uint8_t), fd);
            // Range height.
            xwritenum_assert(range->bitmap_mono.height, sizeof(uint8_t), fd);
            // Range bit per pixel.
            xwritenum_assert(range->bitmap_mono.bpp, sizeof(uint8_t), fd);
        } else {
            // Range height.
            xwritenum_assert(range->bitmap_var.height, sizeof(uint8_t), fd);
            // Range bit per pixel.
            xwritenum_assert(range->bitmap_var.bpp, sizeof(uint8_t), fd);

            // Range bitmap dimensions.
            for (size_t x = 0; x < range_size; x++) {
                pax_bmpv_t bmpv = range->bitmap_var.dims[x];

                // Bitmap draw X offset.
                xwritenum_assert(bmpv.draw_x, sizeof(uint8_t), fd);
                // Bitmap draw Y offset.
                xwritenum_assert(bmpv.draw_y, sizeof(uint8_t), fd);
                // Bitmap drawn width.
                xwritenum_assert(bmpv.draw_w, sizeof(uint8_t), fd);
                // Bitmap drawn height.
                xwritenum_assert(bmpv.draw_h, sizeof(uint8_t), fd);
                // Bitmap measured width.
                xwritenum_assert(bmpv.measured_width, sizeof(uint8_t), fd);
                // Bitmap data index.
                xwritenum_assert(bmpv.index, sizeof(uint64_t), fd);
            }
        }
    }

    /* ==== RAW DATA ==== */
    // Write bitmap data.
    for (size_t i = 0; i < font->n_ranges; i++) {
        pax_font_range_t const *range = &font->ranges[i];
        void const             *data;
        // Determine size of raw data.
        size_t                  length = pax_calc_range_bitmap_size(range);

        // Grab raw data.
        if (range->type == PAX_FONT_TYPE_BITMAP_MONO) {
            data = range->bitmap_mono.glyphs;
        } else {
            data = range->bitmap_var.glyphs;
        }

        // Write to the data clump.
        fwrite_assert(data, 1, length, fd);
    }

    // Write font name.
    fwrite_assert(font->name, 1, strlen(font->name) + 1, fd);

    return;


fd_error:
    pax_set_err(PAX_ERR_UNKNOWN);
}
