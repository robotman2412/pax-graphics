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
#include "pax_fixpt.hpp"

#ifdef PDHG_NAME

#if defined(PDHG_SHADED) && !defined(PDHG_IGNORE_UV)
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

#ifdef PDHG_RESTRICT_UV
#error "Cannot define `PDHG_RESTRICT_UV` for triangles."
#endif

#if defined(PDHG_IGNORE_UV) && defined(PDHG_MCR)
#define PDHG_TRAPEZOID_NAME paxmcr_trapezoid_shaded_nouv
#elif defined(PDHG_SHADED) && defined(PDHG_MCR)
#define PDHG_TRAPEZOID_NAME paxmcr_trapezoid_shaded
#elif defined(PDHG_IGNORE_UV)
#define PDHG_TRAPEZOID_NAME pax_trapezoid_shaded_nouv
#elif defined(PDHG_SHADED)
#define PDHG_TRAPEZOID_NAME pax_trapezoid_shaded
#elif defined(PDHG_MCR)
#define PDHG_TRAPEZOID_NAME paxmcr_trapezoid_unshaded
#else
#define PDHG_TRAPEZOID_NAME pax_trapezoid_unshaded
#endif



// Trapezoid function, a triangle is made out of two of these.
// Assumes y0 < y1.
static void PDHG_TRAPEZOID_NAME(
#ifdef PDHG_MCR
		bool odd_scanline,
#endif
		pax_buf_t *buf, pax_col_t color,
#ifdef PDHG_SHADED
		const pax_shader_t *shader,
#endif
		fixpt_t x0a, fixpt_t x0b, fixpt_t y0, fixpt_t x1a, fixpt_t x1b, fixpt_t y1
#ifdef PDHG_NORMAL_UV
		, fixpt_t u0a, fixpt_t v0a, fixpt_t u0b, fixpt_t v0b, fixpt_t u1a, fixpt_t v1a, fixpt_t u1b, fixpt_t v1b
#endif
		) {
	
	#ifdef PDHG_SHADED
	// Get shader context.
	pax_shader_ctx_t shader_ctx = pax_get_shader_ctx(buf, color, shader);
	if (shader_ctx.skip) return;
	pax_col_conv_t buf2col = PAX_IS_PALETTE(buf->type) ? pax_col_conv_dummy : buf->buf2col;
	#else
	// Get pixel setter.
	pax_index_setter_t setter = pax_get_setter(buf, &color, NULL);
	if (!setter) return;
	#endif
	
	// Determine vertical bounds.
	int iy0 = y0 + 0.5;
	int iy1 = y1 + 0.5;
	if (iy0 >= iy1) return;
	
	// Sort points by X.
	if (x0a > x0b || x1a > x1b) {
		PAX_SWAP(fixpt_t, x0a, x0b);
		PAX_SWAP(fixpt_t, x1a, x1b);
		#ifdef PDHG_NORMAL_UV
		PAX_SWAP(fixpt_t, u0a, u0b);
		PAX_SWAP(fixpt_t, u1a, u1b);
		PAX_SWAP(fixpt_t, v0a, v0b);
		PAX_SWAP(fixpt_t, v1a, v1b);
		#endif
	}
	
	// Clip: Y axis.
	if (iy0 < buf->clip.y) {
		iy0 = buf->clip.y;
	}
	if (iy0 >= buf->clip.y + buf->clip.h) {
		return;
	}
	if (iy1 < buf->clip.y) {
		return;
	}
	if (iy1 >= buf->clip.y + buf->clip.h) {
		iy1 = buf->clip.y + buf->clip.h - 1;
	}
	
	#ifdef PDHG_MCR
	// Snap y to correct scanline.
	if ((iy0 & 1) != odd_scanline) {
		iy0 ++;
	}
	#endif
	
	// Determine X deltas.
	fixpt_t x0a_x1a_dx = (x1a - x0a) / (y1 - y0);
	fixpt_t x0b_x1b_dx = (x1b - x0b) / (y1 - y0);
	
	#ifdef PDHG_NORMAL_UV
	// Determine UV deltas.
	fixpt_t u0a_u1a_du = (u1a - u0a) / (y1 - y0);
	fixpt_t u0b_u1b_du = (u1b - u0b) / (y1 - y0);
	fixpt_t v0a_v1a_dv = (v1a - v0a) / (y1 - y0);
	fixpt_t v0b_v1b_dv = (v1b - v0b) / (y1 - y0);
	#endif
	
	// Initial X interpolation.
	fixpt_t coeff = (iy0 + 0.5f) - y0;
	fixpt_t x_a = x0a + x0a_x1a_dx * coeff;
	fixpt_t x_b = x0b + x0b_x1b_dx * coeff;
	
	#ifdef PDHG_NORMAL_UV
	// Initial UV interpolation.
	fixpt_t u_a = u0a + u0a_u1a_du * coeff;
	fixpt_t u_b = u0b + u0b_u1b_du * coeff;
	fixpt_t v_a = v0a + v0a_v1a_dv * coeff;
	fixpt_t v_b = v0b + v0b_v1b_dv * coeff;
	#endif
	
	// Vertical drawing loop.
	int delta = buf->width * iy0;
	for (int y = iy0; y < iy1; y += PDHG_INCREMENT) {
		int ixa = x_a + 0.5, ixb = x_b + 0.5;
		
		// Clip: X axis.
		if (ixa < buf->clip.x) {
			ixa = buf->clip.x;
		}
		if (ixa > buf->clip.x + buf->clip.w) {
			ixa = buf->clip.x + buf->clip.w;
		}
		if (ixb < buf->clip.x) {
			ixb = buf->clip.x;
		}
		if (ixb > buf->clip.x + buf->clip.w) {
			ixb = buf->clip.x + buf->clip.w;
		}
		
		#ifdef PDHG_NORMAL_UV
		// Determine UV deltas.
		fixpt_t du = (u_b - u_a) / (x_b - x_a);
		fixpt_t dv = (v_b - v_a) / (x_b - x_a);
		
		// Interpolate UV.
		coeff = (ixa + 0.5) - x_a;
		fixpt_t u = u_a + du * coeff;
		fixpt_t v = v_a + dv * coeff;
		#endif
		
		// Horizontal drawing loop.
		for (int x = ixa; x < ixb; x ++) {
			#ifdef PDHG_NORMAL_UV
			// Apply the shader,
			pax_col_t result = (shader_ctx.callback)(color, shader_ctx.do_getter ? buf2col(buf, buf->getter(buf, x+delta)) : 0, x, y, u, v, shader_ctx.callback_args);
			// And simply merge colors accordingly.
			pax_set_index_conv(buf, result, x+delta);
			u += du;
			v += dv;
			#elif defined(PDHG_SHADED)
			// Apply the shader,
			pax_col_t result = (shader_ctx.callback)(color, shader_ctx.do_getter ? buf2col(buf, buf->getter(buf, x+delta)) : 0, x, y, 0, 0, shader_ctx.callback_args);
			// And simply merge colors accordingly.
			pax_set_index_conv(buf, result, x+delta);
			#else
			// And simply merge colors accordingly.
			setter(buf, color, x+delta);
			#endif
		}
		
		#ifdef PDHG_NORMAL_UV
		// Interpolate UVs.
		u_a += PDHG_INCREMENT*u0a_u1a_du;
		u_b += PDHG_INCREMENT*u0b_u1b_du;
		v_a += PDHG_INCREMENT*v0a_v1a_dv;
		v_b += PDHG_INCREMENT*v0b_v1b_dv;
		#endif
		
		// Interpolate X.
		x_a += PDHG_INCREMENT*x0a_x1a_dx;
		x_b += PDHG_INCREMENT*x0b_x1b_dx;
		
		delta += PDHG_INCREMENT*buf->width;
	}
}



