
#include "pax_internal.h"

// Generic rectangle drawing code, assuming some defines are made.
static void PDHG_NAME(pax_buf_t *buf, pax_col_t color, float x, float y, float width, float height) {
	
	// pax_setter_t setter = color >= 0xff000000 ? pax_set_pixel : pax_merge_pixel;
	pax_col_t fill_color = color;
	pax_index_setter_t setter = pax_get_setter(buf, &fill_color, NULL);
	if (!setter) return;
	
	if (pax_enable_shape_aa) {
		bool do_right_edge = true;
		bool do_down_edge  = true;
		
		// Fix the edges according to ((antialiasing))
		float left_alpha = x - (int) x;
		if (left_alpha < 0) left_alpha ++;
		left_alpha = 1 - left_alpha;
		
		float right_alpha = (x + width) - (int) (x + width);
		if (right_alpha < 0) right_alpha ++;
		
		float up_alpha = y - (int) y;
		if (up_alpha < 0) up_alpha ++;
		up_alpha = 1 - up_alpha;
		
		float down_alpha = (y + height) - (int) (y + height);
		if (down_alpha < 0) down_alpha ++;
		
		// A fix for the thin line cases
		if ((int) x == (int) (x + width)) {
			do_right_edge = false;
			left_alpha = width;
		}
		if ((int) y == (int) (y + height)) {
			do_down_edge = false;
			up_alpha = height;
		}
		// if (!do_right_edge && !do_down_edge) {
			
		// } else {
		// 	if (!do_right_edge) {
				
		// 	}
		// 	if (!do_down_edge) {
				
		// 	}
		// }
		
		// Calculate edge colors.
		pax_col_t up_color    = pax_col_reduce_alpha(color, up_alpha);
		pax_col_t down_color  = pax_col_reduce_alpha(color, down_alpha);
		pax_col_t left_color  = pax_col_reduce_alpha(color, left_alpha);
		pax_col_t right_color = pax_col_reduce_alpha(color, right_alpha);
		
		// Colculate corner colors.
		pax_col_t up_left_color    = pax_col_reduce_alpha(color, up_alpha * left_alpha);
		pax_col_t up_right_color   = pax_col_reduce_alpha(color, up_alpha * right_alpha);
		pax_col_t down_left_color  = pax_col_reduce_alpha(color, down_alpha * left_alpha);
		pax_col_t down_right_color = pax_col_reduce_alpha(color, down_alpha * right_alpha);
		
		// Plot corners.
		pax_merge_pixel(buf, up_left_color,    x,         y);
		if (do_right_edge)
			pax_merge_pixel(buf, up_right_color,   x + width, y);
		if (do_down_edge)
			pax_merge_pixel(buf, down_left_color,  x,         y + height);
		if (do_right_edge && do_down_edge)
			pax_merge_pixel(buf, down_right_color, x + width, y + height);
		
		// Fill top and bottom.
		pax_index_setter_t up_setter   = pax_get_setter(buf, &up_color,   NULL);
		pax_index_setter_t down_setter = pax_get_setter(buf, &down_color, NULL);
		int delta_top    = (int) y * buf->width;
		int delta_bottom = (int) (y + height) * buf->width;
		for (int c_x = x + 1; c_x <= x + width - 1; c_x ++) {
			if (up_setter)                   up_setter  (buf, up_color,   c_x+delta_top);
			if (down_setter && do_down_edge) down_setter(buf, down_color, c_x+delta_bottom);
		}
		
		// Fill left and right
		pax_index_setter_t left_setter  = pax_get_setter(buf, &left_color,  NULL);
		pax_index_setter_t right_setter = pax_get_setter(buf, &right_color, NULL);
		int delta_left  = (int) (y + 1) * buf->width + (int) x;
		int delta_right = (int) (y + 1) * buf->width + (int) (x + width);
		for (int c_y = y + 1; c_y <= y + height - 1; c_y ++) {
			if (left_setter)                   left_setter (buf, left_color,  delta_left);
			if (right_setter && do_right_edge) right_setter(buf, right_color, delta_right);
			delta_left  += buf->width;
			delta_right += buf->width;
		}
		
		// Fill center.
		int delta = (int) (y + 1) * buf->width;
		for (int c_y = y + 1; c_y <= y + height - 1; c_y ++) {
			for (int c_x = x + 1; c_x <= x + width - 1; c_x ++) {
				setter(buf, fill_color, c_x+delta);
			}
			delta += buf->width;
		}
		
	} else {
		// Pixel time.
		int delta = (int) (y + 0.5) * buf->width;
		for (int c_y = y + 0.5; c_y <= y + height - 0.5; c_y ++) {
			for (int c_x = x + 0.5; c_x <= x + width - 0.5; c_x ++) {
				setter(buf, fill_color, c_x+delta);
			}
			delta += buf->width;
		}
	}
}

// Clean up macroation.
#undef PDHG_NAME

