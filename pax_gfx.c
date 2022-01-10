/*
	MIT License

	Copyright (c) 2022 Julian Scheffers

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

#include "pax_gfx.h"
#include "pax_shaders.h"

#include <stdio.h>
#include <esp_system.h>
#include <sdkconfig.h>
#include <esp_err.h>
#include <esp_log.h>
#include <stdint.h>
#include <malloc.h>
#include <string.h>
#include <math.h>

pax_err_t pax_last_error = PAX_OK;
static const char *TAG   = "pax";

#ifdef PAX_AUTOREPORT
#define PAX_ERROR(where, errno) { ESP_LOGE(TAG, "@ %s: %s", where, pax_desc_err(errno)); pax_last_error = errno; return; }
#define PAX_ERROR1(where, errno, retval) { ESP_LOGE(TAG, "@ %s: %s", where, pax_desc_err(errno)); pax_last_error = errno; return retval; }
#else
#define PAX_ERROR(where, errno) { pax_last_error = errno; return; }
#define PAX_ERROR1(where, errno, retval) { pax_last_error = errno; return retval; }
#endif

#define PAX_SUCCESS() { pax_last_error = PAX_OK; }



/* =========== HELPERS =========== */

// Buffer sanity check.
#define PAX_BUF_CHECK(where) { if (!(buf) || !(buf)->buf) PAX_ERROR(where, PAX_ERR_NOBUF); }
// Buffer sanity check.
#define PAX_BUF_CHECK1(where, retval) { if (!(buf) || !(buf)->buf) PAX_ERROR1(where, PAX_ERR_NOBUF, retval); }

// Swap two variables.
#define PAX_SWAP(type, a, b) { type tmp = a; a = b; b = tmp; }
// Swap two points represented by floats.
#define PAX_SWAP_POINTS(x0, y0, x1, y1) { float tmp = x1; x1 = x0; x0 = tmp; tmp = y1; y1 = y0; y0 = tmp; }
// Sort two points represented by floats.
#define PAX_SORT_POINTS(x0, y0, x1, y1) { if (y1 < y0) PAX_SWAP_POINTS(x0, y0, x1, y1) }

#define PAX_GET_BPP(type)     ((type) & 0xff)
#define PAX_IS_GREY(type)     (((type) & 0xf0000000) == 0x10000000)
#define PAX_IS_PALLETTE(type) (((type) & 0xf0000000) == 0x20000000)

static inline uint32_t pax_col2buf(pax_buf_t *buf, pax_col_t color) {
	uint8_t bpp = buf->bpp;
	if (PAX_IS_GREY(buf->type)) {
		// Greyscale.
		uint16_t grey = ((color >> 16) & 0xff) + ((color >> 8) & 0xff) + (color & 0xff);
		grey /= 3;
		return ((uint8_t) grey >> (8 - bpp));
	} else if (buf->type == PAX_BUF_4_1111ARGB) {
		// 4BPP 1111-ARGB
		// From: Aaaa aaaa Rrrr rrrr Gggg gggg Bbbb bbbb
		// To:                                      ARGB
		uint16_t value = ((color >> 28) & 0x8) | ((color >> 21) & 0x4) | ((color >> 14) & 0x2) | ((color >> 7) & 0x1);
		return value;
	} else if (buf->type == PAX_BUF_8_332RGB) {
		// 8BPP 332-RGB
		// From: Aaaa aaaa Rrrr rrrr Gggg gggg Bbbb bbbb
		// To:                                 RrrG ggBb
		uint16_t value = ((color >> 16) & 0xe0) | ((color >> 11) & 0x1c) | ((color >> 6) & 0x03);
		return value;
	} else if (buf->type == PAX_BUF_8_2222ARGB) {
		// 8BPP 2222-ARGB
		// From: Aaaa aaaa Rrrr rrrr Gggg gggg Bbbb bbbb
		// To:                                 AaRr GgBb
		uint16_t value = ((color >> 24) & 0xc0) | ((color >> 18) & 0x30) | ((color >> 12) & 0x0c) | ((color >> 6) & 0x03);
		return value;
	} else if (buf->type == PAX_BUF_16_4444ARGB) {
		// 16BPP 4444-ARGB
		// From: Aaaa aaaa Rrrr rrrr Gggg gggg Bbbb bbbb
		// To:                       Aaaa Rrrr Gggg Bbbb
		uint16_t value = ((color >> 16) & 0xf000) | ((color >> 12) & 0x0f00) | ((color >> 8) & 0x00f0) | ((color >> 4) & 0x000f);
		return (value >> 8) | ((value << 8) & 0xff00);
	} else if (buf->type == PAX_BUF_16_565RGB) {
		// 16BPP 565-RGB
		// From: Aaaa aaaa Rrrr rrrr Gggg gggg Bbbb bbbb
		// To:                       Rrrr rGgg gggB bbbb
		uint16_t value = ((color >> 8) & 0xf800) | ((color >> 5) & 0x07e0) | ((color >> 3) & 0x001f);
		return (value >> 8) | ((value << 8) & 0xff00);
	} else if (buf->type == PAX_BUF_32_8888ARGB) {
		return color;
	}
	PAX_ERROR1("pax_col2buf", PAX_ERR_PARAM, 0);
}

static inline uint32_t pax_buf2col(pax_buf_t *buf, uint32_t value) {
	uint8_t bpp = buf->bpp;
	if (PAX_IS_GREY(buf->type)) {
		// Greyscale.
		//uint8_t grey_bits = (buf->type >> 8) & 0xf;
		uint8_t grey = value << (8 - bpp);
		if      (bpp == 7) grey |= grey >> 7;
		else if (bpp == 6) grey |= grey >> 6;
		else if (bpp == 5) grey |= grey >> 5;
		else if (bpp == 4) grey |= grey >> 4;
		else if (bpp == 3) grey = (value * 0x49) >> 1;
		else if (bpp == 2) grey =  value * 0x55;
		else if (bpp == 1) grey = -value;
		return 0xff000000 | (grey << 16) | (grey << 8) | grey;
	} else if (buf->type == PAX_BUF_4_1111ARGB) {
		// 4BPP 1111-ARGB
		// From:                                    ARGB
		// To:   Aaaa aaaa Rrrr rrrr Gggg gggg Bbbb bbbb
		pax_col_t color = ((value << 28) & 0x80000000) | ((value << 21) & 0x00800000) | ((value << 14) & 0x00008000) | ((value << 7) & 0x00000080);
		color |= color >> 1;
		color |= color >> 2;
		color |= color >> 4;
		return color;
	} else if (buf->type == PAX_BUF_8_332RGB) {
		// 8BPP 332-RGB
		// From:                               RrrG ggBb
		// To:   Aaaa aaaa Rrrr rrrr Gggg gggg Bbbb bbbb
		return 0;
	} else if (buf->type == PAX_BUF_8_2222ARGB) {
		// 8BPP 2222-ARGB
		// From:                               AaRr GgBb
		// To:   Aaaa aaaa Rrrr rrrr Gggg gggg Bbbb bbbb
		pax_col_t color = ((value << 24) & 0xc0000000) | ((value << 18) & 0x00c00000) | ((value << 12) & 0x0000c000) | ((value << 6) & 0x000000c0);
		color |= color >> 2;
		color |= color >> 4;
		return color;
	} else if (buf->type == PAX_BUF_16_4444ARGB) {
		// 16BPP 4444-ARGB
		// From:                     Aaaa Rrrr Gggg Bbbb
		// To:   Aaaa aaaa Rrrr rrrr Gggg gggg Bbbb bbbb
		// Add:       Aaaa      Rrrr      Gggg      Bbbb
		pax_col_t color = ((value << 16) & 0xf0000000) | ((value << 12) & 0x00f00000) | ((value << 8) & 0x0000f000) | ((value << 4) & 0x000000f0);
		color |= color >> 4;
		return color;
	} else if (buf->type == PAX_BUF_16_565RGB) {
		value = ((value << 8) & 0xff00) | ((value >> 8) & 0x00ff);
		// 16BPP 565-RGB
		// From:                     Rrrr rGgg gggB bbbb
		// To:   .... .... Rrrr r... Gggg gg.. Bbbb b...
		// Add:  .... .... .... .Rrr .... ..Gg .... .Bbb
		// Take the existing information.
		pax_col_t color = ((value << 8) & 0x00f80000) | ((value << 5) & 0x0000fc00) | ((value << 3) & 0x000000f8);
		// Now, fill in some missing bits.
		color |= ((value << 3) & 0x00070000) | ((value >> 1) & 0x00000300) | ((value >> 2) & 0x00000007);
		return color | 0xff000000;
	} else if (buf->type == PAX_BUF_32_8888ARGB) {
		return value;
	}
	PAX_ERROR1("pax_buf2col", PAX_ERR_PARAM, 0);
}

