
#include "pax_internal.h"

// Generic rectangle drawing code, assuming some defines are made.
void PDHG_NAME(pax_buf_t *buf, pax_col_t color, float x, float y, float width, float height) {
	pax_col_t fill_color = color;
	pax_index_setter_t setter = pax_get_setter(buf, &fill_color, NULL);
	if (!setter) return;
	
	// Pixel time.
	int delta = (int) (y + 0.5) * buf->width;
	for (int c_y = y + 0.5; c_y <= y + height - 0.5; c_y ++) {
		for (int c_x = x + 0.5; c_x <= x + width - 0.5; c_x ++) {
			setter(buf, fill_color, c_x+delta);
		}
		delta += buf->width;
	}
}

// Clean up macroation.
#undef PDHG_NAME

