
#include "pax_internal.c"

void ${namespace}_rect_${variant}(${params}) {
	
	${pre}
	
	$[if mcr
		// MCR enabled.
	]
	
	// ${inc_y}
	
	$[each x0,x1 in interp_x
		// ${x0}
		// ${x1}
	]
	
	$[each y0,y1,y in interp_y
		// ${y0}
		// ${y1}
		// ${y}
	]
	
	${update_pixel}
	
	${post}
	
}
