
// SPDX-License-Identifier: MIT

#include "pax_orientation.h"



// Transforms the co-ordinates as 1x counter-clockwise rotation.
static inline pax_vec2f pax_orient_ccw1_vec2f(pax_vec2i buf_dim, pax_vec2f vec) __attribute__((pure));
static inline pax_vec2f pax_orient_ccw1_vec2f(pax_vec2i buf_dim, pax_vec2f vec) {
    return (pax_vec2f){
        vec.y,
        buf_dim.y - vec.x,
    };
}

// Transforms the co-ordinates as 2x counter-clockwise rotation.
static inline pax_vec2f pax_orient_ccw2_vec2f(pax_vec2i buf_dim, pax_vec2f vec) __attribute__((pure));
static inline pax_vec2f pax_orient_ccw2_vec2f(pax_vec2i buf_dim, pax_vec2f vec) {
    return (pax_vec2f){
        buf_dim.x - vec.x,
        buf_dim.y - vec.y,
    };
}

// Transforms the co-ordinates as 3x counter-clockwise rotation.
static inline pax_vec2f pax_orient_ccw3_vec2f(pax_vec2i buf_dim, pax_vec2f vec) __attribute__((pure));
static inline pax_vec2f pax_orient_ccw3_vec2f(pax_vec2i buf_dim, pax_vec2f vec) {
    return (pax_vec2f){
        buf_dim.x - vec.y,
        vec.x,
    };
}

// Transforms the co-ordinates as flip horizontally.
static inline pax_vec2f pax_orient_flip_vec2f(pax_vec2i buf_dim, pax_vec2f vec) __attribute__((pure));
static inline pax_vec2f pax_orient_flip_vec2f(pax_vec2i buf_dim, pax_vec2f vec) {
    return (pax_vec2f){
        buf_dim.x - vec.x,
        vec.y,
    };
}

// Transforms the co-ordinates as 1x counter-clockwise rotation and flip horizontally.
static inline pax_vec2f pax_orient_ccw1_flip_vec2f(pax_vec2i buf_dim, pax_vec2f vec) __attribute__((pure));
static inline pax_vec2f pax_orient_ccw1_flip_vec2f(pax_vec2i buf_dim, pax_vec2f vec) {
    return (pax_vec2f){
        buf_dim.x - vec.y,
        buf_dim.y - vec.x,
    };
}

// Transforms the co-ordinates as 2x counter-clockwise rotation and flip horizontally.
static inline pax_vec2f pax_orient_ccw2_flip_vec2f(pax_vec2i buf_dim, pax_vec2f vec) __attribute__((pure));
static inline pax_vec2f pax_orient_ccw2_flip_vec2f(pax_vec2i buf_dim, pax_vec2f vec) {
    return (pax_vec2f){
        vec.x,
        buf_dim.y - vec.y,
    };
}

// Transforms the co-ordinates as 3x counter-clockwise rotation and flip horizontally.
static inline pax_vec2f pax_orient_ccw3_flip_vec2f(pax_vec2i buf_dim, pax_vec2f vec) __attribute__((pure));
static inline pax_vec2f pax_orient_ccw3_flip_vec2f(pax_vec2i buf_dim, pax_vec2f vec) {
    (void)buf_dim;
    return (pax_vec2f){
        vec.y,
        vec.x,
    };
}

// Detects orientation and transforms co-ordinates accordingly.
pax_vec2f pax_orient_det_vec2f(pax_buf_t const *buf, pax_vec2f vec) {
#if CONFIG_PAX_COMPILE_ORIENTATION
    pax_vec2i buf_dim = {buf->width, buf->height};
    switch (buf->orientation) {
        default:
        case PAX_O_UPRIGHT: return vec;
        case PAX_O_ROT_CCW: return pax_orient_ccw1_vec2f(buf_dim, vec);
        case PAX_O_ROT_HALF: return pax_orient_ccw2_vec2f(buf_dim, vec);
        case PAX_O_ROT_CW: return pax_orient_ccw3_vec2f(buf_dim, vec);
        case PAX_O_FLIP_H: return pax_orient_flip_vec2f(buf_dim, vec);
        case PAX_O_ROT_CCW_FLIP_H: return pax_orient_ccw1_flip_vec2f(buf_dim, vec);
        case PAX_O_ROT_HALF_FLIP_H: return pax_orient_ccw2_flip_vec2f(buf_dim, vec);
        case PAX_O_ROT_CW_FLIP_H: return pax_orient_ccw3_flip_vec2f(buf_dim, vec);
    }
#else
    (void)buf;
    return vec;
#endif
}