// Generic triangle drawing code, assuming some defines are made.
#ifdef PDHG_STATIC
static
#endif
void PDHG_NAME (
#ifdef PDHG_MCR
		bool odd_scanline,
#endif
		pax_buf_t *buf, pax_col_t color,
#ifdef PDHG_SHADED
		const pax_shader_t *shader,
#endif
		float _x0, float _y0, float _x1, float _y1, float _x2, float _y2
#ifdef PDHG_NORMAL_UV
		, float _u0, float _v0, float _u1, float _v1, float _u2, float _v2
#endif
		) {
	
	fixpt_t x0 = _x0;
	fixpt_t y0 = _y0;
	fixpt_t x1 = _x1;
	fixpt_t y1 = _y1;
	fixpt_t x2 = _x2;
	fixpt_t y2 = _y2;
	#ifdef PDHG_NORMAL_UV
	fixpt_t u0 = _u0;
	fixpt_t v0 = _v0;
	fixpt_t u1 = _u1;
	fixpt_t v1 = _v1;
	fixpt_t u2 = _u2;
	fixpt_t v2 = _v2;
	#endif
	
	// Sort points by height.
	if (y1 < y0) {
		PAX_SWAP_POINTS(x0, y0, x1, y1);
		#ifdef PDHG_NORMAL_UV
		PAX_SWAP_POINTS(u0, v0, u1, v1);
		#endif
	}
	if (y2 < y0) {
		PAX_SWAP_POINTS(x0, y0, x2, y2);
		#ifdef PDHG_NORMAL_UV
		PAX_SWAP_POINTS(u0, v0, u2, v2);
		#endif
	}
	if (y2 < y1) {
		PAX_SWAP_POINTS(x1, y1, x2, y2);
		#ifdef PDHG_NORMAL_UV
		PAX_SWAP_POINTS(u1, v1, u2, v2);
		#endif
	}
	
	// Interpolate coordinates.
	fixpt_t coeff = (y1 - y0) / (y2 - y0);
	fixpt_t x1b = x0 + (x2 - x0) * coeff;
	#ifdef PDHG_NORMAL_UV
	fixpt_t u1b = u0 + (u2 - u0) * coeff;
	fixpt_t v1b = v0 + (v2 - v0) * coeff;
	#endif
	
	// Top half.
	PDHG_TRAPEZOID_NAME(
		#ifdef PDHG_MCR
		odd_scanline,
		#endif
		buf, color,
		#ifdef PDHG_SHADED
		shader,
		#endif
		x0, x0, y0, x1, x1b, y1
		#ifdef PDHG_NORMAL_UV
		, u0, v0, u0, v0, u1, v1, u1b, v1b
		#endif
	);
	// Bottom half.
	PDHG_TRAPEZOID_NAME(
		#ifdef PDHG_MCR
		odd_scanline,
		#endif
		buf, color,
		#ifdef PDHG_SHADED
		shader,
		#endif
		x1, x1b, y1, x2, x2, y2
		#ifdef PDHG_NORMAL_UV
		, u1, u1b, v1, v1b, u2, v2, u2, v2
		#endif
	);
}



#endif // PDHG_NAME

// Clean up macros.
#undef PDHG_INCREMENT
#undef PDHG_NORMAL_UV
#undef PDHG_TRAPEZOID_NAME

// Clean up parameter macros.
#undef PDHG_STATIC
#undef PDHG_NAME
#undef PDHG_SHADED
#undef PDHG_MCR
#undef PDHG_IGNORE_UV
#undef PDHG_RESTRICT_UV
