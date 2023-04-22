/*
	MIT License

	Copyright (c) 2021-2023 Julian Scheffers

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
	SOFTWARE.
*/

#ifndef PAX_TYPES_H
#define PAX_TYPES_H

#include "pax_matrix.h"
#include "pax_config.h"

#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

#ifndef M_PI
#define M_PI 3.141592653589793
#endif

/* ===== VERSION INFORMATION =====*/

// The version of PAX font files used by the font loader.
#define PAX_FONT_LOADER_VERSION 1
// The version of the shader schema.
// Currently, schema version 0 is accepted and is interpreted as shaders as of v1.0.0.
#define PAX_SHADER_VERSION 1
// The identifier used by shaders for software rendering.
// A different ID is interpreted as belonging to an incompatble method of rendering.
// IDs at and above 0x80 are distributed to third party renderers.
#define PAX_RENDERER_ID_SWR 0x00

// A human-readable representation of the current version number.
#define PAX_VERSION_STR "1.1.0-snapshot"
// A numeric representation of the version, one decimal digit per version part (MAJOR.MINOR.PATCH).
#define PAX_VERSION_NUMBER 110
// Whether this is a prerelease version of PAX.
#define PAX_VERSION_IS_SNAPSHOT true
// The MAJOR part of the version (MAJOR.MINOR.PATCH).
#define PAX_VERSION_MAJOR 1
// The MINOR part of the version (MAJOR.MINOR.PATCH).
#define PAX_VERSION_MINOR 1
// The PATCH part of the version (MAJOR.MINOR.PATCH).
#define PAX_VERSION_PATCH 0

/* ========= ERROR DEFS ========== */

// Unknown error.
#define PAX_ERR_UNKNOWN      1
// All is good.
#define PAX_OK               0
// Buffer pointer is null.
#define PAX_ERR_NOBUF       -1
// Out of memory.
#define PAX_ERR_NOMEM       -2
// Invalid parameters.
#define PAX_ERR_PARAM       -3
// Infinite parameters.
#define PAX_ERR_INF         -4
// Out of bounds parameters.
#define PAX_ERR_BOUNDS      -5
// Matrix stack underflow.
#define PAX_ERR_UNDERFLOW   -6
// Out of data.
#define PAX_ERR_NODATA      -7
// Image decoding error.
#define PAX_ERR_DECODE      -8
// Unsupported operation (or not compiled in).
#define PAX_ERR_UNSUPPORTED -9
// Corruption in buffer.
#define PAX_ERR_CORRUPT     -10

/* ============ TYPES ============ */

// More verbose way of saying reset only the top matrix.
#define PAX_RESET_TOP 0
// More verbose way of saying reset the whole matrix stack.
#define PAX_RESET_ALL 1

// The way pixel data is to be stored in a buffer.
enum pax_buf_type {
	PAX_BUF_1_PAL       = 0x20000001,
	PAX_BUF_2_PAL       = 0x20000002,
	PAX_BUF_4_PAL       = 0x20000004,
	PAX_BUF_8_PAL       = 0x20000008,
	PAX_BUF_16_PAL      = 0x20000010,
	
	PAX_BUF_1_GREY      = 0x10000001,
	PAX_BUF_2_GREY      = 0x10000002,
	PAX_BUF_4_GREY      = 0x10000004,
	PAX_BUF_8_GREY      = 0x10000008,
	
	PAX_BUF_8_332RGB    = 0x00033208,
	PAX_BUF_16_565RGB   = 0x00056510,
	
	PAX_BUF_4_1111ARGB  = 0x00111104,
	PAX_BUF_8_2222ARGB  = 0x00444408,
	PAX_BUF_16_4444ARGB = 0x00444410,
	PAX_BUF_32_8888ARGB = 0x00888820
};

// A way in which to perform word wrap.
enum pax_word_wrap {
	// Do not perform word wrap.
	PAX_WW_NONE,
	// Word wrap by the letter.
	PAX_WW_LETTER,
	// Word wrap by the word.
	PAX_WW_WORD,
	// Word wrap with inter-word justfication.
	PAX_WW_JUSTIFY,
};

// Type of task to do.
// Things like text and arcs will decompose to rects and triangles.
enum pax_task_type {
	// Rectangle draw.
	PAX_TASK_RECT,
	// Triangle draw.
	PAX_TASK_TRI,
	// Stop MCR workder.
	PAX_TASK_STOP,
};

// Distinguishes between ways to draw fonts.
enum pax_font_type {
	// For monospace bitmapped fonts.
	PAX_FONT_TYPE_BITMAP_MONO,
	// For variable pitch bitmapped fonts.
	PAX_FONT_TYPE_BITMAP_VAR,
};

