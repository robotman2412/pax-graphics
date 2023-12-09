
// SPDX-License-Identifier: MIT

#include "pax_orientation.h"



// Transforms the co-ordinates as 1x counter-clockwise rotation.
static inline pax_vec2f pax_orient_ccw1_vec2f(pax_buf_t const *buf, pax_vec2f vec) __attribute__((pure));
static inline pax_vec2f pax_orient_ccw1_vec2f(pax_buf_t const *buf, pax_vec2f vec) {
    return (pax_vec2f){
        vec.y,
        buf->height - vec.x,
    };
}

// Transforms the co-ordinates as 2x counter-clockwise rotation.
static inline pax_vec2f pax_orient_ccw2_vec2f(pax_buf_t const *buf, pax_vec2f vec) __attribute__((pure));
static inline pax_vec2f pax_orient_ccw2_vec2f(pax_buf_t const *buf, pax_vec2f vec) {
    return (pax_vec2f){
        buf->width - vec.x,
        buf->height - vec.y,
    };
}

// Transforms the co-ordinates as 2x counter-clockwise rotation.
static inline pax_vec2f pax_orient_ccw3_vec2f(pax_buf_t const *buf, pax_vec2f vec) __attribute__((pure));
static inline pax_vec2f pax_orient_ccw3_vec2f(pax_buf_t const *buf, pax_vec2f vec) {
    return (pax_vec2f){
        buf->width - vec.y,
        vec.x,
    };
}

// Transforms the co-ordinates as flip horizontally.
static inline pax_vec2f pax_orient_flip_vec2f(pax_buf_t const *buf, pax_vec2f vec) __attribute__((pure));
static inline pax_vec2f pax_orient_flip_vec2f(pax_buf_t const *buf, pax_vec2f vec) {
    return (pax_vec2f){
        buf->width - vec.x,
        vec.y,
    };
}

// Transforms the co-ordinates as 1x counter-clockwise rotation and flip horizontally.
static inline pax_vec2f pax_orient_ccw1_flip_vec2f(pax_buf_t const *buf, pax_vec2f vec) __attribute__((pure));
static inline pax_vec2f pax_orient_ccw1_flip_vec2f(pax_buf_t const *buf, pax_vec2f vec) {
    return (pax_vec2f){
        buf->width - vec.y,
        buf->height - vec.x,
    };
}

// Transforms the co-ordinates as 2x counter-clockwise rotation and flip horizontally.
static inline pax_vec2f pax_orient_ccw2_flip_vec2f(pax_buf_t const *buf, pax_vec2f vec) __attribute__((pure));
static inline pax_vec2f pax_orient_ccw2_flip_vec2f(pax_buf_t const *buf, pax_vec2f vec) {
    return (pax_vec2f){
        vec.x,
        buf->height - vec.y,
    };
}

// Transforms the co-ordinates as 2x counter-clockwise rotation and flip horizontally.
static inline pax_vec2f pax_orient_ccw3_flip_vec2f(pax_buf_t const *buf, pax_vec2f vec) __attribute__((pure));
static inline pax_vec2f pax_orient_ccw3_flip_vec2f(pax_buf_t const *buf, pax_vec2f vec) {
    return (pax_vec2f){
        vec.y,
        vec.x,
    };
}

// Detects orientation and transforms co-ordinates accordingly.
pax_vec2f pax_orient_det_vec2f(pax_buf_t const *buf, pax_vec2f vec) {
#if PAX_COMPILE_ORIENTATION
    switch (buf->orientation) {
        default:
        case PAX_O_UPRIGHT: return vec;
        case PAX_O_ROT_CCW: return pax_orient_ccw1_vec2f(buf, vec);
        case PAX_O_ROT_HALF: return pax_orient_ccw2_vec2f(buf, vec);
        case PAX_O_ROT_CW: return pax_orient_ccw3_vec2f(buf, vec);
        case PAX_O_FLIP_H: return pax_orient_flip_vec2f(buf, vec);
        case PAX_O_ROT_CCW_FLIP_H: return pax_orient_ccw1_flip_vec2f(buf, vec);
        case PAX_O_ROT_HALF_FLIP_H: return pax_orient_ccw2_flip_vec2f(buf, vec);
        case PAX_O_ROT_CW_FLIP_H: return pax_orient_ccw3_flip_vec2f(buf, vec);
    }
#else
    return vec;
#endif
}

