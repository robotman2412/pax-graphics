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

#include "pax_internal.h"

#ifdef PDHG_NAME

#if defined(PDHG_SHADED) && !defined(PDHG_RESTRICT_UV) && !defined(PDHG_IGNORE_UV)
#define PDHG_NORMAL_UV
#endif

#ifdef PDHG_MCR
#define PDHG_INCREMENT 2
#else
#define PDHG_INCREMENT 1
#endif

#if !defined(PDHG_SHADED) && defined(PDHG_IGNORE_UV)
#error "Cannot define `PDHG_IGNORE_UV` without `PDHG_SHADED`."
#endif

#if !defined(PDHG_SHADED) && defined(PDHG_RESTRICT_UV)
#error "Cannot define `PDHG_RESTRICT_UV` without `PDHG_SHADED`."
#endif

#if defined(PDHG_RESTRICT_UV) && defined(PDHG_IGNORE_UV)
#error "Cannot define both `PDHG_IGNORE_UV` and `PDHG_RESTRICT_UV`; select at most one."
#endif

// Generic rectangle drawing code, assuming some defines are made.
#ifdef PDHG_STATIC
static
#endif
void PDHG_NAME(
#ifdef PDHG_MCR
		bool odd_scanline,
#endif
		pax_buf_t *buf, pax_col_t color,
#ifdef PDHG_SHADED
		const pax_shader_t *shader,
#endif
		float x, float y, float width, float height
#ifdef PDHG_NORMAL_UV
		, float u0, float v0, float u1, float v1, float u2, float v2, float u3, float v3
#endif
#ifdef PDHG_RESTRICT_UV
		, float u0, float v0, float u1, float v1
#endif
		) {
	
	#ifdef PDHG_SHADED
	// Get shader context.
	pax_shader_ctx_t shader_ctx = pax_get_shader_ctx(buf, color, shader);
	if (shader_ctx.skip) return;
	// Get pixel setter.
	pax_col_conv_t buf2col = PAX_IS_PALETTE(buf->type) ? pax_col_conv_dummy : buf->buf2col;
	#else // PDHG_SHADED
	// Get pixel setter.
	pax_index_setter_t setter = pax_get_setter(buf, &color, NULL);
	if (!setter) return;
	#endif // PDHG_SHADED
	
	// Fix width and height.
	if (width < 0) {
		x += width;
		width = -width;
		#ifdef PDHG_NORMAL_UV
		PAX_SWAP_POINTS(u0, v0, u1, v1);
		PAX_SWAP_POINTS(u2, v2, u3, v3);
		#endif
		#ifdef PDHG_RESTRICT_UV
		PAX_SWAP(float, u0, u1)
		#endif
	}
	if (height < 0) {
		y += height;
		height = -height;
		#ifdef PDHG_NORMAL_UV
		PAX_SWAP_POINTS(u0, v0, u3, v3);
		PAX_SWAP_POINTS(u1, v1, u2, v2);
		#endif
		#ifdef PDHG_RESTRICT_UV
		PAX_SWAP(float, v0, v1)
		#endif
	}
	
	// Clip rect in inside of buffer.
	if (x < buf->clip.x) {
		#ifdef PDHG_NORMAL_UV
		float part = (buf->clip.x - x) / width;
		u0 = u0 + (u1 - u0) * part;
		v0 = v0 + (v1 - v0) * part;
		u3 = u3 + (u2 - u3) * part;
		v3 = v3 + (v2 - v3) * part;
		#endif
		#ifdef PDHG_RESTRICT_UV
		float part = (buf->clip.x - x) / width;
		u0 = u0 + (u1 - u0) * part;
		#endif
		
		width -= buf->clip.x - x;
		x = buf->clip.x;
	}
	if (x + width > buf->clip.x + buf->clip.w) {
		#ifdef PDHG_NORMAL_UV
		float part = (buf->clip.x + buf->clip.w - x) / width;
		u1 = u0 + (u1 - u0) * part;
		v1 = v0 + (v1 - v0) * part;
		u2 = u3 + (u2 - u3) * part;
		v2 = v3 + (v2 - v3) * part;
		#endif
		#ifdef PDHG_RESTRICT_UV
		float part = (buf->clip.x + buf->clip.w - x) / width;
		u1 = u0 + (u1 - u0) * part;
		#endif
		
		width = buf->clip.x + buf->clip.w - x;
	}
	if (y < buf->clip.y) {
		#ifdef PDHG_NORMAL_UV
		float part = (buf->clip.y - y) / height;
		u0 = u0 + (u3 - u0) * part;
		v0 = v0 + (v3 - v0) * part;
		u1 = u1 + (u2 - u1) * part;
		v1 = v1 + (v2 - v1) * part;
		#endif
		#ifdef PDHG_RESTRICT_UV
		float part = (buf->clip.y - y) / height;
		v0 = v0 + (v1 - v0) * part;
		#endif
		
		height -= buf->clip.y - y;
		y = buf->clip.y;
	}
	if (y + height > buf->clip.y + buf->clip.h) {
		#ifdef PDHG_NORMAL_UV
		float part = (buf->clip.y + buf->clip.h - y) / height;
		u3 = u0 + (u3 - u0) * part;
		v3 = v0 + (v3 - v0) * part;
		u2 = u1 + (u2 - u1) * part;
		v2 = v1 + (v2 - v1) * part;
		#endif
		#ifdef PDHG_RESTRICT_UV
		float part = (buf->clip.y + buf->clip.h - y) / height;
		v1 = v0 + (v1 - v0) * part;
		#endif
		
		height = buf->clip.y + buf->clip.h - y;
	}
	
	#if defined(PDHG_NORMAL_UV) || defined(PDHG_RESTRICT_UV)
	// Adjust UVs to match pixel co-ordinates.
	float min_x = (int) (x + 0.5)          + 0.5;
	float max_x = (int) (x + width - 0.5)  + 0.5;
	float min_y = (int) (y + 0.5)          + 0.5;
	float max_y = (int) (y + height - 0.5) + 0.5;
	#endif
	
	#ifdef PDHG_NORMAL_UV
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
	float u0_u3_du = (u3 - u0) / (height - 1);
	float v0_v3_dv = (v3 - v0) / (height - 1);
	float u1_u2_du = (u2 - u1) / (height - 1);
	float v1_v2_dv = (v2 - v1) / (height - 1);
	
	float u_a = u0, v_a = v0;
	float u_b = u1, v_b = v1;
	#endif // PDHG_NORMAL_UV
	
	#ifdef PDHG_RESTRICT_UV
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
	#endif // PDHG_RESTRICT_UV
	
	int c_y = y + 0.5;
	#ifdef PDHG_MCR
	// Snap c_y to the correct line.
	if ((c_y & 1) != odd_scanline) {
		c_y ++;
		#ifdef PDHG_NORMAL_UV
		u_a += u0_u3_du;
		v_a += v0_v3_dv;
		u_b += u1_u2_du;
		v_b += v1_v2_dv;
		#endif
		#ifdef PDHG_RESTRICT_UV
		v   += v0_v1_dv;
		#endif
	}
	#endif
	
	// Pixel time.
	int delta = c_y * buf->width;
	for (; c_y <= y + height - 0.5; c_y += PDHG_INCREMENT) {
		#ifdef PDHG_NORMAL_UV
		float ua_ub_du = (u_b - u_a) / (max_x - min_x);
		float va_vb_dv = (v_b - v_a) / (max_x - min_x);
		float u = u_a, v = v_a;
		#endif
		#ifdef PDHG_RESTRICT_UV
		float u = u0;
		#endif
		for (int c_x = x + 0.5; c_x <= x + width - 0.5; c_x ++) {
			#ifdef PDHG_NORMAL_UV
			pax_col_t result = (shader_ctx.callback)(color, shader_ctx.do_getter ? buf2col(buf, buf->getter(buf, c_x+delta)) : 0, c_x, c_y, u, v, shader_ctx.callback_args);
			pax_set_index_conv(buf, result, c_x+delta);
			u += ua_ub_du;
			v += va_vb_dv;
			#endif
			#ifdef PDHG_RESTRICT_UV
			pax_col_t result = (shader_ctx.callback)(color, shader_ctx.do_getter ? buf2col(buf, buf->getter(buf, c_x+delta)) : 0, c_x, c_y, u, v, shader_ctx.callback_args);
			pax_set_index_conv(buf, result, c_x+delta);
			u += u0_u1_du;
			#endif
			#ifdef PDHG_IGNORE_UV
			pax_col_t result = (shader_ctx.callback)(color, shader_ctx.do_getter ? buf2col(buf, buf->getter(buf, c_x+delta)) : 0, c_x, c_y, 0, 0, shader_ctx.callback_args);
			pax_set_index_conv(buf, result, c_x+delta);
			#endif
			#ifndef PDHG_SHADED
			setter(buf, color, c_x+delta);
			#endif
		}
		#ifdef PDHG_NORMAL_UV
		u_a   += PDHG_INCREMENT*u0_u3_du;
		v_a   += PDHG_INCREMENT*v0_v3_dv;
		u_b   += PDHG_INCREMENT*u1_u2_du;
		v_b   += PDHG_INCREMENT*v1_v2_dv;
		#endif
		#ifdef PDHG_RESTRICT_UV
		v     += PDHG_INCREMENT*v0_v1_dv;
		#endif
		delta += PDHG_INCREMENT*buf->width;
	}
}

#endif // PDHG_NAME

// Clean up macros.
#undef PDHG_INCREMENT
#undef PDHG_NORMAL_UV

// Clean up parameter macros.
#undef PDHG_STATIC
#undef PDHG_NAME
#undef PDHG_SHADED
#undef PDHG_MCR
#undef PDHG_IGNORE_UV
#undef PDHG_RESTRICT_UV