// Set a pixel, unsafe (don't check bounds or buffer, no color conversion).
static inline void pax_set_pixel_u(pax_buf_t *buf, uint32_t color, int x, int y) {
	uint8_t bpp = buf->bpp;
	if (bpp == 1) {
		// 1BPP
		// uint8_t *ptr = &buf->buf_8bpp[(x + y * buf->width) >> 3];
		// uint8_t mask = 0x01 << (x & 7);
		// *ptr = (*ptr & ~mask) | (color << (x & 7) * 2);
	} else if (bpp == 2) {
		// 2BPP
		// uint8_t *ptr = &buf->buf_8bpp[(x + y * buf->width) >> 2];
		// uint8_t mask = 0x03 << (x & 3) * 2;
		// *ptr = (*ptr & ~mask) | (color << (x & 3) * 2);
	} else if (bpp == 4) {
		// 4BPP
		// uint8_t *ptr = &buf->buf_8bpp[(x + y * buf->width) >> 1];
		// uint8_t mask = (x & 1) ? 0xf0 : 0x0f;
		// *ptr = (*ptr & ~mask) | (color << (x & 1) * 4);
	} else if (bpp == 8) {
		// 8BPP
		buf->buf_8bpp[x + y * buf->width] = color;
	} else if (bpp == 16) {
		// 16BPP
		buf->buf_16bpp[x + y * buf->width] = color;
	} else if (bpp == 32) {
		// 32BPP
		buf->buf_32bpp[x + y * buf->width] = color;
	} else {
		PAX_ERROR("pax_set_pixel_u", PAX_ERR_PARAM);
	}
}

// Get a pixel, unsafe (don't check bounds or buffer, no color conversion).
static inline uint32_t pax_get_pixel_u(pax_buf_t *buf, int x, int y) {
	uint8_t bpp = buf->bpp;
	if (bpp == 8) {
		return buf->buf_16bpp[x + y * buf->width];
	} else if (bpp == 16) {
		return buf->buf_16bpp[x + y * buf->width];
	} else if (bpp == 32) {
		return buf->buf_32bpp[x + y * buf->width];
	} else {
		//PAX_ERROR1("pax_get_pixel_u", PAX_ERR_PARAM, 0);
		return 0;
	}
}



/* ======= DRAWING HELPERS ======= */

// Internal method for unshaded triangles.
// Assumes points are sorted by Y.
static inline void pax_tri_unshaded(pax_buf_t *buf, pax_col_t color,
		float x0, float y0, float x1, float y1, float x2, float y2) {
	
	// Find the appropriate Y for y0, y1 and y2 inside the triangle.
	float y_post_0 = (int) (y0 + 0.5) + 0.5;
	float y_post_1 = (int) (y1 + 0.5) + 0.5;
	float y_pre_2  = (int) (y2 - 0.5) + 0.5;
	
	// And the coefficients for x0->x1, x1->x2 and x0->x2.
	float x0_x1_dx = (x1 - x0) / (y1 - y0);
	float x1_x2_dx = (x2 - x1) / (y2 - y1);
	float x0_x2_dx = (x2 - x0) / (y2 - y0);
	
	// Clip: Y axis.
	if (y_post_0 > buf->clip.y + buf->clip.h) {
		y_post_0 = (int) (buf->clip.y + buf->clip.h - 0.5) + 0.5;
	}
	if (y_post_1 > buf->clip.y + buf->clip.h) {
		y_post_1 = (int) (buf->clip.y + buf->clip.h - 0.5) + 0.5;
	}
	if (y_pre_2  > buf->clip.y + buf->clip.h) {
		y_pre_2  = (int) (buf->clip.y + buf->clip.h - 0.5) + 0.5;
	}
	
	if (y_pre_2  < buf->clip.y) {
		y_pre_2  = (int) (buf->clip.y + 0.5) + 0.5;
	}
	if (y_post_1 < buf->clip.y) {
		y_post_1 = (int) (buf->clip.y + 0.5) + 0.5;
	}
	if (y_post_0 < buf->clip.y) {
		y_post_0 = (int) (buf->clip.y + 0.5) + 0.5;
	}
	
	// Draw top half.
	// This condition is false if no one point is inside the triangle and above y1.
	if (y_post_0 < y_post_1 && y_post_0 >= y0) {
		// Find the X counterparts to the other points we found.
		float x_a = x0 + x0_x1_dx * (y_post_0 - y0);
		float x_b = x0 + x0_x2_dx * (y_post_0 - y0);
		for (int y = y_post_0; y < (int) y_post_1; y++) {
			// Plot the horizontal line.
			float x_left, x_right;
			if (x_a < x_b) {
				x_left  = x_a;
				x_right = x_b;
			} else {
				x_left  = x_b;
				x_right = x_a;
			}
			// Clip: X axis.
			if (x_right > buf->clip.x + buf->clip.w) {
				x_right = buf->clip.x + buf->clip.w;
			}
			if (x_left < buf->clip.x) {
				x_left = buf->clip.x;
			}
			for (int x = x_left + 0.5; x < x_right; x ++) {
				// And simply merge colors accordingly.
				pax_merge_pixel(buf, color, x, y);
			}
			// Move X.
			x_a += x0_x1_dx;
			x_b += x0_x2_dx;
		}
	}
	// Draw bottom half.
	// This condition might be confusing, but it's false if no point at all is inside the triangle.
	if (y_post_0 <= y_pre_2 && y_post_1 >= y1 && y_pre_2 <= y2) {
		// Find the X counterparts to the other points we found.
		float x_a = x1 + x1_x2_dx * (y_post_1 - y1);
		float x_b = x0 + x0_x2_dx * (y_post_1 - y0);
		for (int y = y_post_1; y <= (int) y_pre_2; y++) {
			// Plot the horizontal line.
			float x_left, x_right;
			if (x_a < x_b) {
				x_left  = x_a;
				x_right = x_b;
			} else {
				x_left  = x_b;
				x_right = x_a;
			}
			// Clip: X axis.
			if (x_right > buf->clip.x + buf->clip.w) {
				x_right = buf->clip.x + buf->clip.w;
			}
			if (x_left < buf->clip.x) {
				x_left = buf->clip.x;
			}
			for (int x = x_left + 0.5; x < x_right; x ++) {
				// And simply merge colors accordingly.
				pax_merge_pixel(buf, color, x, y);
			}
			// Move X.
			x_a += x1_x2_dx;
			x_b += x0_x2_dx;
		}
	}
}

