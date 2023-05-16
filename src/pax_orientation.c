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

#include "pax_orientation.h"



// Transforms the co-ordinates as 1x counter-clockwise rotation.
static inline pax_vec2f pax_orient_ccw1_vec2f(const pax_buf_t *buf, pax_vec2f vec) __attribute__((pure));
static inline pax_vec2f pax_orient_ccw1_vec2f(const pax_buf_t *buf, pax_vec2f vec) {
	return (pax_vec2f) {
		vec.y,
		buf->height - vec.x,
	};
}

// Transforms the co-ordinates as 2x counter-clockwise rotation.
static inline pax_vec2f pax_orient_ccw2_vec2f(const pax_buf_t *buf, pax_vec2f vec) __attribute__((pure));
static inline pax_vec2f pax_orient_ccw2_vec2f(const pax_buf_t *buf, pax_vec2f vec) {
	return (pax_vec2f) {
		buf->width  - vec.x,
		buf->height - vec.y,
	};
}

// Transforms the co-ordinates as 2x counter-clockwise rotation.
static inline pax_vec2f pax_orient_ccw3_vec2f(const pax_buf_t *buf, pax_vec2f vec) __attribute__((pure));
static inline pax_vec2f pax_orient_ccw3_vec2f(const pax_buf_t *buf, pax_vec2f vec) {
	return (pax_vec2f) {
		buf->width - vec.y,
		vec.x,
	};
}

// Transforms the co-ordinates as flip horizontally.
static inline pax_vec2f pax_orient_flip_vec2f(const pax_buf_t *buf, pax_vec2f vec) __attribute__((pure));
static inline pax_vec2f pax_orient_flip_vec2f(const pax_buf_t *buf, pax_vec2f vec) {
	return (pax_vec2f) {
		buf->width  - vec.x,
		vec.y,
	};
}

// Transforms the co-ordinates as 1x counter-clockwise rotation and flip horizontally.
static inline pax_vec2f pax_orient_ccw1_flip_vec2f(const pax_buf_t *buf, pax_vec2f vec) __attribute__((pure));
static inline pax_vec2f pax_orient_ccw1_flip_vec2f(const pax_buf_t *buf, pax_vec2f vec) {
	return (pax_vec2f) {
		buf->width  - vec.y,
		buf->height - vec.x,
	};
}

// Transforms the co-ordinates as 2x counter-clockwise rotation and flip horizontally.
static inline pax_vec2f pax_orient_ccw2_flip_vec2f(const pax_buf_t *buf, pax_vec2f vec) __attribute__((pure));
static inline pax_vec2f pax_orient_ccw2_flip_vec2f(const pax_buf_t *buf, pax_vec2f vec) {
	return (pax_vec2f) {
		vec.x,
		buf->height - vec.y,
	};
}

// Transforms the co-ordinates as 2x counter-clockwise rotation and flip horizontally.
static inline pax_vec2f pax_orient_ccw3_flip_vec2f(const pax_buf_t *buf, pax_vec2f vec) __attribute__((pure));
static inline pax_vec2f pax_orient_ccw3_flip_vec2f(const pax_buf_t *buf, pax_vec2f vec) {
	return (pax_vec2f) {
		vec.y,
		vec.x,
	};
}

// Detects orientation and transforms co-ordinates accordingly.
pax_vec2f pax_orient_det_vec2f(const pax_buf_t *buf, pax_vec2f vec) {
	#if PAX_COMPILE_ORIENTATION
	switch (buf->orientation) {
		default:
		case PAX_O_UPRIGHT:			return vec;
		case PAX_O_ROT_CCW:			return pax_orient_ccw1_vec2f(buf, vec);
		case PAX_O_ROT_HALF:		return pax_orient_ccw2_vec2f(buf, vec);
		case PAX_O_ROT_CW:			return pax_orient_ccw3_vec2f(buf, vec);
		case PAX_O_FLIP_H:			return pax_orient_flip_vec2f(buf, vec);
		case PAX_O_ROT_CCW_FLIP_H:	return pax_orient_ccw1_flip_vec2f(buf, vec);
		case PAX_O_ROT_HALF_FLIP_H:	return pax_orient_ccw2_flip_vec2f(buf, vec);
		case PAX_O_ROT_CW_FLIP_H:	return pax_orient_ccw3_flip_vec2f(buf, vec);
	}
	#else
	return vec;
	#endif
}

