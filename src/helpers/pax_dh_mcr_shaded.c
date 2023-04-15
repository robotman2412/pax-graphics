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

#ifndef PAX_GFX_C
#ifndef ARDUINO
#pragma message "This file should not be compiled on it's own."
#endif
#else

#include "pax_internal.h"

/* ======== SHADED DRAWING ======= */

// Multi-core method for shaded triangles.
// If odd_scanline is true, the odd (counted from 0) lines are drawn, otherwise the even lines are drawn.
void paxmcr_tri_shaded(bool odd_scanline, pax_buf_t *buf, pax_col_t color, const pax_shader_t *shader,
		float x0, float y0, float x1, float y1, float x2, float y2,
		float u0, float v0, float u1, float v1, float u2, float v2) {
	
	pax_shader_ctx_t shader_ctx = pax_get_shader_ctx(buf, color, shader);
	if (shader_ctx.skip) return;
	pax_col_conv_t buf2col = PAX_IS_PALETTE(buf->type) ? pax_col_conv_dummy : buf->buf2col;
	
	// Sort points by height.
	if (y1 < y0) {
		PAX_SWAP_POINTS(x0, y0, x1, y1);
		PAX_SWAP_POINTS(u0, v0, u1, v1);
	}
	if (y2 < y0) {
		PAX_SWAP_POINTS(x0, y0, x2, y2);
		PAX_SWAP_POINTS(u0, v0, u2, v2);
	}
	if (y2 < y1) {
		PAX_SWAP_POINTS(x1, y1, x2, y2);
		PAX_SWAP_POINTS(u1, v1, u2, v2);
	}
	
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
		// Snap y to the correct line.
		int y = y_post_0;
		if ((y & 1) != odd_scanline) {
			y ++;
			x_a += x0_x1_dx;
			x_b += x0_x2_dx;
			u_a += u0_u1_du;
			v_a += v0_v1_dv;
			u_b += u0_u2_du;
			v_b += v0_v2_dv;
		}
		// Precalc index stuff.
		int delta = y * buf->width;
		for (; y < (int) y_post_1; y += 2) {
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
			if (x_right > buf->clip.x + buf->clip.w - 1) {
				float new_x_right = buf->clip.x + buf->clip.w - 1;
				float delta = (new_x_right - x_left) / (x_right - x_left);
				x_right = new_x_right;
				u_right = u_left + (u_right - u_left) * delta;
				v_right = v_left + (v_right - v_left) * delta;
			}
			if (x_left < buf->clip.x) {
				float new_x_left = buf->clip.x;
				float delta = (new_x_left - x_left) / (x_right - x_left);
				x_left = new_x_left;
				u_left = u_left + (u_right - u_left) * delta;
				v_left = v_left + (v_right - v_left) * delta;
			}
			// Find UV ranges.
			int x = x_left + 0.5;
			int nIter = x_right - x;
			// Fix UVs.
			float u = u_left, v = v_left;
			float du = (u_right - u_left) / nIter;
			float dv = (v_right - v_left) / nIter;
			x_right -= 0.5;
			for (; x <= x_right; x ++) {
				// Apply the shader,
				pax_col_t result = (shader_ctx.callback)(color, shader_ctx.do_getter ? buf2col(buf, buf->getter(buf, x+delta)) : 0, x, y, u, v, shader_ctx.callback_args);
				// And simply merge colors accordingly.
				pax_set_index_conv(buf, result, x+delta);
				u += du;
				v += dv;
			}
			// Move X.
			x_a   += 2*x0_x1_dx;
			x_b   += 2*x0_x2_dx;
			u_a   += 2*u0_u1_du;
			v_a   += 2*v0_v1_dv;
			u_b   += 2*u0_u2_du;
			v_b   += 2*v0_v2_dv;
			delta += 2*buf->width;
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
		// Snap y to the correct line.
		int y = y_post_1;
		if ((y & 1) != odd_scanline) {
			y ++;
			x_a += x1_x2_dx;
			x_b += x0_x2_dx;
			u_a += u1_u2_du;
			v_a += v1_v2_dv;
			u_b += u0_u2_du;
			v_b += v0_v2_dv;
		}
		// Precalc index stuff.
		int delta = y * buf->width;
		for (; y <= (int) y_pre_2; y += 2) {
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
			if (x_right > buf->clip.x + buf->clip.w - 1) {
				float new_x_right = buf->clip.x + buf->clip.w - 1;
				float delta = (new_x_right - x_left) / (x_right - x_left);
				x_right = new_x_right;
				u_right = u_left + (u_right - u_left) * delta;
				v_right = v_left + (v_right - v_left) * delta;
			}
			if (x_left < buf->clip.x) {
				float new_x_left = buf->clip.x;
				float delta = (new_x_left - x_left) / (x_right - x_left);
				x_left = new_x_left;
				u_left = u_left + (u_right - u_left) * delta;
				v_left = v_left + (v_right - v_left) * delta;
			}
			// Find UV ranges.
			int x = x_left + 0.5;
			int nIter = x_right - x;
			// Fix UVs.
			float u = u_left, v = v_left;
			float du = (u_right - u_left) / nIter;
			float dv = (v_right - v_left) / nIter;
			x_right -= 0.5;
			for (; x <= x_right; x ++) {
				// Apply the shader,
				pax_col_t result = (shader_ctx.callback)(color, shader_ctx.do_getter ? buf2col(buf, buf->getter(buf, x+delta)) : 0, x, y, u, v, shader_ctx.callback_args);
				// And simply merge colors accordingly.
				pax_set_index_conv(buf, result, x+delta);
				u += du;
				v += dv;
			}
			// Move X.
			x_a   += 2*x1_x2_dx;
			x_b   += 2*x0_x2_dx;
			u_a   += 2*u1_u2_du;
			v_a   += 2*v1_v2_dv;
			u_b   += 2*u0_u2_du;
			v_b   += 2*v0_v2_dv;
			delta += 2*buf->width;
		}
	}
}