// Internal method for shaded triangles.
// Assumes points are sorted by Y.
static inline void pax_tri_shaded(pax_buf_t *buf, pax_col_t color, pax_shader_t *shader,
		float x0, float y0, float x1, float y1, float x2, float y2,
		float u0, float v0, float u1, float v1, float u2, float v2) {
	
	// Find the appropriate Y for y0, y1 and y2 inside the triangle.
	float y_post_0 = (int) (y0 + 0.5) + 0.5;
	float y_post_1 = (int) (y1 + 0.5) + 0.5;
	float y_pre_2  = (int) (y2 - 0.5) + 0.5;
	
	// And the coefficients for x0->x1, x1->x2 and x0->x2.
	float x0_x1_dx = (x1 - x0) / (y1 - y0);
	float x1_x2_dx = (x2 - x1) / (y2 - y1);
	float x0_x2_dx = (x2 - x0) / (y2 - y0);
	
	// And UVs.
	float u0_u1_du = (u1 - u0) / (y1 - y0);
	float v0_v1_dv = (v1 - v0) / (y1 - y0);
	float u0_u2_du = (u2 - u0) / (y2 - y0);
	float v0_v2_dv = (v2 - v0) / (y2 - y0);
	float u1_u2_du = (u2 - u1) / (y2 - y1);
	float v1_v2_dv = (v2 - v1) / (y2 - y1);
	
	// Clip: Y axis.
	if (y_post_0 > buf->clip.y + buf->clip.h) {
		y_post_0 = (int) (buf->clip.y + buf->clip.h - 0.5) + 0.5;
	}
	if (y_post_1 > buf->clip.y + buf->clip.h) {
		y_post_1 = (int) (buf->clip.y + buf->clip.h - 0.5) + 0.5;
	}
	if (y_pre_2  > buf->clip.y + buf->clip.h) {
		y_pre_2  = (int) (buf->clip.y + buf->clip.h - 0.5) + 0.5;
	}
	
	if (y_pre_2  < buf->clip.y) {
		y_pre_2  = (int) (buf->clip.y + 0.5) + 0.5;
	}
	if (y_post_1 < buf->clip.y) {
		y_post_1 = (int) (buf->clip.y + 0.5) + 0.5;
	}
	if (y_post_0 < buf->clip.y) {
		y_post_0 = (int) (buf->clip.y + 0.5) + 0.5;
	}
	
	// Draw top half.
	// This condition is false if no one point is inside the triangle and above y1.
	if (y_post_0 < y_post_1 && y_post_0 >= y0) {
		float coeff = (y_post_0 - y0);
		// Find the X counterparts to the other points we found.
		float x_a = x0 + x0_x1_dx * coeff;
		float x_b = x0 + x0_x2_dx * coeff;
		// And UV ranges.
		float u_a = u0 + u0_u1_du * coeff;
		float v_a = v0 + v0_v1_dv * coeff;
		float u_b = u0 + u0_u2_du * coeff;
		float v_b = v0 + v0_v2_dv * coeff;
		for (int y = y_post_0; y < (int) y_post_1; y++) {
			// Plot the horizontal line.
			float x_left, x_right;
			float u_left, u_right;
			float v_left, v_right;
			if (x_a < x_b) {
				x_left  = x_a;
				x_right = x_b;
				u_left  = u_a;
				u_right = u_b;
				v_left  = v_a;
				v_right = v_b;
			} else {
				x_left  = x_b;
				x_right = x_a;
				u_left  = u_b;
				u_right = u_a;
				v_left  = v_b;
				v_right = v_a;
			}
			// Clip: X axis.
			// if (x_right > buf->clip.x + buf->clip.w) {
			// 	float new_x_right = buf->clip.x + buf->clip.w;
			// 	float delta = (new_x_right - x_right) / (x_left - x_right);
			// 	x_right = new_x_right;
			// 	u_right = (u_left - u_right) * delta;
			// 	v_right = (v_left - v_right) * delta;
			// }
			// if (x_left < buf->clip.x) {
			// 	float new_x_left = buf->clip.x;
			// 	float delta = (new_x_left - x_right) / (x_left - x_right);
			// 	x_left = new_x_left;
			// 	u_left = (u_left - u_right) * delta;
			// 	v_left = (v_left - v_right) * delta;
			// }
			// Find UV ranges.
			int x = x_left + 0.5;
			int nIter = x_right - x;
			// Fix UVs.
			float u = u_left, v = v_left;
			float du = (u_right - u_left) / nIter;
			float dv = (v_right - v_left) / nIter;
			for (; x < x_right; x ++) {
				// Apply the shader,
				pax_col_t result = (shader->callback)(color, x, y, u, v, shader->callback_args);
				// And simply merge colors accordingly.
				pax_merge_pixel(buf, result, x, y);
				u += du;
				v += dv;
			}
			// Move X.
			x_a += x0_x1_dx;
			x_b += x0_x2_dx;
			u_a += u0_u1_du;
			v_a += v0_v1_dv;
			u_b += u0_u2_du;
			v_b += v0_v2_dv;
		}
	}
	// Draw bottom half.
	// This condition might be confusing, but it's false if no point at all is inside the triangle.
	if (y_post_0 <= y_pre_2 && y_post_1 >= y1 && y_pre_2 <= y2) {
		float coeff0 = (y_post_1 - y0);
		float coeff1 = (y_post_1 - y1);
		// Find the X counterparts to the other points we found.
		float x_a = x1 + x1_x2_dx * coeff1;
		float x_b = x0 + x0_x2_dx * coeff0;
		// And UV ranges.
		float u_a = u1 + u1_u2_du * coeff1;
		float v_a = v1 + v1_v2_dv * coeff1;
		float u_b = u0 + u0_u2_du * coeff0;
		float v_b = v0 + v0_v2_dv * coeff0;
		for (int y = y_post_1; y <= (int) y_pre_2; y++) {
			// Plot the horizontal line.
			float x_left, x_right;
			float u_left, u_right;
			float v_left, v_right;
			if (x_a < x_b) {
				x_left  = x_a;
				x_right = x_b;
				u_left  = u_a;
				u_right = u_b;
				v_left  = v_a;
				v_right = v_b;
			} else {
				x_left  = x_b;
				x_right = x_a;
				u_left  = u_b;
				u_right = u_a;
				v_left  = v_b;
				v_right = v_a;
			}
			// Clip: X axis.
			// if (x_right > buf->clip.x + buf->clip.w) {
			// 	x_right = buf->clip.x + buf->clip.w;
			// }
			// if (x_left < buf->clip.x) {
			// 	x_left = buf->clip.x;
			// }
			// Find UV ranges.
			int x = x_left + 0.5;
			int nIter = x_right - x;
			// Fix UVs.
			float u = u_left, v = v_left;
			float du = (u_right - u_left) / nIter;
			float dv = (v_right - v_left) / nIter;
			for (; x < x_right; x ++) {
				// Apply the shader,
				pax_col_t result = (shader->callback)(color, x, y, u, v, shader->callback_args);
				// And simply merge colors accordingly.
				pax_merge_pixel(buf, result, x, y);
				u += du;
				v += dv;
			}
			// Move X.
			x_a += x1_x2_dx;
			x_b += x0_x2_dx;
			u_a += u1_u2_du;
			v_a += v1_v2_dv;
			u_b += u0_u2_du;
			v_b += v0_v2_dv;
		}
	}
}