// Detects orientation and transforms co-ordinates accordingly.
pax_vec2f pax_unorient_det_vec2f(pax_buf_t const *buf, pax_vec2f vec) {
#if CONFIG_PAX_COMPILE_ORIENTATION
    pax_vec2i buf_dim   = {buf->width, buf->height};
    pax_vec2i buf_dim_r = {buf->height, buf->width};
    switch (buf->orientation) {
        default:
        case PAX_O_UPRIGHT: return vec;
        case PAX_O_ROT_CCW: return pax_orient_ccw3_vec2f(buf_dim_r, vec);
        case PAX_O_ROT_HALF: return pax_orient_ccw2_vec2f(buf_dim, vec);
        case PAX_O_ROT_CW: return pax_orient_ccw1_vec2f(buf_dim_r, vec);
        case PAX_O_FLIP_H: return pax_orient_flip_vec2f(buf_dim, vec);
        case PAX_O_ROT_CCW_FLIP_H: return pax_orient_ccw1_flip_vec2f(buf_dim_r, vec);
        case PAX_O_ROT_HALF_FLIP_H: return pax_orient_ccw2_flip_vec2f(buf_dim, vec);
        case PAX_O_ROT_CW_FLIP_H: return pax_orient_ccw3_flip_vec2f(buf_dim_r, vec);
    }
#else
    (void)buf;
    return vec;
#endif
}


// Transforms the co-ordinates as 1x counter-clockwise rotation.
static inline pax_rectf pax_orient_ccw1_rectf(pax_vec2i buf_dim, pax_rectf vec) __attribute__((pure));
static inline pax_rectf pax_orient_ccw1_rectf(pax_vec2i buf_dim, pax_rectf vec) {
    return (pax_rectf){
        vec.y,
        buf_dim.y - vec.x,
        vec.h,
        -vec.w,
    };
}

// Transforms the co-ordinates as 2x counter-clockwise rotation.
static inline pax_rectf pax_orient_ccw2_rectf(pax_vec2i buf_dim, pax_rectf vec) __attribute__((pure));
static inline pax_rectf pax_orient_ccw2_rectf(pax_vec2i buf_dim, pax_rectf vec) {
    return (pax_rectf){
        buf_dim.x - vec.x,
        buf_dim.y - vec.y,
        -vec.w,
        -vec.h,
    };
}

// Transforms the co-ordinates as 3x counter-clockwise rotation.
static inline pax_rectf pax_orient_ccw3_rectf(pax_vec2i buf_dim, pax_rectf vec) __attribute__((pure));
static inline pax_rectf pax_orient_ccw3_rectf(pax_vec2i buf_dim, pax_rectf vec) {
    return (pax_rectf){
        buf_dim.x - vec.y,
        vec.x,
        -vec.h,
        vec.w,
    };
}

// Transforms the co-ordinates as flip horizontally.
static inline pax_rectf pax_orient_flip_rectf(pax_vec2i buf_dim, pax_rectf vec) __attribute__((pure));
static inline pax_rectf pax_orient_flip_rectf(pax_vec2i buf_dim, pax_rectf vec) {
    return (pax_rectf){
        buf_dim.x - vec.x,
        vec.y,
        -vec.w,
        vec.h,
    };
}