// Detects orientation and transforms co-ordinates accordingly.
pax_vec2f pax_unorient_det_vec2f(const pax_buf_t *buf, pax_vec2f vec) {
	#if PAX_COMPILE_ORIENTATION
	switch (buf->orientation) {
		default:
		case PAX_O_UPRIGHT:			return vec;
		case PAX_O_ROT_CCW:			return pax_orient_ccw3_vec2f(buf, vec);
		case PAX_O_ROT_HALF:		return pax_orient_ccw2_vec2f(buf, vec);
		case PAX_O_ROT_CW:			return pax_orient_ccw1_vec2f(buf, vec);
		case PAX_O_FLIP_H:			return pax_orient_flip_vec2f(buf, vec);
		case PAX_O_ROT_CCW_FLIP_H:	return pax_orient_ccw1_flip_vec2f(buf, vec);
		case PAX_O_ROT_HALF_FLIP_H:	return pax_orient_ccw2_flip_vec2f(buf, vec);
		case PAX_O_ROT_CW_FLIP_H:	return pax_orient_ccw3_flip_vec2f(buf, vec);
	}
	#else
	return vec;
	#endif
}


// Transforms the co-ordinates as 1x counter-clockwise rotation.
static inline pax_rectf pax_orient_ccw1_rectf(const pax_buf_t *buf, pax_rectf vec) __attribute__((pure));
static inline pax_rectf pax_orient_ccw1_rectf(const pax_buf_t *buf, pax_rectf vec) {
	return (pax_rectf) {
		vec.y,
		buf->height - vec.x,
		vec.h,
		-vec.w,
	};
}

// Transforms the co-ordinates as 2x counter-clockwise rotation.
static inline pax_rectf pax_orient_ccw2_rectf(const pax_buf_t *buf, pax_rectf vec) __attribute__((pure));
static inline pax_rectf pax_orient_ccw2_rectf(const pax_buf_t *buf, pax_rectf vec) {
	return (pax_rectf) {
		buf->width  - vec.x,
		buf->height - vec.y,
		-vec.w,
		-vec.h,
	};
}

// Transforms the co-ordinates as 2x counter-clockwise rotation.
static inline pax_rectf pax_orient_ccw3_rectf(const pax_buf_t *buf, pax_rectf vec) __attribute__((pure));
static inline pax_rectf pax_orient_ccw3_rectf(const pax_buf_t *buf, pax_rectf vec) {
	return (pax_rectf) {
		buf->width - vec.y,
		vec.x,
		-vec.h,
		vec.w,
	};
}

// Transforms the co-ordinates as flip horizontally.
static inline pax_rectf pax_orient_flip_rectf(const pax_buf_t *buf, pax_rectf vec) __attribute__((pure));
static inline pax_rectf pax_orient_flip_rectf(const pax_buf_t *buf, pax_rectf vec) {
	return (pax_rectf) {
		buf->width  - vec.x,
		buf->height - vec.y,
		-vec.w,
		vec.h,
	};
}

// Transforms the co-ordinates as 1x counter-clockwise rotation and flip horizontally.
static inline pax_rectf pax_orient_ccw1_flip_rectf(const pax_buf_t *buf, pax_rectf vec) __attribute__((pure));
static inline pax_rectf pax_orient_ccw1_flip_rectf(const pax_buf_t *buf, pax_rectf vec) {
	return (pax_rectf) {
		buf->width  - vec.y,
		buf->height - vec.x,
		-vec.h,
		-vec.w,
	};
}

// Transforms the co-ordinates as 2x counter-clockwise rotation and flip horizontally.
static inline pax_rectf pax_orient_ccw2_flip_rectf(const pax_buf_t *buf, pax_rectf vec) __attribute__((pure));
static inline pax_rectf pax_orient_ccw2_flip_rectf(const pax_buf_t *buf, pax_rectf vec) {
	return (pax_rectf) {
		vec.x,
		buf->height - vec.y,
		vec.w,
		-vec.h,
	};
}