// Internal method for shaded rects.
static inline void pax_rect_shaded(pax_buf_t *buf, pax_col_t color, pax_shader_t *shader,
		float x, float y, float width, float height,
		float u0, float v0, float u1, float v1, float u2, float v2, float u3, float v3) {
	
	// Fix width and height.
	if (width < 0) {
		x += width;
		width = -width;
		PAX_SWAP_POINTS(u0, v0, u1, v1);
		PAX_SWAP_POINTS(u2, v2, u3, v3);
	}
	if (height < 0) {
		y += height;
		height = -height;
		PAX_SWAP_POINTS(u0, v0, u3, v3);
		PAX_SWAP_POINTS(u1, v1, u2, v2);
	}
	
	// Find UV deltas.
	float u0_u3_du = (u3 - u0) / height;
	float v0_v3_dv = (v3 - v0) / height;
	float u1_u2_du = (u2 - u1) / height;
	float v1_v2_dv = (v2 - v1) / height;
	
	float u_a = u0, v_a = v0;
	float u_b = u1, v_b = v1;
	
	// Pixel time.
	for (int _y = y + 0.5; _y < y + height + 0.5; _y ++) {
		float ua_ub_du = (u_b - u_a) / width;
		float va_vb_dv = (v_b - v_a) / width;
		float u = u_a, v = v_a;
		for (int _x = x + 0.5; _x < x + width + 0.5; _x ++) {
			pax_col_t result = (shader->callback)(color, _x, _y, u, v, shader->callback_args);
			pax_merge_pixel(buf, result, _x, _y);
			u += ua_ub_du;
			v += va_vb_dv;
		}
		u_a += u0_u3_du;
		v_a += v0_v3_dv;
		u_b += u1_u2_du;
		v_b += v1_v2_dv;
	}
	
}


/* ============ DEBUG ============ */

// Describe error.
char *pax_desc_err(pax_err_t error) {
	char *unknown = "Unknown error";
	char *desc[] = {
		"Success",
		"No framebuffer",
		"No memory",
		"Invalid parameters",
		"Infinite parameters",
		"Out of bounds",
		"Matrix stack underflow"
	};
	if (error > 0 || error < -6) return unknown;
	else return desc[-error];
}

// Debug stuff.
void pax_debug(pax_buf_t *buf) {
	ESP_LOGW(TAG, "Performing buffer dump in format %08x", buf->type);
}


/* ============ BUFFER =========== */

// Create a new buffer.
// If mem is NULL, a new area is allocated.
void pax_buf_init(pax_buf_t *buf, void *mem, int width, int height, pax_buf_type_t type) {
	bool use_alloc = !mem;
	if (use_alloc) {
		// Allocate the right amount of bytes.
		ESP_LOGI(TAG, "Allocating new memory for buffer.");
		mem = malloc((PAX_GET_BPP(type) * width * height + 7) >> 3);
		if (!mem) PAX_ERROR("pax_buf_init", PAX_ERR_NOMEM);
	}
	*buf = (pax_buf_t) {
		.type       = type,
		.buf        = mem,
		.width      = width,
		.height     = height,
		.bpp        = PAX_GET_BPP(type),
		.stack_2d   = {
			.parent = NULL,
			.value  = matrix_2d_identity()
		},
		.do_free    = use_alloc
	};
	pax_mark_clean(buf);
	pax_noclip(buf);
	PAX_SUCCESS();
}

// Destroy the buffer, freeing its memory.
void pax_buf_destroy(pax_buf_t *buf) {
	PAX_BUF_CHECK("pax_buf_destroy");
	
	matrix_stack_2d_t *current = buf->stack_2d.parent;
	while (current) {
		matrix_stack_2d_t *next = current->parent;
		free(current);
		current = next;
	}
	if (buf->do_free) {
		free(buf->buf);
	}
	
	PAX_SUCCESS();
}

// Convert the buffer to the given new format.
// If dest is NULL or equal to src, src will be converted.
void pax_buf_convert(pax_buf_t *dst, pax_buf_t *src, pax_buf_type_t type) {
	if (!(src) || !(src)->buf) PAX_ERROR("pax_buf_convert (src)", PAX_ERR_NOBUF);
	if (!(dst) || !(dst)->buf) PAX_ERROR("pax_buf_convert (dst)", PAX_ERR_NOBUF);
	
	//if (!dst) dst = src;
	// We can't go using realloc on an unknown buffer.
	if (!dst->do_free) PAX_ERROR("pax_buf_convert", PAX_ERR_PARAM);
	// Src and dst must match in size.
	if (src->width != dst->width || src->height != dst->height) PAX_ERROR("pax_buf_convert", PAX_ERR_BOUNDS);
	
	dst->bpp = PAX_GET_BPP(type);
	dst->type = type;
	size_t new_pixels = dst->width * dst->height;
	size_t new_size = (new_pixels * dst->bpp + 7) / 8;
	if (dst->bpp > src->bpp) {
		ESP_LOGI(TAG, "Expanding buffer.");
		// Resize the memory for DST beforehand.
		dst->buf = realloc(dst->buf, new_size);
		if (!dst->buf) PAX_ERROR("pax_buf_convert", PAX_ERR_NOMEM);
		// Reverse iterate if the new BPP is larger than the old BPP.
		for (int y = dst->height - 1; y >= 0; y --) {
			for (int x = dst->width - 1; x >= 0; x --) {
				pax_col_t col_src = pax_get_pixel(src, x, y);
				pax_set_pixel(dst, col_src, x, y);
			}
		}
	} else {
		ESP_LOGI(TAG, "Shrinking buffer.");
		// Otherwise, iterate normally.
		for (int y = 0; y < dst->height; y ++) {
			for (int x = 0; x < dst->width; x ++) {
				pax_col_t col_src = pax_get_pixel(src, x, y);
				pax_set_pixel(dst, col_src, x, y);
			}
		}
		// Resize the memory for DST afterwards.
		dst->buf = realloc(dst->buf, new_size);
		if (!dst->buf) PAX_ERROR("pax_buf_convert", PAX_ERR_NOMEM);
	}
}

// Clip the buffer to the desired rectangle.
void pax_clip(pax_buf_t *buf, float x, float y, float width, float height) {
	// Make width and height positive.
	if (width < 0) {
		x += width;
		width = -width;
	}
	if (height < 0) {
		y += height;
		height = -height;
	}
	// Clip the entire rectangle to be at most the buffer's size.
	if (x < 0) {
		width += x;
		x = 0;
	}
	if (y < 0) {
		height += y;
		y = 0;
	}
	if (x + width > buf->width) {
		width = buf->width - x;
	}
	if (y + height > buf->height) {
		height = buf->height - y;
	}
	// Apply the clip.
	buf->clip = (pax_rect_t) {
		.x = x,
		.y = y,
		.w = width,
		.h = height
	};
}

