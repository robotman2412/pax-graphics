
#include "pax_internal.c"

void ${namespace}_rect_${variant}(${params}) {
	
	${pre}
	
	// Fix width and height.
	if (width < 0) {
		x += width;
		width = -width;
		${flip_x}
	}
	if (height < 0) {
		y += height;
		height = -height;
		${flip_y}
	}
	
	// Clip rect in inside of buffer.
	if (x < buf->clip.x) {
		float part = (buf->clip.x - x) / width;
		$[each x0,x1 in interp_x][code
			${x0} = ${x0} + (${x1} - ${x0}) * part;
		]
		
		width -= buf->clip.x - x;
		x = buf->clip.x;
	}
	if (x + width > buf->clip.x + buf->clip.w) {
		float part = (buf->clip.x + buf->clip.w - x) / width;
		$[each x0,x1 in interp_x][code
			${x1} = ${x0} + (${x1} - ${x0}) * part;
		]
		
		width = buf->clip.x + buf->clip.w - x;
	}
	if (y < buf->clip.y) {
		float part = (buf->clip.y - y) / height;
		$[each y0,y1 in interp_y][code
			${y0} = ${y0} + (${y1} - ${y0}) * part;
		]
		
		height -= buf->clip.y - y;
		y = buf->clip.y;
	}
	if (y + height > buf->clip.y + buf->clip.h) {
		float part = (buf->clip.y + buf->clip.h - y) / height;
		$[each y0,y1 in interp_y][code
			${y1} = ${y0} + (${y1} - ${y0}) * part;
		]
		
		height = buf->clip.y + buf->clip.h - y;
	}
	
	// Adjust UVs to match pixel co-ordinates.
	float min_x = (int) (x + 0.5)          + 0.5;
	float max_x = (int) (x + width - 0.5)  + 0.5;
	float min_y = (int) (y + 0.5)          + 0.5;
	float max_y = (int) (y + height - 0.5) + 0.5;
	{ // Adjust the X part.
		$[each x0,x1 in interp_x][code
			float new_${x0} = ${x0} + (${x1} - ${x0}) / width * (min_x - x);
			float new_${x1} = ${x0} + (${x1} - ${x0}) / width * (max_x - x);
		]
		$[each x0,x1 in interp_x][code
			${x0} = new_${x0};
			${x1} = new_${x1};
		]
	}
	{ // Adjust the Y part.
		$[each y0,y1 in interp_y][code
			float new_${y0} = ${y0} + (${y1} - ${y0}) / height * (min_y - y);
			float new_${y1} = ${y0} + (${y1} - ${y0}) / height * (max_y - y);
		]
		$[each y0,y1 in interp_y][code
			${y0} = new_${y0};
			${y1} = new_${y1};
		]
	}
	
	// Find UV deltas.
	$[each x0,x1 in interp_x][code
		float ${x0}_${x1}_d = (${x1} - ${x0}) / (max_x - min_x);
	]
	$[each y0,y1 in interp_y][code
		float ${y0}_${y1}_d = (${y1} - ${y0}) / (max_y - min_y);
	]
	
	$[each y0,y1,y in interp_y][code
		float ${y} = ${y0};
	]
	
	float u0_u1_d = (u1 - u0) / (max_x - min_x);
	float v0_v1_d = (v1 - v0) / (max_y - min_y);
	
	float v = v0;
	
	int c_y = y + 0.5;
	$[if mcr][code
		// Snap c_y to the correct line.
		if ((c_y & 1) != odd_scanline) {
			c_y ++;
			$[each y0,y1,y in interp_y][code
				${y} += ${y0}_${y1}_d;
			]
		}
	]
	
	// Pixel time.
	int delta = c_y * buf->width;
	for (; c_y <= y + height - 0.5; c_y += ${inc_y}) {
		$[each x0,x1,x in interp_x][code
			float ${x} = ${x0};
		]
		for (int c_x = x + 0.5; c_x <= x + width - 0.5; c_x ++) {
			${pixel_update}
			$[each x0,x1,x in interp_x][code
				${x} += ${x0}_${x1}_d;
			]
		}
		$[each in y0,y1,y][code
			${y} += ${inc_y}*${y0}_${y1}_d;
		]
		delta += ${inc_y}*buf->width;
	}
	
	${post}
	
}