// Detects orientation and transforms co-ordinates accordingly.
pax_vec2f pax_unorient_det_vec2f(pax_buf_t const *buf, pax_vec2f vec) {
#if PAX_COMPILE_ORIENTATION
    switch (buf->orientation) {
        default:
        case PAX_O_UPRIGHT: return vec;
        case PAX_O_ROT_CCW: return pax_orient_ccw3_vec2f(buf, vec);
        case PAX_O_ROT_HALF: return pax_orient_ccw2_vec2f(buf, vec);
        case PAX_O_ROT_CW: return pax_orient_ccw1_vec2f(buf, vec);
        case PAX_O_FLIP_H: return pax_orient_flip_vec2f(buf, vec);
        case PAX_O_ROT_CCW_FLIP_H: return pax_orient_ccw1_flip_vec2f(buf, vec);
        case PAX_O_ROT_HALF_FLIP_H: return pax_orient_ccw2_flip_vec2f(buf, vec);
        case PAX_O_ROT_CW_FLIP_H: return pax_orient_ccw3_flip_vec2f(buf, vec);
    }
#else
    return vec;
#endif
}


// Transforms the co-ordinates as 1x counter-clockwise rotation.
static inline pax_rectf pax_orient_ccw1_rectf(pax_buf_t const *buf, pax_rectf vec) __attribute__((pure));
static inline pax_rectf pax_orient_ccw1_rectf(pax_buf_t const *buf, pax_rectf vec) {
    return (pax_rectf){
        vec.y,
        buf->height - vec.x,
        vec.h,
        -vec.w,
    };
}

// Transforms the co-ordinates as 2x counter-clockwise rotation.
static inline pax_rectf pax_orient_ccw2_rectf(pax_buf_t const *buf, pax_rectf vec) __attribute__((pure));
static inline pax_rectf pax_orient_ccw2_rectf(pax_buf_t const *buf, pax_rectf vec) {
    return (pax_rectf){
        buf->width - vec.x,
        buf->height - vec.y,
        -vec.w,
        -vec.h,
    };
}

// Transforms the co-ordinates as 2x counter-clockwise rotation.
static inline pax_rectf pax_orient_ccw3_rectf(pax_buf_t const *buf, pax_rectf vec) __attribute__((pure));
static inline pax_rectf pax_orient_ccw3_rectf(pax_buf_t const *buf, pax_rectf vec) {
    return (pax_rectf){
        buf->width - vec.y,
        vec.x,
        -vec.h,
        vec.w,
    };
}

// Transforms the co-ordinates as flip horizontally.
static inline pax_rectf pax_orient_flip_rectf(pax_buf_t const *buf, pax_rectf vec) __attribute__((pure));
static inline pax_rectf pax_orient_flip_rectf(pax_buf_t const *buf, pax_rectf vec) {
    return (pax_rectf){
        buf->width - vec.x,
        buf->height - vec.y,
        -vec.w,
        vec.h,
    };
}

// Transforms the co-ordinates as 1x counter-clockwise rotation and flip horizontally.
static inline pax_rectf pax_orient_ccw1_flip_rectf(pax_buf_t const *buf, pax_rectf vec) __attribute__((pure));
static inline pax_rectf pax_orient_ccw1_flip_rectf(pax_buf_t const *buf, pax_rectf vec) {
    return (pax_rectf){
        buf->width - vec.y,
        buf->height - vec.x,
        -vec.h,
        -vec.w,
    };
}

// Transforms the co-ordinates as 2x counter-clockwise rotation and flip horizontally.
static inline pax_rectf pax_orient_ccw2_flip_rectf(pax_buf_t const *buf, pax_rectf vec) __attribute__((pure));
static inline pax_rectf pax_orient_ccw2_flip_rectf(pax_buf_t const *buf, pax_rectf vec) {
    return (pax_rectf){
        vec.x,
        buf->height - vec.y,
        vec.w,
        -vec.h,
    };
}