// Clip the buffer to it's full size.
void pax_noclip(pax_buf_t *buf) {
	buf->clip = (pax_rect_t) {
		.x = 0,
		.y = 0,
		.w = buf->width,
		.h = buf->height
	};
}

// Check whether the buffer is dirty.
bool pax_is_dirty(pax_buf_t *buf) {
	PAX_BUF_CHECK1("pax_is_dirty", 0);
	return buf->dirty_x0 < buf->dirty_x1;
}

// Mark the entire buffer as clean.
void pax_mark_clean(pax_buf_t *buf) {
	PAX_BUF_CHECK("pax_mark_clean");
	buf->dirty_x0 = buf->width - 1;
	buf->dirty_y0 = buf->height - 1;
	buf->dirty_x1 = 0;
	buf->dirty_y1 = 0;
	PAX_SUCCESS();
}

// Mark the entire buffer as dirty.
void pax_mark_dirty0(pax_buf_t *buf) {
	PAX_BUF_CHECK("pax_mark_dirty0");
	buf->dirty_x0 = 0;
	buf->dirty_y0 = 0;
	buf->dirty_x1 = buf->width;
	buf->dirty_y1 = buf->height;
	PAX_SUCCESS();
}

// Mark a single point as dirty.
void pax_mark_dirty1(pax_buf_t *buf, int x, int y) {
	PAX_BUF_CHECK("pax_mark_dirty1");
	
	if (x < buf->dirty_x0) buf->dirty_x0 = x;
	if (x > buf->dirty_x1) buf->dirty_x1 = x;
	if (y < buf->dirty_x0) buf->dirty_y0 = y;
	if (y > buf->dirty_x1) buf->dirty_y1 = y;
	
	PAX_SUCCESS();
}

// Mark a rectangle as dirty.
void pax_mark_dirty2(pax_buf_t *buf, int x, int y, int width, int height) {
	PAX_BUF_CHECK("pax_mark_dirty2");
	
	if (x              < buf->dirty_x0) buf->dirty_x0 = x;
	if (x + width  - 1 > buf->dirty_x1) buf->dirty_x1 = x + width  - 1;
	if (y              < buf->dirty_x0) buf->dirty_y0 = y;
	if (y + height - 1 > buf->dirty_x1) buf->dirty_y1 = y + height - 1;
	
	PAX_SUCCESS();
}



/* ============ COLORS =========== */

// A linear interpolation based only in ints.
static inline uint8_t pax_lerp(uint8_t part, uint8_t from, uint8_t to) {
	return from + (( (to - from) * (part + (part >> 7)) ) >> 8);
}

// Converts HSV to ARGB.
pax_col_t pax_col_hsv(uint8_t _h, uint8_t s, uint8_t v) {
	uint16_t h     = _h * 6;
	uint16_t phase = h >> 8;
	// Parts of HSV.
	uint8_t up, down, other;
	other  = ~s;
	if (h & 0x100) {
		// Down goes away.
		up     = 0xff;
		down   = pax_lerp(s, 0xff, ~h & 0xff);
	} else {
		// Up comes in.
		up     = pax_lerp(s, 0xff,  h & 0xff);
		down   = 0xff;
	}
	// Apply brightness.
	up    = pax_lerp(v, 0, up);
	down  = pax_lerp(v, 0, down);
	other = pax_lerp(v, 0, other);
	// Apply to RGB.
	uint8_t r, g, b;
	switch (phase >> 1) {
		case 0:
			// From R to G.
			r = down; g = up; b = other;
			break;
		case 1:
			// From G to B.
			r = other; g = down; b = up;
			break;
		case 2:
			// From B to R.
			r = up; g = other; b = down;
			break;
		default:
			// Shut up, compiler.
			return 0;
	}
	// Merge.
	return 0xff000000 | (r << 16) | (g << 8) | b;
}

// Linearly interpolates between from and to, including alpha.
pax_col_t pax_col_lerp(uint8_t part, pax_col_t from, pax_col_t to) {
	return (pax_lerp(part, from >> 24, to >> 24) << 24)
		 | (pax_lerp(part, from >> 16, to >> 16) << 16)
		 | (pax_lerp(part, from >>  8, to >>  8) <<  8)
		 |  pax_lerp(part, from,       to);
}

// Merges the two colors, based on alpha.
pax_col_t pax_col_merge(pax_col_t base, pax_col_t top) {
	if (!(top >> 24)) return base;
	if ((top >> 24) == 255) return top;
	uint8_t part = top >> 24;
	return (pax_lerp(part, base >> 24, 255)       << 24)
		 | (pax_lerp(part, base >> 16, top >> 16) << 16)
		 | (pax_lerp(part, base >>  8, top >>  8) <<  8)
		 |  pax_lerp(part, base,       top);
}



/* ============ MATRIX =========== */

// 2D rotation matrix: represents a 2D shearing.
matrix_2d_t matrix_2d_rotate(float angle) {
	float _cos = cosf(-angle);
	float _sin = sinf(-angle);
	// return (matrix_2d_t) { .arr = {
	// 	_cos, -_sin, 0,
	// 	_sin,  _cos, 0
	// }};
	return (matrix_2d_t) { .arr = {
		_cos, -_sin, 0,
		_sin,  _cos, 0
	}};
}

// 2D matrix: applies the transformation that b represents on to a.
matrix_2d_t matrix_2d_multiply(matrix_2d_t a, matrix_2d_t b) {
	// [a b c] [p q r] [ap+bs aq+bt ar+bu+c]
	// [d e f]*[s t u]=[dp+es dq+et dr+eu+f]
	// [0 0 1] [0 0 1] [0     0     1      ]
	return (matrix_2d_t) { .arr = {
		a.a0*b.a0 + a.a1*b.b0,   a.a0*b.a1 + a.a1*b.b1,  a.a0*b.a2 + a.a1*b.b2 + a.a2,
		a.b0*b.a0 + a.b1*b.b0,   a.b0*b.a1 + a.b1*b.b1,  a.b0*b.a2 + a.b1*b.b2 + a.b2
	}};
}

// 2D matrix: applies the transformation that a represents on to a point.
void matrix_2d_transform(matrix_2d_t a, float *x, float *y) {
	// [a b c] [x]  [a]  [b] [c] [ax+by+c]
	// [d e f]*[y]=x[d]+y[e]+[f]=[dx+ey+f]
	// [0 0 1] [1]  [0]  [0] [1] [1      ]
	float _x = *x, _y = *y;
	*x = a.a0*_x + a.a1*_y + a.a2;
	*y = a.b0*_x + a.b1*_y + a.b2;
}

// Apply the given matrix to the stack.
void pax_apply_2d(pax_buf_t *buf, matrix_2d_t a) {
	PAX_BUF_CHECK("pax_apply_2d");
	buf->stack_2d.value = matrix_2d_multiply(buf->stack_2d.value, a);
	PAX_SUCCESS();
}

// Push the current matrix up the stack.
void pax_push_2d(pax_buf_t *buf) {
	PAX_BUF_CHECK("pax_push_2d");
	matrix_stack_2d_t *parent = malloc(sizeof(matrix_stack_2d_t));
	if (!parent) PAX_ERROR("pax_push_2d", PAX_ERR_NOMEM);
	*parent = buf->stack_2d;
	buf->stack_2d.parent = parent;
	PAX_SUCCESS();
}