// Transforms the co-ordinates as 1x counter-clockwise rotation and flip horizontally.
static inline pax_rectf pax_orient_ccw1_flip_rectf(pax_vec2i buf_dim, pax_rectf vec) __attribute__((pure));
static inline pax_rectf pax_orient_ccw1_flip_rectf(pax_vec2i buf_dim, pax_rectf vec) {
    return (pax_rectf){
        buf_dim.x - vec.y,
        buf_dim.y - vec.x,
        -vec.h,
        -vec.w,
    };
}

// Transforms the co-ordinates as 2x counter-clockwise rotation and flip horizontally.
static inline pax_rectf pax_orient_ccw2_flip_rectf(pax_vec2i buf_dim, pax_rectf vec) __attribute__((pure));
static inline pax_rectf pax_orient_ccw2_flip_rectf(pax_vec2i buf_dim, pax_rectf vec) {
    return (pax_rectf){
        vec.x,
        buf_dim.y - vec.y,
        vec.w,
        -vec.h,
    };
}

// Transforms the co-ordinates as 3x counter-clockwise rotation and flip horizontally.
static inline pax_rectf pax_orient_ccw3_flip_rectf(pax_vec2i buf_dim, pax_rectf vec) __attribute__((pure));
static inline pax_rectf pax_orient_ccw3_flip_rectf(pax_vec2i buf_dim, pax_rectf vec) {
    (void)buf_dim;
    return (pax_rectf){
        vec.y,
        vec.x,
        vec.h,
        vec.w,
    };
}

// Detects orientation and transforms co-ordinates accordingly.
pax_rectf pax_orient_det_rectf(pax_buf_t const *buf, pax_rectf vec) {
#if CONFIG_PAX_COMPILE_ORIENTATION
    pax_vec2i buf_dim = {buf->width, buf->height};
    switch (buf->orientation) {
        default:
        case PAX_O_UPRIGHT: return vec;
        case PAX_O_ROT_CCW: return pax_orient_ccw1_rectf(buf_dim, vec);
        case PAX_O_ROT_HALF: return pax_orient_ccw2_rectf(buf_dim, vec);
        case PAX_O_ROT_CW: return pax_orient_ccw3_rectf(buf_dim, vec);
        case PAX_O_FLIP_H: return pax_orient_flip_rectf(buf_dim, vec);
        case PAX_O_ROT_CCW_FLIP_H: return pax_orient_ccw1_flip_rectf(buf_dim, vec);
        case PAX_O_ROT_HALF_FLIP_H: return pax_orient_ccw2_flip_rectf(buf_dim, vec);
        case PAX_O_ROT_CW_FLIP_H: return pax_orient_ccw3_flip_rectf(buf_dim, vec);
    }
#else
    (void)buf;
    return vec;
#endif
}

// Detects orientation and transforms co-ordinates accordingly.
pax_rectf pax_unorient_det_rectf(pax_buf_t const *buf, pax_rectf vec) {
#if CONFIG_PAX_COMPILE_ORIENTATION
    pax_vec2i buf_dim   = {buf->width, buf->height};
    pax_vec2i buf_dim_r = {buf->height, buf->width};
    switch (buf->orientation) {
        default:
        case PAX_O_UPRIGHT: return vec;
        case PAX_O_ROT_CCW: return pax_orient_ccw3_rectf(buf_dim_r, vec);
        case PAX_O_ROT_HALF: return pax_orient_ccw2_rectf(buf_dim, vec);
        case PAX_O_ROT_CW: return pax_orient_ccw1_rectf(buf_dim_r, vec);
        case PAX_O_FLIP_H: return pax_orient_flip_rectf(buf_dim, vec);
        case PAX_O_ROT_CCW_FLIP_H: return pax_orient_ccw1_flip_rectf(buf_dim_r, vec);
        case PAX_O_ROT_HALF_FLIP_H: return pax_orient_ccw2_flip_rectf(buf_dim, vec);
        case PAX_O_ROT_CW_FLIP_H: return pax_orient_ccw3_flip_rectf(buf_dim_r, vec);
    }
#else
    (void)buf;
    return vec;
#endif
}