// Transforms the co-ordinates as 2x counter-clockwise rotation and flip horizontally.
static inline pax_rectf pax_orient_ccw3_flip_rectf(const pax_buf_t *buf, pax_rectf vec) __attribute__((pure));
static inline pax_rectf pax_orient_ccw3_flip_rectf(const pax_buf_t *buf, pax_rectf vec) {
	return (pax_rectf) {
		vec.y,
		vec.x,
		vec.h,
		vec.w,
	};
}

// Detects orientation and transforms co-ordinates accordingly.
pax_rectf pax_orient_det_rectf(const pax_buf_t *buf, pax_rectf vec) {
	#if PAX_COMPILE_ORIENTATION
	switch (buf->orientation) {
		default:
		case PAX_O_UPRIGHT:			return vec;
		case PAX_O_ROT_CCW:			return pax_orient_ccw1_rectf(buf, vec);
		case PAX_O_ROT_HALF:		return pax_orient_ccw2_rectf(buf, vec);
		case PAX_O_ROT_CW:			return pax_orient_ccw3_rectf(buf, vec);
		case PAX_O_FLIP_H:			return pax_orient_flip_rectf(buf, vec);
		case PAX_O_ROT_CCW_FLIP_H:	return pax_orient_ccw1_flip_rectf(buf, vec);
		case PAX_O_ROT_HALF_FLIP_H:	return pax_orient_ccw2_flip_rectf(buf, vec);
		case PAX_O_ROT_CW_FLIP_H:	return pax_orient_ccw3_flip_rectf(buf, vec);
	}
	#else
	return vec;
	#endif
}

// Detects orientation and transforms co-ordinates accordingly.
pax_rectf pax_unorient_det_rectf(const pax_buf_t *buf, pax_rectf vec) {
	#if PAX_COMPILE_ORIENTATION
	switch (buf->orientation) {
		default:
		case PAX_O_UPRIGHT:			return vec;
		case PAX_O_ROT_CCW:			return pax_orient_ccw3_rectf(buf, vec);
		case PAX_O_ROT_HALF:		return pax_orient_ccw2_rectf(buf, vec);
		case PAX_O_ROT_CW:			return pax_orient_ccw1_rectf(buf, vec);
		case PAX_O_FLIP_H:			return pax_orient_flip_rectf(buf, vec);
		case PAX_O_ROT_CCW_FLIP_H:	return pax_orient_ccw1_flip_rectf(buf, vec);
		case PAX_O_ROT_HALF_FLIP_H:	return pax_orient_ccw2_flip_rectf(buf, vec);
		case PAX_O_ROT_CW_FLIP_H:	return pax_orient_ccw3_flip_rectf(buf, vec);
	}
	#else
	return vec;
	#endif
}


// Transforms the co-ordinates as 1x counter-clockwise rotation.
static inline pax_vec2i pax_orient_ccw1_vec2i(const pax_buf_t *buf, pax_vec2i vec) __attribute__((pure));
static inline pax_vec2i pax_orient_ccw1_vec2i(const pax_buf_t *buf, pax_vec2i vec) {
	return (pax_vec2i) {
		vec.y,
		buf->height - 1 - vec.x,
	};
}

// Transforms the co-ordinates as 2x counter-clockwise rotation.
static inline pax_vec2i pax_orient_ccw2_vec2i(const pax_buf_t *buf, pax_vec2i vec) __attribute__((pure));
static inline pax_vec2i pax_orient_ccw2_vec2i(const pax_buf_t *buf, pax_vec2i vec) {
	return (pax_vec2i) {
		buf->width  - 1 - vec.x,
		buf->height - 1 - vec.y,
	};
}

// Transforms the co-ordinates as 2x counter-clockwise rotation.
static inline pax_vec2i pax_orient_ccw3_vec2i(const pax_buf_t *buf, pax_vec2i vec) __attribute__((pure));
static inline pax_vec2i pax_orient_ccw3_vec2i(const pax_buf_t *buf, pax_vec2i vec) {
	return (pax_vec2i) {
		buf->width - 1 - vec.y,
		vec.x,
	};
}