// Pop the top matrix off the stack.
void pax_pop_2d(pax_buf_t *buf) {
	PAX_BUF_CHECK("pax_pop_2d");
	matrix_stack_2d_t *parent = buf->stack_2d.parent;
	if (!parent) PAX_ERROR("pax_pop_2d", PAX_ERR_UNDERFLOW);
	buf->stack_2d = *parent;
	free(parent);
	PAX_SUCCESS();
}



/* ======== DRAWING: PIXEL ======= */

// Set a pixel, merging with alpha.
void pax_merge_pixel(pax_buf_t *buf, pax_col_t color, int x, int y) {
	PAX_BUF_CHECK("pax_merge_pixel");
	if (x < 0 || x >= buf->width || y < 0 || y >= buf->height) {
		// This won't do.
		pax_last_error = PAX_ERR_BOUNDS;
		return;
	}
	PAX_SUCCESS();
	pax_col_t base = pax_buf2col(buf, pax_get_pixel_u(buf, x, y));
	pax_set_pixel_u(buf, pax_col2buf(buf, pax_col_merge(base, color)), x, y);
}

// Set a pixel.
void pax_set_pixel(pax_buf_t *buf, pax_col_t color, int x, int y) {
	PAX_BUF_CHECK("pax_set_pixel");
	if (x < 0 || x >= buf->width || y < 0 || y >= buf->height) {
		// This won't do.
		pax_last_error = PAX_ERR_BOUNDS;
		return;
	}
	PAX_SUCCESS();
	pax_set_pixel_u(buf, pax_col2buf(buf, color), x, y);
}

// Get a pixel.
pax_col_t pax_get_pixel(pax_buf_t *buf, int x, int y) {
	PAX_BUF_CHECK1("pax_get_pixel", 0);
	if (x < 0 || x >= buf->width || y < 0 || y >= buf->height) {
		// This won't do.
		pax_last_error = PAX_ERR_BOUNDS;
		return 0;
	}
	PAX_SUCCESS();
	return pax_buf2col(buf, pax_get_pixel_u(buf, x, y));
}



/* ========= DRAWING: 2D ========= */

// Draw a rectangle with a shader.
// If uvs is NULL, a default will be used (0,0; 1,0; 1,1; 0,1).
void pax_shade_rect(pax_buf_t *buf, pax_col_t color, pax_shader_t *shader,
		pax_quad_t *uvs, float x, float y, float width, float height) {
	if (!uvs) {
		uvs = &(pax_quad_t) {
			.x0 = 0, .y0 = 0,
			.x1 = 1, .y1 = 0,
			.x2 = 1, .y2 = 1,
			.x3 = 0, .y3 = 1
		};
	}
	
	pax_tri_t uv0 = {
		.x0 = uvs->x0, .y0 = uvs->y0,
		.x1 = uvs->x1, .y1 = uvs->y1,
		.x2 = uvs->x2, .y2 = uvs->y2
	};
	pax_tri_t uv1 = {
		.x0 = uvs->x0, .y0 = uvs->y0,
		.x1 = uvs->x3, .y1 = uvs->y3,
		.x2 = uvs->x2, .y2 = uvs->y2
	};
	
	if (matrix_2d_is_identity2(buf->stack_2d.value)) {
		// Simplify this.
		matrix_2d_transform(buf->stack_2d.value, &x, &y);
		width  *= buf->stack_2d.value.a0;
		height *= buf->stack_2d.value.b1;
		pax_rect_shaded(
			buf, color, shader, x, y, width, height,
			uvs->x0, uvs->y0, uvs->x1, uvs->y1,
			uvs->x2, uvs->y2, uvs->x3, uvs->y3
		);
	} else {
		// We still ned triangles.
		pax_shade_tri(buf, color, shader, &uv0, x, y, x + width, y, x + width, y + height);
		pax_shade_tri(buf, color, shader, &uv1, x, y, x, y + height, x + width, y + height);
	}
}

// Draw a triangle with a shader.
// If uvs is NULL, a default will be used (0,0; 1,0; 0,1).
void pax_shade_tri(pax_buf_t *buf, pax_col_t color, pax_shader_t *shader,
		pax_tri_t *uvs, float x0, float y0, float x1, float y1, float x2, float y2) {
	PAX_BUF_CHECK("pax_shade_tri");
	matrix_2d_transform(buf->stack_2d.value, &x0, &y0);
	matrix_2d_transform(buf->stack_2d.value, &x1, &y1);
	matrix_2d_transform(buf->stack_2d.value, &x2, &y2);
	
	if (!isfinite(x0) || !isfinite(y0) || !isfinite(x1) || !isfinite(y1) || !isfinite(x2) || !isfinite(y2)) {
		// We can't draw to infinity.
		pax_last_error = PAX_ERR_INF;
		return;
	}
	
	if (!uvs) {
		uvs = &(pax_tri_t) {
			.x0 = 0, .y0 = 0,
			.x1 = 1, .y1 = 0,
			.x2 = 0, .y2 = 1
		};
	}
	
	// Sort points by height.
	if (y1 < y0) {
		PAX_SWAP_POINTS(x0, y0, x1, y1);
		PAX_SWAP_POINTS(uvs->x0, uvs->y0, uvs->x1, uvs->y1);
	}
	if (y2 < y0) {
		PAX_SWAP_POINTS(x0, y0, x2, y2);
		PAX_SWAP_POINTS(uvs->x0, uvs->y0, uvs->x2, uvs->y2);
	}
	if (y2 < y1) {
		PAX_SWAP_POINTS(x1, y1, x2, y2);
		PAX_SWAP_POINTS(uvs->x1, uvs->y1, uvs->x2, uvs->y2);
	}
	
	if (y2 == y0 || (x2 == x0 && x1 == x0)) {
		// We can't draw a flat triangle.
		PAX_SUCCESS();
		return;
	}
	
	pax_tri_shaded(buf, color, shader,
		x0, y0, x1, y1, x2, y2,
		uvs->x0, uvs->y0, uvs->x1, uvs->y1, uvs->x2, uvs->y2
	);
	
	PAX_SUCCESS();
}

// Draw an arc with a shader, angles in radians.
// If uvs is NULL, a default will be used (0,0; 1,0; 1,1; 0,1).
void pax_shade_arc(pax_buf_t *buf, pax_col_t color, pax_shader_t *shader,
		pax_quad_t *uvs, float x,  float y,  float r,  float a0, float a1);

// Draw a circle with a shader.
// If uvs is NULL, a default will be used (0,0; 1,0; 1,1; 0,1).
void pax_shade_circle(pax_buf_t *buf, pax_col_t color, pax_shader_t *shader,
		pax_quad_t *uvs, float x,  float y,  float r);

// Draw a rectangle.
void pax_draw_rect(pax_buf_t *buf, pax_col_t color, float x, float y, float width, float height) {
	PAX_BUF_CHECK("pax_draw_rect");
	if (matrix_2d_is_identity2(buf->stack_2d.value)) {
		// This can be simplified significantly.
		matrix_2d_transform(buf->stack_2d.value, &x, &y);
		width  *= buf->stack_2d.value.a0;
		height *= buf->stack_2d.value.b1;
		pax_simple_rect(buf, color, x, y, width, height);
	} else {
		// We need to go full quad.
		float x0 = x,         y0 = y;
		float x1 = x + width, y1 = y;
		float x2 = x + width, y2 = y + height;
		float x3 = x,         y3 = y + height;
		matrix_2d_transform(buf->stack_2d.value, &x0, &y0);
		matrix_2d_transform(buf->stack_2d.value, &x1, &y1);
		matrix_2d_transform(buf->stack_2d.value, &x2, &y2);
		matrix_2d_transform(buf->stack_2d.value, &x3, &y3);
		pax_simple_tri(buf, color, x0, y0, x1, y1, x2, y2);
		pax_simple_tri(buf, color, x0, y0, x3, y3, x2, y2);
	}
}