// Transforms the co-ordinates as 1x counter-clockwise rotation.
static inline pax_vec2i pax_orient_ccw1_vec2i(pax_vec2i buf_dim, pax_vec2i vec) __attribute__((pure));
static inline pax_vec2i pax_orient_ccw1_vec2i(pax_vec2i buf_dim, pax_vec2i vec) {
    return (pax_vec2i){
        vec.y,
        buf_dim.y - 1 - vec.x,
    };
}

// Transforms the co-ordinates as 2x counter-clockwise rotation.
static inline pax_vec2i pax_orient_ccw2_vec2i(pax_vec2i buf_dim, pax_vec2i vec) __attribute__((pure));
static inline pax_vec2i pax_orient_ccw2_vec2i(pax_vec2i buf_dim, pax_vec2i vec) {
    return (pax_vec2i){
        buf_dim.x - 1 - vec.x,
        buf_dim.y - 1 - vec.y,
    };
}

// Transforms the co-ordinates as 3x counter-clockwise rotation.
static inline pax_vec2i pax_orient_ccw3_vec2i(pax_vec2i buf_dim, pax_vec2i vec) __attribute__((pure));
static inline pax_vec2i pax_orient_ccw3_vec2i(pax_vec2i buf_dim, pax_vec2i vec) {
    return (pax_vec2i){
        buf_dim.x - 1 - vec.y,
        vec.x,
    };
}

// Transforms the co-ordinates as flip horizontally.
static inline pax_vec2i pax_orient_flip_vec2i(pax_vec2i buf_dim, pax_vec2i vec) __attribute__((pure));
static inline pax_vec2i pax_orient_flip_vec2i(pax_vec2i buf_dim, pax_vec2i vec) {
    return (pax_vec2i){
        buf_dim.x - 1 - vec.x,
        vec.y,
    };
}

// Transforms the co-ordinates as 1x counter-clockwise rotation and flip horizontally.
static inline pax_vec2i pax_orient_ccw1_flip_vec2i(pax_vec2i buf_dim, pax_vec2i vec) __attribute__((pure));
static inline pax_vec2i pax_orient_ccw1_flip_vec2i(pax_vec2i buf_dim, pax_vec2i vec) {
    return (pax_vec2i){
        buf_dim.x - 1 - vec.y,
        buf_dim.y - 1 - vec.x,
    };
}

// Transforms the co-ordinates as 2x counter-clockwise rotation and flip horizontally.
static inline pax_vec2i pax_orient_ccw2_flip_vec2i(pax_vec2i buf_dim, pax_vec2i vec) __attribute__((pure));
static inline pax_vec2i pax_orient_ccw2_flip_vec2i(pax_vec2i buf_dim, pax_vec2i vec) {
    return (pax_vec2i){
        vec.x,
        buf_dim.y - 1 - vec.y,
    };
}

// Transforms the co-ordinates as 3x counter-clockwise rotation and flip horizontally.
static inline pax_vec2i pax_orient_ccw3_flip_vec2i(pax_vec2i buf_dim, pax_vec2i vec) __attribute__((pure));
static inline pax_vec2i pax_orient_ccw3_flip_vec2i(pax_vec2i buf_dim, pax_vec2i vec) {
    (void)buf_dim;
    return (pax_vec2i){
        vec.y,
        vec.x,
    };
}