// Transforms the co-ordinates as 1x counter-clockwise rotation.
static inline pax_vec2i pax_orient_flip_vec2i(const pax_buf_t *buf, pax_vec2i vec) __attribute__((pure));
static inline pax_vec2i pax_orient_flip_vec2i(const pax_buf_t *buf, pax_vec2i vec) {
	return (pax_vec2i) {
		buf->width  - 1 - vec.x,
		vec.y,
	};
}

// Transforms the co-ordinates as 1x counter-clockwise rotation.
static inline pax_vec2i pax_orient_ccw1_flip_vec2i(const pax_buf_t *buf, pax_vec2i vec) __attribute__((pure));
static inline pax_vec2i pax_orient_ccw1_flip_vec2i(const pax_buf_t *buf, pax_vec2i vec) {
	return (pax_vec2i) {
		buf->width  - 1 - vec.y,
		buf->height - 1 - vec.x,
	};
}

// Transforms the co-ordinates as 2x counter-clockwise rotation.
static inline pax_vec2i pax_orient_ccw2_flip_vec2i(const pax_buf_t *buf, pax_vec2i vec) __attribute__((pure));
static inline pax_vec2i pax_orient_ccw2_flip_vec2i(const pax_buf_t *buf, pax_vec2i vec) {
	return (pax_vec2i) {
		vec.x,
		buf->height - 1 - vec.y,
	};
}

// Transforms the co-ordinates as 2x counter-clockwise rotation.
static inline pax_vec2i pax_orient_ccw3_flip_vec2i(const pax_buf_t *buf, pax_vec2i vec) __attribute__((pure));
static inline pax_vec2i pax_orient_ccw3_flip_vec2i(const pax_buf_t *buf, pax_vec2i vec) {
	return (pax_vec2i) {
		vec.y,
		vec.x,
	};
}

// Detects orientation and transforms co-ordinates accordingly.
pax_vec2i pax_orient_det_vec2i(const pax_buf_t *buf, pax_vec2i vec) {
	#if PAX_COMPILE_ORIENTATION
	switch (buf->orientation) {
		default:
		case PAX_O_UPRIGHT:			return vec;
		case PAX_O_ROT_CCW:			return pax_orient_ccw1_vec2i(buf, vec);
		case PAX_O_ROT_HALF:		return pax_orient_ccw2_vec2i(buf, vec);
		case PAX_O_ROT_CW:			return pax_orient_ccw3_vec2i(buf, vec);
		case PAX_O_FLIP_H:			return pax_orient_flip_vec2i(buf, vec);
		case PAX_O_ROT_CCW_FLIP_H:	return pax_orient_ccw1_flip_vec2i(buf, vec);
		case PAX_O_ROT_HALF_FLIP_H:	return pax_orient_ccw2_flip_vec2i(buf, vec);
		case PAX_O_ROT_CW_FLIP_H:	return pax_orient_ccw3_flip_vec2i(buf, vec);
	}
	#else
	return vec;
	#endif
}

// Detects orientation and transforms co-ordinates accordingly.
pax_vec2i pax_unorient_det_vec2i(const pax_buf_t *buf, pax_vec2i vec) {
	#if PAX_COMPILE_ORIENTATION
	switch (buf->orientation) {
		default:
		case PAX_O_UPRIGHT:			return vec;
		case PAX_O_ROT_CCW:			return pax_orient_ccw3_vec2i(buf, vec);
		case PAX_O_ROT_HALF:		return pax_orient_ccw2_vec2i(buf, vec);
		case PAX_O_ROT_CW:			return pax_orient_ccw1_vec2i(buf, vec);
		case PAX_O_FLIP_H:			return pax_orient_flip_vec2i(buf, vec);
		case PAX_O_ROT_CCW_FLIP_H:	return pax_orient_ccw1_flip_vec2i(buf, vec);
		case PAX_O_ROT_HALF_FLIP_H:	return pax_orient_ccw2_flip_vec2i(buf, vec);
		case PAX_O_ROT_CW_FLIP_H:	return pax_orient_ccw3_flip_vec2i(buf, vec);
	}
	#else
	return vec;
	#endif
}