typedef enum   pax_buf_type    pax_buf_type_t;
typedef enum   pax_word_wrap   pax_word_wrap_t;
typedef enum   pax_task_type   pax_task_type_t;
typedef enum   pax_font_type   pax_font_type_t;

// Promises that the shape will be fully opaque when drawn.
#define PAX_PROMISE_OPAQUE		0x01
// Promises that the shape will be fully transparent when drawn.
#define PAX_PROMISE_INVISIBLE	0x02
// Promises that the shape will be drawn as a cutout (either fully opaque or fully transparent for a given pixel).
#define PAX_PROMISE_CUTOUT		0x03
// Promises that the shader does not need the UVs.
#define PAX_PROMISE_IGNORE_UVS	0x04
// Promises that the shader ignores the existing color.
#define PAX_PROMISE_IGNORE_BASE	0x08

struct matrix_stack_2d;

struct pax_buf;
struct pax_shader;

struct pax_bmpv;
struct pax_font;
struct pax_font_range;

struct pax_task;

typedef struct pax_buf         pax_buf_t;
typedef struct pax_shader      pax_shader_t;
typedef struct pax_task        pax_task_t;
typedef struct pax_shader_ctx  pax_shader_ctx_t;
typedef struct pax_bmpv        pax_bmpv_t;
typedef struct pax_font        pax_font_t;
typedef struct pax_font_range  pax_font_range_t;

typedef int32_t                pax_err_t;
typedef uint32_t               pax_col_t;

// Helper for color conversion.
// Used for both buffer type to ARGB and vice versa.
// Buffer argument is mostly used for images with palette.
typedef pax_col_t (*pax_col_conv_t)(const pax_buf_t *buf, pax_col_t color);
// Helper for setting pixels in drawing routines.
// Used to allow optimising away alpha in colors.
typedef void (*pax_setter_t)(pax_buf_t *buf, pax_col_t color, int x, int y);
// Helper for getting pixels in drawing routines.
// Used to allow optimising away inline branching.
typedef pax_col_t (*pax_index_getter_t)(const pax_buf_t *buf, int index);
// Helper for setting pixels in drawing routines.
// Used to allow optimising away color conversion.
typedef void (*pax_index_setter_t)(pax_buf_t *buf, pax_col_t color, int index);

// Function pointer for shader promises.
// The promise function will provide a bitfield answer to contextual questions where false is the safe option.
typedef uint64_t (*pax_promise_func_t)(pax_buf_t *buf, pax_col_t tint, void *args);
// Function pointer for shader callback.
// Tint is the color parameter to the pax_shade_xxx function.
typedef pax_col_t (*pax_shader_func_v0_t)(pax_col_t tint, int x, int y, float u, float v, void *args);
// Function pointer for shader callback.
// Tint is the color parameter to the pax_shade_xxx function.
// It is assumed that color math is done by the shader based on the "existing" parameter.
typedef pax_col_t (*pax_shader_func_v1_t)(pax_col_t tint, pax_col_t existing, int x, int y, float u, float v, void *args);

// A simple linked list data structure used to store matrices in a stack.
struct matrix_stack_2d {
	matrix_stack_2d_t *parent;
	matrix_2d_t        value;
};

// The main data structure in PAX.
// Stores pixel data and matrix information among other things.
struct pax_buf {
	// Buffer type, color modes, etc.
	pax_buf_type_t    type;
	// Whether to perform free on the buffer on deinit.
	bool              do_free;
	// Whether to perform free on the palette on deinit.
	bool              do_free_pal;
	// Whether to reverse the endianness of the buffer.
	bool              reverse_endianness;
	union {
		// Shorthand for 8bpp buffer.
		uint8_t      *buf_8bpp;
		// Shorthand for 16bpp buffer.
		uint16_t     *buf_16bpp;
		// Shorthand for 32bpp buffer.
		uint32_t     *buf_32bpp;
		// Buffer pointer.
		void         *buf;
	};
	// Bits per pixel.
	int               bpp;
	
	union {
	// Pallette for buffers with a pallette type.
	pax_col_t        *pallette;
	// Pallette for buffers with a pallette type.
	pax_col_t        *palette;
	};
	union {
	// The number of colors in the pallette.
	size_t            pallette_size;
	// The number of colors in the pallette.
	size_t            palette_size;
	};
	
	// Width in pixels.
	int               width;
	// Height    in pixels.
	int               height;
	
	// Dirty x (top left).
	int               dirty_x0;
	// Dirty y (top left).
	int               dirty_y0;
	// Dirty x (bottom right).
	int               dirty_x1;
	// Dirty y (bottom right).
	int               dirty_y1;
	