// Detects orientation and transforms co-ordinates accordingly.
pax_vec2i pax_orient_det_vec2i(pax_buf_t const *buf, pax_vec2i vec) {
#if CONFIG_PAX_COMPILE_ORIENTATION
    pax_vec2i buf_dim = {buf->width, buf->height};
    switch (buf->orientation) {
        default:
        case PAX_O_UPRIGHT: return vec;
        case PAX_O_ROT_CCW: return pax_orient_ccw1_vec2i(buf_dim, vec);
        case PAX_O_ROT_HALF: return pax_orient_ccw2_vec2i(buf_dim, vec);
        case PAX_O_ROT_CW: return pax_orient_ccw3_vec2i(buf_dim, vec);
        case PAX_O_FLIP_H: return pax_orient_flip_vec2i(buf_dim, vec);
        case PAX_O_ROT_CCW_FLIP_H: return pax_orient_ccw1_flip_vec2i(buf_dim, vec);
        case PAX_O_ROT_HALF_FLIP_H: return pax_orient_ccw2_flip_vec2i(buf_dim, vec);
        case PAX_O_ROT_CW_FLIP_H: return pax_orient_ccw3_flip_vec2i(buf_dim, vec);
    }
#else
    (void)buf;
    return vec;
#endif
}

// Detects orientation and transforms co-ordinates accordingly.
pax_vec2i pax_unorient_det_vec2i(pax_buf_t const *buf, pax_vec2i vec) {
#if CONFIG_PAX_COMPILE_ORIENTATION
    pax_vec2i buf_dim   = {buf->width, buf->height};
    pax_vec2i buf_dim_r = {buf->height, buf->width};
    switch (buf->orientation) {
        default:
        case PAX_O_UPRIGHT: return vec;
        case PAX_O_ROT_CCW: return pax_orient_ccw3_vec2i(buf_dim_r, vec);
        case PAX_O_ROT_HALF: return pax_orient_ccw2_vec2i(buf_dim, vec);
        case PAX_O_ROT_CW: return pax_orient_ccw1_vec2i(buf_dim_r, vec);
        case PAX_O_FLIP_H: return pax_orient_flip_vec2i(buf_dim, vec);
        case PAX_O_ROT_CCW_FLIP_H: return pax_orient_ccw1_flip_vec2i(buf_dim_r, vec);
        case PAX_O_ROT_HALF_FLIP_H: return pax_orient_ccw2_flip_vec2i(buf_dim, vec);
        case PAX_O_ROT_CW_FLIP_H: return pax_orient_ccw3_flip_vec2i(buf_dim_r, vec);
    }
#else
    (void)buf;
    return vec;
#endif
}


// Transforms the co-ordinates as 1x counter-clockwise rotation.
static inline pax_recti pax_orient_ccw1_recti(pax_vec2i buf_dim, pax_recti rect) __attribute__((pure));
static inline pax_recti pax_orient_ccw1_recti(pax_vec2i buf_dim, pax_recti rect) {
    return (pax_recti){
        rect.y,
        buf_dim.y - rect.x,
        rect.h,
        -rect.w,
    };
}

// Transforms the co-ordinates as 2x counter-clockwise rotation.
static inline pax_recti pax_orient_ccw2_recti(pax_vec2i buf_dim, pax_recti rect) __attribute__((pure));
static inline pax_recti pax_orient_ccw2_recti(pax_vec2i buf_dim, pax_recti rect) {
    return (pax_recti){
        buf_dim.x - rect.x,
        buf_dim.y - rect.y,
        -rect.w,
        -rect.h,
    };
}

// Transforms the co-ordinates as 3x counter-clockwise rotation.
static inline pax_recti pax_orient_ccw3_recti(pax_vec2i buf_dim, pax_recti rect) __attribute__((pure));
static inline pax_recti pax_orient_ccw3_recti(pax_vec2i buf_dim, pax_recti rect) {
    return (pax_recti){
        buf_dim.x - rect.y,
        rect.x,
        -rect.h,
        rect.w,
    };
}

// Transforms the co-ordinates as flip horizontally.
static inline pax_recti pax_orient_flip_recti(pax_vec2i buf_dim, pax_recti rect) __attribute__((pure));
static inline pax_recti pax_orient_flip_recti(pax_vec2i buf_dim, pax_recti rect) {
    return (pax_recti){
        buf_dim.x - rect.x,
        rect.y,
        -rect.w,
        rect.h,
    };
}