// Multi-core optimisation which maps a buffer directly onto another.
// If odd_scanline is true, the odd (counted from 0) lines are drawn, otherwise the even lines are drawn.
void paxmcr_overlay_buffer(bool odd_scanline, pax_buf_t *base, pax_buf_t *top, int x, int y, int width, int height, bool assume_opaque) {
	int tex_x = 0, tex_y = 0;
	
	// Perform clipping.
	if (x < base->clip.x) {
		tex_x = base->clip.x - x;
		width -= tex_x;
		x  = base->clip.x;
	}
	if (x + width > base->clip.x + base->clip.w) {
		width = base->clip.x + base->clip.w - x;
	}
	if (y < base->clip.y) {
		tex_y = base->clip.y - y;
		height -= tex_y;
		y  = base->clip.y;
	}
	if (y + height > base->clip.y + base->clip.h) {
		height = base->clip.y + base->clip.h - y;
	}
	
	PAX_LOGD(TAG, "paxmcr_overlay_buffer");
	
	bool equal = top->type == base->type;
	if (equal && x == 0 && y == 0 && width == base->width && height == base->height && base->reverse_endianness == top->reverse_endianness) {
		// When copying one buffer onto another as a background,
		// and the types are the same, perform a memcpy() instead.
		// memcpy(base->buf, top->buf, (PAX_GET_BPP(base->type) * width * height + 7) >> 3);
		// return;
	}
	// Fix Y co-ordinates.
	if ((y & 1) != odd_scanline) {
		y ++;
		tex_y ++;
	}
	
	// Now, let us MAP.
	int top_delta  = tex_y * top->width;
	int base_delta = y     * base->width;
	if (assume_opaque) {
		if (equal) {
			// Equal types and alpha.
			for (int c_y = odd_scanline; c_y < height; c_y += 2) {
				for (int c_x = 0; c_x < width; c_x++) {
					pax_col_t col = top->getter(top, tex_x+top_delta);
					base->setter(base, col, x+base_delta);
					tex_x ++;
					x ++;
				}
				tex_x      -= width;
				x          -= width;
				top_delta  += 2*top->width;
				base_delta += 2*base->width;
			}
		} else {
			// Not equal types, but no alpha.
			for (int c_y = odd_scanline; c_y < height; c_y += 2) {
				for (int c_x = 0; c_x < width; c_x++) {
					pax_col_t col = top->buf2col(top, top->getter(top, tex_x+top_delta));
					base->setter(base, base->col2buf(base, col), x+base_delta);
					tex_x ++;
					x ++;
				}
				tex_x      -= width;
				x          -= width;
				top_delta  += 2*top->width;
				base_delta += 2*base->width;
			}
		}
	} else {
		// With alpha.
		for (int c_y = odd_scanline; c_y < height; c_y += 2) {
			for (int c_x = 0; c_x < width; c_x++) {
				pax_col_t col = top->buf2col(top, top->getter(top, tex_x+top_delta));
				pax_merge_index(base, col, x+base_delta);
				tex_x ++;
				x ++;
			}
			tex_x      -= width;
			x          -= width;
			top_delta  += 2*top->width;
			base_delta += 2*base->width;
		}
	}
}

