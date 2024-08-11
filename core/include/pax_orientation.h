
// SPDX-License-Identifier: MIT

#ifndef PAX_ORIENTATION_H
#define PAX_ORIENTATION_H

#include "pax_types.h"

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

/* ======= ROTATION HELPERS ====== */

// Flip the orientation horizontally.
static inline pax_orientation_t pax_orient_flip_h(pax_orientation_t x) __attribute__((const));
static inline pax_orientation_t pax_orient_flip_h(pax_orientation_t x) {
    if ((unsigned int)x >= 8)
        return PAX_O_FLIP_H;
    return (pax_orientation_t)(4 ^ (int)x);
}

// Flip the orientation vertically.
static inline pax_orientation_t pax_orient_flip_v(pax_orientation_t x) __attribute__((const));
static inline pax_orientation_t pax_orient_flip_v(pax_orientation_t x) {
    switch (x) {
        default:
        case PAX_O_UPRIGHT: return PAX_O_ROT_HALF_FLIP_H;
        case PAX_O_ROT_CCW: return PAX_O_ROT_CW_FLIP_H;
        case PAX_O_ROT_HALF: return PAX_O_FLIP_H;
        case PAX_O_ROT_CW: return PAX_O_ROT_CCW_FLIP_H;
        case PAX_O_FLIP_H: return PAX_O_ROT_HALF;
        case PAX_O_ROT_CCW_FLIP_H: return PAX_O_ROT_CW;
        case PAX_O_ROT_HALF_FLIP_H: return PAX_O_UPRIGHT;
        case PAX_O_ROT_CW_FLIP_H: return PAX_O_ROT_CCW;
    }
}

// Rotate the orientation a quarter turn counter-clockwise.
static inline pax_orientation_t pax_orient_rot_ccw(pax_orientation_t x) __attribute__((const));
static inline pax_orientation_t pax_orient_rot_ccw(pax_orientation_t x) {
    switch (x) {
        default:
        case PAX_O_UPRIGHT: return PAX_O_ROT_CCW;
        case PAX_O_ROT_CCW: return PAX_O_ROT_HALF;
        case PAX_O_ROT_HALF: return PAX_O_ROT_CW;
        case PAX_O_ROT_CW: return PAX_O_UPRIGHT;
        case PAX_O_FLIP_H: return PAX_O_ROT_CW_FLIP_H;
        case PAX_O_ROT_CCW_FLIP_H: return PAX_O_FLIP_H;
        case PAX_O_ROT_HALF_FLIP_H: return PAX_O_ROT_CCW_FLIP_H;
        case PAX_O_ROT_CW_FLIP_H: return PAX_O_ROT_HALF_FLIP_H;
    }
}

// Rotate the orientation a quarter turn clockwise.
static inline pax_orientation_t pax_orient_rot_cw(pax_orientation_t x) __attribute__((const));
static inline pax_orientation_t pax_orient_rot_cw(pax_orientation_t x) {
    switch (x) {
        default:
        case PAX_O_UPRIGHT: return PAX_O_ROT_CW;
        case PAX_O_ROT_CCW: return PAX_O_UPRIGHT;
        case PAX_O_ROT_HALF: return PAX_O_ROT_CCW;
        case PAX_O_ROT_CW: return PAX_O_ROT_HALF;
        case PAX_O_FLIP_H: return PAX_O_ROT_CCW_FLIP_H;
        case PAX_O_ROT_CCW_FLIP_H: return PAX_O_ROT_HALF_FLIP_H;
        case PAX_O_ROT_HALF_FLIP_H: return PAX_O_ROT_CW_FLIP_H;
        case PAX_O_ROT_CW_FLIP_H: return PAX_O_FLIP_H;
    }
}

// Rotate the orientation by a half turn.
static inline pax_orientation_t pax_orient_rot_half(pax_orientation_t x) __attribute__((const));
static inline pax_orientation_t pax_orient_rot_half(pax_orientation_t x) {
    switch (x) {
        default:
        case PAX_O_UPRIGHT: return PAX_O_ROT_HALF;
        case PAX_O_ROT_CCW: return PAX_O_ROT_CW;
        case PAX_O_ROT_HALF: return PAX_O_UPRIGHT;
        case PAX_O_ROT_CW: return PAX_O_ROT_CCW;
        case PAX_O_FLIP_H: return PAX_O_ROT_HALF_FLIP_H;
        case PAX_O_ROT_CCW_FLIP_H: return PAX_O_ROT_CW_FLIP_H;
        case PAX_O_ROT_HALF_FLIP_H: return PAX_O_FLIP_H;
        case PAX_O_ROT_CW_FLIP_H: return PAX_O_ROT_CCW_FLIP_H;
    }
}

// Get the inverse equivalent to the orientation.
static inline pax_orientation_t pax_orient_inverse(pax_orientation_t x) __attribute__((const));
static inline pax_orientation_t pax_orient_inverse(pax_orientation_t x) {
    switch (x) {
        default:
        case PAX_O_UPRIGHT: return PAX_O_UPRIGHT;
        case PAX_O_ROT_CCW: return PAX_O_ROT_CW;
        case PAX_O_ROT_HALF: return PAX_O_ROT_HALF;
        case PAX_O_ROT_CW: return PAX_O_ROT_CCW;
        case PAX_O_FLIP_H: return PAX_O_FLIP_H;
        case PAX_O_ROT_CCW_FLIP_H: return PAX_O_ROT_CCW_FLIP_H;
        case PAX_O_ROT_HALF_FLIP_H: return PAX_O_ROT_HALF_FLIP_H;
        case PAX_O_ROT_CW_FLIP_H: return PAX_O_ROT_CW_FLIP_H;
    }
}


// Detects orientation and transforms co-ordinates accordingly.
pax_vec2f pax_orient_det_vec2f(pax_buf_t const *buf, pax_vec2f vec) __attribute__((pure));
// Detects orientation and transforms co-ordinates accordingly.
pax_vec2f pax_unorient_det_vec2f(pax_buf_t const *buf, pax_vec2f vec) __attribute__((pure));

// Detects orientation and transforms co-ordinates accordingly.
pax_rectf pax_orient_det_rectf(pax_buf_t const *buf, pax_rectf vec) __attribute__((pure));
// Detects orientation and transforms co-ordinates accordingly.
pax_rectf pax_unorient_det_rectf(pax_buf_t const *buf, pax_rectf vec) __attribute__((pure));

// Detects orientation and transforms co-ordinates accordingly.
pax_vec2i pax_orient_det_vec2i(pax_buf_t const *buf, pax_vec2i vec) __attribute__((pure));
// Detects orientation and transforms co-ordinates accordingly.
pax_vec2i pax_unorient_det_vec2i(pax_buf_t const *buf, pax_vec2i vec) __attribute__((pure));

// Detects orientation and transforms co-ordinates accordingly.
pax_recti pax_orient_det_recti(pax_buf_t const *buf, pax_recti vec) __attribute__((pure));
// Detects orientation and transforms co-ordinates accordingly.
pax_recti pax_unorient_det_recti(pax_buf_t const *buf, pax_recti vec) __attribute__((pure));


#ifdef __cplusplus
} // extern "C"
#endif //__cplusplus

#endif // PAX_ORIENTATION_H