// Transforms the co-ordinates as 1x counter-clockwise rotation and flip horizontally.
static inline pax_recti pax_orient_ccw1_flip_recti(pax_vec2i buf_dim, pax_recti rect) __attribute__((pure));
static inline pax_recti pax_orient_ccw1_flip_recti(pax_vec2i buf_dim, pax_recti rect) {
    return (pax_recti){
        buf_dim.x - rect.y,
        buf_dim.y - rect.x,
        -rect.h,
        -rect.w,
    };
}

// Transforms the co-ordinates as 2x counter-clockwise rotation and flip horizontally.
static inline pax_recti pax_orient_ccw2_flip_recti(pax_vec2i buf_dim, pax_recti rect) __attribute__((pure));
static inline pax_recti pax_orient_ccw2_flip_recti(pax_vec2i buf_dim, pax_recti rect) {
    return (pax_recti){
        rect.x,
        buf_dim.y - rect.y,
        rect.w,
        -rect.h,
    };
}

// Transforms the co-ordinates as 3x counter-clockwise rotation and flip horizontally.
static inline pax_recti pax_orient_ccw3_flip_recti(pax_vec2i buf_dim, pax_recti rect) __attribute__((pure));
static inline pax_recti pax_orient_ccw3_flip_recti(pax_vec2i buf_dim, pax_recti rect) {
    (void)buf_dim;
    return (pax_recti){
        rect.y,
        rect.x,
        rect.h,
        rect.w,
    };
}

// Detects orientation and transforms co-ordinates accordingly.
pax_recti pax_orient_det_recti(pax_buf_t const *buf, pax_recti rect) {
#if CONFIG_PAX_COMPILE_ORIENTATION
    pax_vec2i buf_dim = {buf->width, buf->height};
    switch (buf->orientation) {
        default:
        case PAX_O_UPRIGHT: return rect;
        case PAX_O_ROT_CCW: return pax_orient_ccw1_recti(buf_dim, rect);
        case PAX_O_ROT_HALF: return pax_orient_ccw2_recti(buf_dim, rect);
        case PAX_O_ROT_CW: return pax_orient_ccw3_recti(buf_dim, rect);
        case PAX_O_FLIP_H: return pax_orient_flip_recti(buf_dim, rect);
        case PAX_O_ROT_CCW_FLIP_H: return pax_orient_ccw1_flip_recti(buf_dim, rect);
        case PAX_O_ROT_HALF_FLIP_H: return pax_orient_ccw2_flip_recti(buf_dim, rect);
        case PAX_O_ROT_CW_FLIP_H: return pax_orient_ccw3_flip_recti(buf_dim, rect);
    }
#else
    (void)buf;
    return rect;
#endif
}

// Detects orientation and transforms co-ordinates accordingly.
pax_recti pax_unorient_det_recti(pax_buf_t const *buf, pax_recti rect) {
#if CONFIG_PAX_COMPILE_ORIENTATION
    pax_vec2i buf_dim   = {buf->width, buf->height};
    pax_vec2i buf_dim_r = {buf->height, buf->width};
    switch (buf->orientation) {
        default:
        case PAX_O_UPRIGHT: return rect;
        case PAX_O_ROT_CCW: return pax_orient_ccw3_recti(buf_dim_r, rect);
        case PAX_O_ROT_HALF: return pax_orient_ccw2_recti(buf_dim, rect);
        case PAX_O_ROT_CW: return pax_orient_ccw1_recti(buf_dim_r, rect);
        case PAX_O_FLIP_H: return pax_orient_flip_recti(buf_dim, rect);
        case PAX_O_ROT_CCW_FLIP_H: return pax_orient_ccw1_flip_recti(buf_dim_r, rect);
        case PAX_O_ROT_HALF_FLIP_H: return pax_orient_ccw2_flip_recti(buf_dim, rect);
        case PAX_O_ROT_CW_FLIP_H: return pax_orient_ccw3_flip_recti(buf_dim_r, rect);
    }
#else
    (void)buf;
    return rect;
#endif
}