// Draw a line.
void pax_draw_line(pax_buf_t *buf, pax_col_t color, float x0, float y0, float x1, float y1) {
	PAX_BUF_CHECK("pax_draw_line");
	matrix_2d_transform(buf->stack_2d.value, &x0, &y0);
	matrix_2d_transform(buf->stack_2d.value, &x1, &y1);
	pax_simple_line(buf, color, x0, y0, x1, y1);
}

// Draw a triangle.
void pax_draw_tri(pax_buf_t *buf, pax_col_t color, float x0, float y0, float x1, float y1, float x2, float y2) {
	PAX_BUF_CHECK("pax_draw_tri");
	matrix_2d_transform(buf->stack_2d.value, &x0, &y0);
	matrix_2d_transform(buf->stack_2d.value, &x1, &y1);
	matrix_2d_transform(buf->stack_2d.value, &x2, &y2);
	pax_simple_tri(buf, color, x0, y0, x1, y1, x2, y2);
}

// Draw na arc, angles in radians.
void pax_draw_arc(pax_buf_t *buf, pax_col_t color, float x,  float y,  float r,  float a0, float a1) {
	PAX_BUF_CHECK("pax_draw_arc");
	
	// Simplify the angles slightly.
	float a2 = fmodf(a0, M_PI * 2);
	a1 += a2 - a0;
	a0 = a2;
	if (a1 < a0) PAX_SWAP(float, a0, a1);
	if (a1 - a0 > M_PI * 2) {
		a1 = M_PI * 2;
		a0 = 0;
	}
	
	// Pick an appropriate number of divisions.
	int n_div;
	matrix_2d_t matrix = buf->stack_2d.value;
	float _r = r * sqrtf(matrix.a0*matrix.a0 + matrix.b0*matrix.b0) * sqrtf(matrix.a1*matrix.a1 + matrix.b1*matrix.b1);
	if (_r > 30) n_div = (a1 - a0) / M_PI * 32 + 1;
	else n_div = (a1 - a0) / M_PI * 16 + 1;
	
	// Get the sine and cosine of one division, used for rotation in the loop.
	float div_angle = (a1 - a0) / n_div;
	float _sin = sinf(div_angle);
	float _cos = cosf(div_angle);
	
	// Start with a unit vector matrix according to a0.
	float x0 = cosf(a0);
	float y0 = sinf(a0);
	
	// Draw it as a series of triangles, rotating with what is essentially matrix multiplication.
	for (int i = 0; i < n_div; i++) {
		// Perform the rotation.
		float x1 = x0 * _cos - y0 * _sin;
		float y1 = x0 * _sin + y0 * _cos;
		// We subtract y0 and y1 from y because our up is -y.
		pax_draw_tri(buf, color, x, y, x + x0 * r, y - y0 * r, x + x1 * r, y - y1 * r);
		// Assign them yes.
		x0 = x1;
		y0 = y1;
	}
	
	PAX_SUCCESS();
}

// Draw a circle.
void pax_draw_circle(pax_buf_t *buf, pax_col_t color, float x,  float y,  float r) {
	pax_draw_arc(buf, color, x, y, r, 0, M_PI * 2);
}



/* ======= DRAWING: TEXT ======= */

// Determine whether a character is visible.
// Includes space.
static inline bool pax_is_visible_char(char c) {
	return c > 0x1f && c < 0x7f;
}

// Draw a string with the given font.
// If font is NULL, the default font (7x9) will be used.
void pax_draw_text(pax_buf_t *buf, pax_col_t color, pax_font_t *font, float font_size, float _x, float _y, char *text) {
	PAX_BUF_CHECK("pax_draw_text");
	
	if (!font) font = PAX_FONT_DEFAULT;
	
	if (font_size == 0) font_size = font->glyphs_uni_h;
	float size_mul = font_size / font->glyphs_uni_h;
	float w = size_mul * font->glyphs_uni_w;
	float h = size_mul * font->glyphs_uni_h;
	
	size_t len = strlen(text);
	
	float x = _x, y = _y;
	
	pax_shader_font_bitmap_uni_args_t args = {
		.font          = font
	};
	pax_shader_t shader = {
		.callback      = pax_shader_font_bitmap_uni,
		.callback_args = &args
	};
	
	for (size_t i = 0; i < len; i ++) {
		char c = text[i], next = text[i + 1];
		if (c == '\r' || c == '\n') {
			x = _x;
			y += h + 1;
			if (c == '\r' && next == '\n') i++;
		} else {
			args.glyph = pax_is_visible_char(c) ? c : 1;
			pax_shade_rect(buf, color, &shader, NULL, x, y, w, h);
			x += w;
		}
	}
	
	PAX_SUCCESS();
}

// Calculate the size of the string with the given font.
// Size is before matrix transformation.
// If font is NULL, the default font (7x9) will be used.
pax_vec1_t pax_text_size(pax_font_t *font, float font_size, char *text) {
	if (!font) font = PAX_FONT_DEFAULT;
	
	if (font_size == 0) font_size = font->glyphs_uni_h;
	float size_mul = font_size / font->glyphs_uni_h;
	float w = size_mul * font->glyphs_uni_w;
	float h = size_mul * font->glyphs_uni_h;
	
	float text_w = 0;
	float text_h = h + 1;
	
	size_t len = strlen(text);
	
	float x = 0, y = 0;
	
	for (size_t i = 0; i < len; i ++) {
		char c = text[i], next = text[i + 1];
		if (c == '\r' || c == '\n') {
			x = 0;
			y += h + 1;
			text_h = y + h + 1;
			if (c == '\r' && next == '\n') i++;
		} else {
			x += w;
			float _text_w = x + w;
			if (_text_w > text_w) text_w = _text_w;
		}
	}
	
	return (pax_vec1_t) { .x = text_w, .y = text_h };
}



/* ======= DRAWING: SIMPLE ======= */

// Fill the background.
void pax_background(pax_buf_t *buf, pax_col_t color) {
	PAX_BUF_CHECK("pax_background");
	
	uint32_t value = pax_col2buf(buf, color);
	
	if (buf->bpp == 16) {
		for (size_t i = 0; i < buf->width * buf->height; i++) {
			buf->buf_16bpp[i] = value;
		}
	} else if (buf->bpp == 32) {
		for (size_t i = 0; i < buf->width * buf->height; i++) {
			buf->buf_32bpp[i] = value;
		}
	}
	
	PAX_SUCCESS();
}