// Multi-core optimisation which makes more assumptions about UVs.
// If odd_scanline is true, the odd (counted from 0) lines are drawn, otherwise the even lines are drawn.
void paxmcr_rect_shaded1(bool odd_scanline, pax_buf_t *buf, pax_col_t color, const pax_shader_t *shader,
		float x, float y, float width, float height, float u0, float v0, float u1, float v1) {
	
	pax_shader_ctx_t shader_ctx = pax_get_shader_ctx(buf, color, shader);
	if (shader_ctx.skip) return;
	pax_col_conv_t buf2col = PAX_IS_PALETTE(buf->type) ? pax_col_conv_dummy : buf->buf2col;
	
	// Fix width and height.
	if (width < 0) {
		x += width;
		width = -width;
		PAX_SWAP(float, u0, u1);
	}
	if (height < 0) {
		y += height;
		height = -height;
		PAX_SWAP(float, v0, v1);
	}
	
	// Clip rect in inside of buffer.
	if (x < buf->clip.x) {
		float part = (buf->clip.x - x) / width;
		u0 = u0 + (u1 - u0) * part;
		
		width -= buf->clip.x - x;
		x = buf->clip.x;
	}
	if (x + width > buf->clip.x + buf->clip.w) {
		float part = (buf->clip.x + buf->clip.w - x) / width;
		u1 = u0 + (u1 - u0) * part;
		
		width = buf->clip.x + buf->clip.w - x;
	}
	if (y < buf->clip.y) {
		float part = (buf->clip.y - y) / height;
		v0 = v0 + (v1 - v0) * part;
		
		height -= buf->clip.y - y;
		y = buf->clip.y;
	}
	if (y + height > buf->clip.y + buf->clip.h) {
		float part = (buf->clip.y + buf->clip.h - y) / height;
		v1 = v0 + (v1 - v0) * part;
		
		height = buf->clip.y + buf->clip.h - y;
	}
	
	// Adjust UVs to match pixel co-ordinates.
	float min_x = (int) (x + 0.5)          + 0.5;
	float max_x = (int) (x + width - 0.5)  + 0.5;
	float min_y = (int) (y + 0.5)          + 0.5;
	float max_y = (int) (y + height - 0.5) + 0.5;
	{ // Adjust the X part.
		float new_u0 = u0 + (u1 - u0) / width * (min_x - x);
		float new_u1 = u0 + (u1 - u0) / width * (max_x - x);
		u0 = new_u0;
		u1 = new_u1;
	}
	{ // Adjust the Y part.
		float new_v0 = v0 + (v1 - v0) / height * (min_y - y);
		float new_v1 = v0 + (v1 - v0) / height * (max_y - y);
		v0 = new_v0;
		v1 = new_v1;
	}
	
	// Find UV deltas.
	float u0_u1_du = (u1 - u0) / (max_x - min_x);
	float v0_v1_dv = (v1 - v0) / (max_y - min_y);
	
	float v = v0;
	
	// Snap c_y to the correct line.
	int c_y = y + 0.5;
	if ((c_y & 1) != odd_scanline) {
		c_y ++;
		v += v0_v1_dv;
	}
	
	// Pixel time.
	int delta = c_y * buf->width;
	for (; c_y <= y + height - 0.5; c_y += 2) {
		float u = u0;
		for (int c_x = x + 0.5; c_x <= x + width - 0.5; c_x ++) {
			pax_col_t result = (shader_ctx.callback)(color, shader_ctx.do_getter ? buf2col(buf, buf->getter(buf, c_x+delta)) : 0, c_x, c_y, u, v, shader_ctx.callback_args);
			pax_set_index_conv(buf, result, c_x+delta);
			u += u0_u1_du;
		}
		v     += 2*v0_v1_dv;
		delta += 2*buf->width;
	}
	
}