	// Color to buffer function to use.
	pax_col_conv_t    col2buf;
	// Buffer to color function to use.
	pax_col_conv_t    buf2col;
	
	// Setter to use to write a pixel index.
	pax_index_setter_t setter;
	// Getter to use to read a pixel index.
	pax_index_getter_t getter;
	
	// Clip rectangle.
	// Shapes are only drawn inside the clip rectangle.
	// This excludes PNG decoding functions.
	pax_recti         clip;
	// Matrix stack.
	// The top most entry is used to transform shapes.
	matrix_stack_2d_t stack_2d;
	
	// Rotation increment.
	// 0 is not rotated, each unit is one quarter turn counter-clockwise.
	int rotation;
};

// A shader definition, used by pax_shade_ methods.
struct pax_shader {
	// Version of the shader schema this was made for.
	uint8_t  schema_version;
	// Bitwise inversion of schema version.
	uint8_t  schema_complement;
	// Rendering type of this shader.
	uint8_t  renderer_id;
	// Optional callback which is used to make contextual promises.
	void    *promise_callback;
	// Callback which defines the colors to output.
	void    *callback;
	// Shader arguments.
	void    *callback_args;
	// Whether to promise that an alpha of 0 in tint will return a fully transparent.
	bool     alpha_promise_0;
	// Whether to promise that an alpha of 255 in tint will return a fully opaque.
	bool     alpha_promise_255;
};

// Information relevant to each character of a variable pitch font.
struct pax_bmpv {
	// The position of the drawn portion.
	int8_t draw_x, draw_y;
	// The size of the drawn portion.
	uint8_t draw_w, draw_h;
	// The measured width of the glyph.
	uint8_t measured_width;
	// The index in the glyphs data for this glyph.
	size_t index;
};

// Information relevant for the entirety of each font.
struct pax_font {
	// The searchable name of the font.
	const char             *name;
	// The number of ranges included in the font.
	size_t                  n_ranges;
	// The ranges included in the font.
	const pax_font_range_t *ranges;
	// Default point size.
	uint16_t                default_size;
	// Whether or not it is recommended to use antialiasing.
	// Applies to pax_draw_text, but not it's variants.
	bool                    recommend_aa;
};

// Describes a range of glyphs in a font.
struct pax_font_range {
	// The type of font range.
	pax_font_type_t  type;
	// First character in range.
	uint32_t         start;
	// Last character in range.
	uint32_t         end;
	union {
		// Monospace, bitmapped fonts.
		struct {
			// The raw glyph bytes.
			const uint8_t *glyphs;
			// The width of all glyphs.
			uint8_t        width;
			// The height of all glyphs.
			uint8_t        height;
			// The Bits Per Pixel of all glyphs.
			uint8_t        bpp;
		} bitmap_mono;
		// Variable pitch, bitmapped fonts.
		struct {
			// The raw glyph bytes.
			const uint8_t    *glyphs;
			// Additional dimensions defined per glyph.
			const pax_bmpv_t *dims;
			// The height of all glyphs.
			uint8_t           height;
			// The Bits Per Pixel of all glyphs.
			uint8_t           bpp;
		} bitmap_var;
	};
};

// A task to perform, used by multicore rendering.
// Every task has pre-transformed co-ordinates.
// If you change the shader object's content (AKA the value that args points to),
// You should run pax_join before making the change.
struct pax_task {
	// The buffer to apply this task to.
	pax_buf_t      *buffer;
	// The type of thing to do.
	pax_task_type_t type;
	// Color to use.
	pax_col_t       color;
	// Shader to use.
	pax_shader_t   *shader;
	// UVs to use.
	union {
		// UVs to use for rects and arcs.
		pax_quadf *quad_uvs;
		// UVs to use for triangle.
		pax_trif  *tri_uvs;
	};
	// Additional parameters.
	// This is an array of floats for X, Y, and dimensions of shapes.
	float          *shape;
	// Number of floats in the shape array.
	size_t          shape_len;
};

// Context used at drawing time for shaders.
struct pax_shader_ctx {
	// The callback internally used per pixel.
	pax_shader_func_v1_t callback;
	// The args to throw at the callback.
	void                *callback_args;
	// Whether to skip drawing.
	bool                 skip;
	// Whether to do a get the pixel value for merging.
	bool                 do_getter;
};

// The absolute minimum possible size a valid font can be in memory.
#define PAX_FONT_LOADER_MINUMUM_SIZE (sizeof(pax_font_t) + sizeof(pax_font_range_t) + 3)

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //PAX_TYPES_H