// Transforms the co-ordinates as 2x counter-clockwise rotation and flip horizontally.
static inline pax_rectf pax_orient_ccw3_flip_rectf(pax_buf_t const *buf, pax_rectf vec) __attribute__((pure));
static inline pax_rectf pax_orient_ccw3_flip_rectf(pax_buf_t const *buf, pax_rectf vec) {
    return (pax_rectf){
        vec.y,
        vec.x,
        vec.h,
        vec.w,
    };
}

// Detects orientation and transforms co-ordinates accordingly.
pax_rectf pax_orient_det_rectf(pax_buf_t const *buf, pax_rectf vec) {
#if PAX_COMPILE_ORIENTATION
    switch (buf->orientation) {
        default:
        case PAX_O_UPRIGHT: return vec;
        case PAX_O_ROT_CCW: return pax_orient_ccw1_rectf(buf, vec);
        case PAX_O_ROT_HALF: return pax_orient_ccw2_rectf(buf, vec);
        case PAX_O_ROT_CW: return pax_orient_ccw3_rectf(buf, vec);
        case PAX_O_FLIP_H: return pax_orient_flip_rectf(buf, vec);
        case PAX_O_ROT_CCW_FLIP_H: return pax_orient_ccw1_flip_rectf(buf, vec);
        case PAX_O_ROT_HALF_FLIP_H: return pax_orient_ccw2_flip_rectf(buf, vec);
        case PAX_O_ROT_CW_FLIP_H: return pax_orient_ccw3_flip_rectf(buf, vec);
    }
#else
    return vec;
#endif
}

// Detects orientation and transforms co-ordinates accordingly.
pax_rectf pax_unorient_det_rectf(pax_buf_t const *buf, pax_rectf vec) {
#if PAX_COMPILE_ORIENTATION
    switch (buf->orientation) {
        default:
        case PAX_O_UPRIGHT: return vec;
        case PAX_O_ROT_CCW: return pax_orient_ccw3_rectf(buf, vec);
        case PAX_O_ROT_HALF: return pax_orient_ccw2_rectf(buf, vec);
        case PAX_O_ROT_CW: return pax_orient_ccw1_rectf(buf, vec);
        case PAX_O_FLIP_H: return pax_orient_flip_rectf(buf, vec);
        case PAX_O_ROT_CCW_FLIP_H: return pax_orient_ccw1_flip_rectf(buf, vec);
        case PAX_O_ROT_HALF_FLIP_H: return pax_orient_ccw2_flip_rectf(buf, vec);
        case PAX_O_ROT_CW_FLIP_H: return pax_orient_ccw3_flip_rectf(buf, vec);
    }
#else
    return vec;
#endif
}


// Transforms the co-ordinates as 1x counter-clockwise rotation.
static inline pax_vec2i pax_orient_ccw1_vec2i(pax_buf_t const *buf, pax_vec2i vec) __attribute__((pure));
static inline pax_vec2i pax_orient_ccw1_vec2i(pax_buf_t const *buf, pax_vec2i vec) {
    return (pax_vec2i){
        vec.y,
        buf->height - 1 - vec.x,
    };
}

// Transforms the co-ordinates as 2x counter-clockwise rotation.
static inline pax_vec2i pax_orient_ccw2_vec2i(pax_buf_t const *buf, pax_vec2i vec) __attribute__((pure));
static inline pax_vec2i pax_orient_ccw2_vec2i(pax_buf_t const *buf, pax_vec2i vec) {
    return (pax_vec2i){
        buf->width - 1 - vec.x,
        buf->height - 1 - vec.y,
    };
}

// Transforms the co-ordinates as 2x counter-clockwise rotation.
static inline pax_vec2i pax_orient_ccw3_vec2i(pax_buf_t const *buf, pax_vec2i vec) __attribute__((pure));
static inline pax_vec2i pax_orient_ccw3_vec2i(pax_buf_t const *buf, pax_vec2i vec) {
    return (pax_vec2i){
        buf->width - 1 - vec.y,
        vec.x,
    };
}