// Draw a rectangle, ignoring matrix transform.
void pax_simple_rect(pax_buf_t *buf, pax_col_t color, float x, float y, float width, float height) {
	PAX_BUF_CHECK("pax_simple_rect");
	
	// Fix rect dimensions.
	if (width < 0) {
		width = -width;
		x -= width;
	}
	if (height < 0) {
		height = -height;
		y -= height;
	}
	
	// Clip rect in inside of buffer.
	if (x < buf->clip.x) {
		width += buf->clip.x - x;
		x = buf->clip.x;
	}
	if (y < buf->clip.y) {
		height += buf->clip.y - y;
		y = buf->clip.y;
	}
	if (x + width > buf->clip.x + buf->clip.w) {
		width = buf->clip.x + buf->clip.w - x;
	}
	if (y + height > buf->clip.y + buf->clip.h) {
		height = buf->clip.y + buf->clip.h - y;
	}
	
	pax_mark_dirty2(buf, x + 0.5, y + 0.5, width + 0.5, height + 0.5);
	
	// Pixel time.
	for (int _y = y + 0.5; _y < y + height + 0.5; _y ++) {
		for (int _x = x + 0.5; _x < x + width + 0.5; _x ++) {
			pax_merge_pixel(buf, color, _x, _y);
		}
	}
	
	PAX_SUCCESS();
}

// Draw a line, ignoring matrix transform.
void pax_simple_line(pax_buf_t *buf, pax_col_t color, float x0, float y0, float x1, float y1) {
	PAX_BUF_CHECK("pax_simple_line");
	
	if (!isfinite(x0) || !isfinite(y0) || !isfinite(x1) || !isfinite(y1)) {
		// We can't draw to infinity.
		pax_last_error = PAX_ERR_INF;
		return;
	}
	
	if (y0 > y1) PAX_SWAP_POINTS(x0, y0, x1, y1);
	
	// Determine whether the line might fall within the clip rect.
	if (!buf->clip.w || !buf->clip.h) goto noneed;
	if (y1 < buf->clip.y || y0 > buf->clip.y + buf->clip.h - 1) goto noneed;
	if (x0 == x1 && (x0 < buf->clip.x || x0 > buf->clip.x + buf->clip.w - 1)) goto noneed;
	if (x0 < buf->clip.x && x1 < buf->clip.x) goto noneed;
	if (x0 > buf->clip.x + buf->clip.w - 1 && x1 > buf->clip.x + buf->clip.w - 1) goto noneed;
	
	// Clip top.
	if (y0 < buf->clip.y) {
		x0 = (x1 - x0) * (buf->clip.y - y0) / (y1 - y0);
		y0 = buf->clip.y;
	}
	// Clip bottom.
	if (y1 > buf->clip.y + buf->clip.h - 1) {
		x1 = (x1 - x0) * (buf->clip.y + buf->clip.h - 1 - y0) / (y1 - y0);
		y1 = buf->clip.y + buf->clip.h - 1;
	}
	// Clip left.
	if (x1 < buf->clip.x) {
		y1 = (y1 - y0) * (buf->clip.x - x0) / (x1 - x0);
		x1 = buf->clip.x;
	} else if (x0 < buf->clip.x) {
		y0 = (y1 - y0) * (buf->clip.x - x0) / (x1 - x0);
		x0 = buf->clip.x;
	}
	// Clip right.
	if (x1 > buf->clip.x + buf->clip.w - 1) {
		y1 = (y1 - y0) * (buf->clip.x + buf->clip.w - 1 - x0) / (x1 - x0);
		x1 = buf->clip.x + buf->clip.w - 1;
	} else if (x0 > buf->clip.x + buf->clip.w - 1) {
		y0 = (y1 - y0) * (buf->clip.x + buf->clip.w - 1 - x0) / (x1 - x0);
		x0 = buf->clip.x + buf->clip.w - 1;
	}
	
	// If any point is outside clip now, we don't draw a line.
	if (y0 < buf->clip.y || y1 > buf->clip.y + buf->clip.h - 1) goto noneed;
	
	// Determine whether the line is "steep" (dx*dx > dy*dy).
	float dx = x1 - x0;
	float dy = y1 - y0;
	bool is_steep = fabs(dx) < fabs(dy);
	int nIter;
	
	if (is_steep) nIter = fabs(dy) + 0.5;
	else nIter = fabs(dx) + 0.5;
	
	// Adjust dx and dy.
	dx /= nIter;
	dy /= nIter;
	float x = x0, y = y0;
	for (int i = 0; i < nIter; i++) {
		x += dx;
		y += dy;
		pax_merge_pixel(buf, color, x + 0.5, y + 0.5);
	}
	
	// This label is used if there's no need to try to draw a line.
	noneed:
	PAX_SUCCESS();
}

// Draw a triangle, ignoring matrix transform.
void pax_simple_tri(pax_buf_t *buf, pax_col_t color, float x0, float y0, float x1, float y1, float x2, float y2) {
	PAX_BUF_CHECK("pax_simple_tri");
	
	if (!isfinite(x0) || !isfinite(y0) || !isfinite(x1) || !isfinite(y1) || !isfinite(x2) || !isfinite(y2)) {
		// We can't draw to infinity.
		pax_last_error = PAX_ERR_INF;
		return;
	}
	
	// Sort points by height.
	PAX_SORT_POINTS(x0, y0, x1, y1);
	PAX_SORT_POINTS(x0, y0, x2, y2);
	PAX_SORT_POINTS(x1, y1, x2, y2);
	
	if (y2 == y0 || (x2 == x0 && x1 == x0)) {
		// We can't draw a flat triangle.
		PAX_SUCCESS();
		return;
	}
	
	pax_tri_unshaded(buf, color, x0, y0, x1, y1, x2, y2);
	
	PAX_SUCCESS();
}

// Draw a arc, ignoring matrix transform.
// Angles in radians.
void pax_simple_arc(pax_buf_t *buf, pax_col_t color, float x, float y, float r, float a0, float a1) {
	PAX_BUF_CHECK("pax_simple_arc");
	
	// Simplify the angles slightly.
	float a2 = fmodf(a0, M_PI * 2);
	a1 += a2 - a0;
	a0 = a2;
	if (a1 < a0) PAX_SWAP(float, a0, a1);
	if (a1 - a0 > M_PI * 2) {
		a1 = M_PI * 2;
		a0 = 0;
	}
	
	// Pick an appropriate number of divisions.
	int n_div;
	if (r > 30) n_div = (a1 - a0) / M_PI * 32 + 1;
	if (r > 20) n_div = (a1 - a0) / M_PI * 16 + 1;
	else n_div = (a1 - a0) / M_PI * 8 + 1;
	
	// Get the sine and cosine of one division, used for rotation in the loop.
	float div_angle = (a1 - a0) / n_div;
	float _sin = sinf(div_angle);
	float _cos = cosf(div_angle);
	
	// Start with a unit vector matrix according to a0.
	float x0 = cosf(a0);
	float y0 = sinf(a0);
	
	// Draw it as a series of triangles, rotating with what is essentially matrix multiplication.
	for (int i = 0; i < n_div; i++) {
		// Perform the rotation.
		float x1 = x0 * _cos - y0 * _sin;
		float y1 = x0 * _sin + y0 * _cos;
		// We subtract y0 and y1 from y because our up is -y.
		pax_simple_tri(buf, color, x, y, x + x0 * r, y - y0 * r, x + x1 * r, y - y1 * r);
		// Assign them yes.
		x0 = x1;
		y0 = y1;
	}
	
	PAX_SUCCESS();
}

// Draw a circle, ignoring matrix transform.
void pax_simple_circle(pax_buf_t *buf, pax_col_t color, float x, float y, float r) {
	pax_simple_arc(buf, color, x, y, r, 0, M_PI);
}