// Multi-core method for shaded rects.
// If odd_scanline is true, the odd (counted from 0) lines are drawn, otherwise the even lines are drawn.
void paxmcr_rect_shaded(bool odd_scanline, pax_buf_t *buf, pax_col_t color, const pax_shader_t *shader,
		float x, float y, float width, float height,
		float u0, float v0, float u1, float v1, float u2, float v2, float u3, float v3) {
	
	bool is_default_uv = u0 == 0 && v0 == 0 && u1 == 1 && v1 == 0 && u2 == 1 && v2 == 1 && u3 == 0 && v3 == 1;
	// Try to perform a mapping optimisation.
	if ((shader->callback == pax_shader_texture || shader->callback == pax_shader_texture_aa) && color == 0xffffffff) {
		pax_buf_t *top = (pax_buf_t *) shader->callback_args;
		if (is_default_uv && (int) (width + 0.5) == top->width && (int) (height + 0.5) == top->height) {
			paxmcr_overlay_buffer(odd_scanline, buf, top, x + 0.5, y + 0.5, width + 0.5, height + 0.5, shader->alpha_promise_255);
			return;
		}
	} else if (is_default_uv || (v0 == v1 && v2 == v3 && u0 == u3 && u1 == u2)) {
		paxmcr_rect_shaded1(odd_scanline, buf, color, shader, x, y, width, height, u0, v0, u2, v2);
		return;
	}
	
	pax_shader_ctx_t shader_ctx = pax_get_shader_ctx(buf, color, shader);
	if (shader_ctx.skip) return;
	pax_col_conv_t buf2col = PAX_IS_PALETTE(buf->type) ? pax_col_conv_dummy : buf->buf2col;
	
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
	
	// Clip rect in inside of buffer.
	if (x < buf->clip.x) {
		float part = (buf->clip.x - x) / width;
		u0 = u0 + (u1 - u0) * part;
		v0 = v0 + (v1 - v0) * part;
		u3 = u3 + (u2 - u3) * part;
		v3 = v3 + (v2 - v3) * part;
		
		width -= buf->clip.x - x;
		x = buf->clip.x;
	}
	if (x + width > buf->clip.x + buf->clip.w) {
		float part = (buf->clip.x + buf->clip.w - x) / width;
		u1 = u0 + (u1 - u0) * part;
		v1 = v0 + (v1 - v0) * part;
		u2 = u3 + (u2 - u3) * part;
		v2 = v3 + (v2 - v3) * part;
		
		width = buf->clip.x + buf->clip.w - x;
	}
	if (y < buf->clip.y) {
		float part = (buf->clip.y - y) / height;
		u0 = u0 + (u3 - u0) * part;
		v0 = v0 + (v3 - v0) * part;
		u1 = u1 + (u2 - u1) * part;
		v1 = v1 + (v2 - v1) * part;
		
		height -= buf->clip.y - y;
		y = buf->clip.y;
	}
	if (y + height > buf->clip.y + buf->clip.h) {
		float part = (buf->clip.y + buf->clip.h - y) / height;
		u3 = u0 + (u3 - u0) * part;
		v3 = v0 + (v3 - v0) * part;
		u2 = u1 + (u2 - u1) * part;
		v2 = v1 + (v2 - v1) * part;
		
		height = buf->clip.y + buf->clip.h - y;
	}
	
	// Adjust UVs to match pixel co-ordinates.
	float min_x = (int) (x + 0.5)          + 0.5;
	float max_x = (int) (x + width - 0.5)  + 0.5;
	float min_y = (int) (y + 0.5)          + 0.5;
	float max_y = (int) (y + height - 0.5) + 0.5;
	// Adjust X part.
	{ // Adjust UV0 and UV1.
		float new_u0 = u0 + (u1 - u0) / width * (min_x - x);
		float new_u1 = u0 + (u1 - u0) / width * (max_x - x);
		float new_v0 = v0 + (v1 - v0) / width * (min_x - x);
		float new_v1 = v0 + (v1 - v0) / width * (max_x - x);
		u0 = new_u0;
		u1 = new_u1;
		v0 = new_v0;
		v1 = new_v1;
	}
	{ // Adjust UV3 and UV2.
		float new_u3 = u3 + (u2 - u3) / width * (min_x - x);
		float new_u2 = u3 + (u2 - u3) / width * (max_x - x);
		float new_v3 = v3 + (v2 - v3) / width * (min_x - x);
		float new_v2 = v3 + (v2 - v3) / width * (max_x - x);
		u3 = new_u3;
		u2 = new_u2;
		v3 = new_v3;
		v2 = new_v2;
	}
	// Adjust Y part.
	{ // Adjust UV1 and UV2.
		float new_u1 = u1 + (u2 - u1) / height * (min_y - y);
		float new_u2 = u1 + (u2 - u1) / height * (max_y - y);
		float new_v1 = v1 + (v2 - v1) / height * (min_y - y);
		float new_v2 = v1 + (v2 - v1) / height * (max_y - y);
		u1 = new_u1;
		u2 = new_u2;
		v1 = new_v1;
		v2 = new_v2;
	}
	{ // Adjust UV0 and UV3.
		float new_u0 = u0 + (u3 - u0) / height * (min_y - y);
		float new_u3 = u0 + (u3 - u0) / height * (max_y - y);
		float new_v0 = v0 + (v3 - v0) / height * (min_y - y);
		float new_v3 = v0 + (v3 - v0) / height * (max_y - y);
		u0 = new_u0;
		u3 = new_u3;
		v0 = new_v0;
		v3 = new_v3;
	}
	
	// Find UV deltas.
	float u0_u3_du = (u3 - u0) / (max_y - min_y);
	float v0_v3_dv = (v3 - v0) / (max_y - min_y);
	float u1_u2_du = (u2 - u1) / (max_y - min_y);
	float v1_v2_dv = (v2 - v1) / (max_y - min_y);
	
	float u_a = u0, v_a = v0;
	float u_b = u1, v_b = v1;
	
	// Snap c_y to the correct line.
	int c_y = y + 0.5;
	if ((c_y & 1) != odd_scanline) {
		c_y ++;
		u_a += u0_u3_du;
		v_a += v0_v3_dv;
		u_b += u1_u2_du;
		v_b += v1_v2_dv;
	}
	
	// Pixel time.
	int delta = c_y * buf->width;
	for (; c_y <= y + height - 0.5; c_y += 2) {
		float ua_ub_du = (u_b - u_a) / (max_x - min_x);
		float va_vb_dv = (v_b - v_a) / (max_x - min_x);
		float u = u_a, v = v_a;
		for (int c_x = x + 0.5; c_x <= x + width - 0.5; c_x ++) {
			pax_col_t result = (shader_ctx.callback)(color, shader_ctx.do_getter ? buf2col(buf, buf->getter(buf, c_x+delta)) : 0, c_x, c_y, u, v, shader_ctx.callback_args);
			pax_set_index_conv(buf, result, c_x+delta);
			u += ua_ub_du;
			v += va_vb_dv;
		}
		u_a   += 2*u0_u3_du;
		v_a   += 2*v0_v3_dv;
		u_b   += 2*u1_u2_du;
		v_b   += 2*v1_v2_dv;
		delta += 2*buf->width;
	}
	
}

#endif