// Transforms the co-ordinates as 1x counter-clockwise rotation.
static inline pax_vec2i pax_orient_flip_vec2i(pax_buf_t const *buf, pax_vec2i vec) __attribute__((pure));
static inline pax_vec2i pax_orient_flip_vec2i(pax_buf_t const *buf, pax_vec2i vec) {
    return (pax_vec2i){
        buf->width - 1 - vec.x,
        vec.y,
    };
}

// Transforms the co-ordinates as 1x counter-clockwise rotation.
static inline pax_vec2i pax_orient_ccw1_flip_vec2i(pax_buf_t const *buf, pax_vec2i vec) __attribute__((pure));
static inline pax_vec2i pax_orient_ccw1_flip_vec2i(pax_buf_t const *buf, pax_vec2i vec) {
    return (pax_vec2i){
        buf->width - 1 - vec.y,
        buf->height - 1 - vec.x,
    };
}

// Transforms the co-ordinates as 2x counter-clockwise rotation.
static inline pax_vec2i pax_orient_ccw2_flip_vec2i(pax_buf_t const *buf, pax_vec2i vec) __attribute__((pure));
static inline pax_vec2i pax_orient_ccw2_flip_vec2i(pax_buf_t const *buf, pax_vec2i vec) {
    return (pax_vec2i){
        vec.x,
        buf->height - 1 - vec.y,
    };
}

// Transforms the co-ordinates as 2x counter-clockwise rotation.
static inline pax_vec2i pax_orient_ccw3_flip_vec2i(pax_buf_t const *buf, pax_vec2i vec) __attribute__((pure));
static inline pax_vec2i pax_orient_ccw3_flip_vec2i(pax_buf_t const *buf, pax_vec2i vec) {
    return (pax_vec2i){
        vec.y,
        vec.x,
    };
}

// Detects orientation and transforms co-ordinates accordingly.
pax_vec2i pax_orient_det_vec2i(pax_buf_t const *buf, pax_vec2i vec) {
#if PAX_COMPILE_ORIENTATION
    switch (buf->orientation) {
        default:
        case PAX_O_UPRIGHT: return vec;
        case PAX_O_ROT_CCW: return pax_orient_ccw1_vec2i(buf, vec);
        case PAX_O_ROT_HALF: return pax_orient_ccw2_vec2i(buf, vec);
        case PAX_O_ROT_CW: return pax_orient_ccw3_vec2i(buf, vec);
        case PAX_O_FLIP_H: return pax_orient_flip_vec2i(buf, vec);
        case PAX_O_ROT_CCW_FLIP_H: return pax_orient_ccw1_flip_vec2i(buf, vec);
        case PAX_O_ROT_HALF_FLIP_H: return pax_orient_ccw2_flip_vec2i(buf, vec);
        case PAX_O_ROT_CW_FLIP_H: return pax_orient_ccw3_flip_vec2i(buf, vec);
    }
#else
    return vec;
#endif
}

// Detects orientation and transforms co-ordinates accordingly.
pax_vec2i pax_unorient_det_vec2i(pax_buf_t const *buf, pax_vec2i vec) {
#if PAX_COMPILE_ORIENTATION
    switch (buf->orientation) {
        default:
        case PAX_O_UPRIGHT: return vec;
        case PAX_O_ROT_CCW: return pax_orient_ccw3_vec2i(buf, vec);
        case PAX_O_ROT_HALF: return pax_orient_ccw2_vec2i(buf, vec);
        case PAX_O_ROT_CW: return pax_orient_ccw1_vec2i(buf, vec);
        case PAX_O_FLIP_H: return pax_orient_flip_vec2i(buf, vec);
        case PAX_O_ROT_CCW_FLIP_H: return pax_orient_ccw1_flip_vec2i(buf, vec);
        case PAX_O_ROT_HALF_FLIP_H: return pax_orient_ccw2_flip_vec2i(buf, vec);
        case PAX_O_ROT_CW_FLIP_H: return pax_orient_ccw3_flip_vec2i(buf, vec);
    }
#else
    return vec;
#endif
}
