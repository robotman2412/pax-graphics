
// SPDX-License-Identifier: MIT

#include "pax_gui.h"



// Light theme.
pgui_theme_t const pgui_theme_light = {
    // Size parameters.
    .dims = {
        .min_size               = {100, 30},
        .min_input_size         = {100, 30},
        .min_label_size         = {22, 22},
        .border_thickness       = 1,
        .highlight_thickness    = 2,
        .rounding               = 7,
        .padding                = {4, 4, 4, 4},
    },
    // Text style.
    .font                   = pax_font_saira_regular,
    .font_size              = 18,
    // Dropdown style.
    .dropdown = {
        .segmented     = false,
        .solid_arrow   = false,
        .covering_menu = true,
    },
    // Scrollbar style.
    .scroll = {
        .bg_col          = 0x3f000000,
        .fg_col          = 0x7fffffff,
        .width           = 6,
        .min_size        = 12,
        .offset          = 4,
        .rounding        = 3,
    },
    
    // Element styles.
    .palette = {
        // Default color palette applicable to everything.
        [PGUI_VARIANT_DEFAULT] = {
            .bg_col             = 0xffffffff,
            .fg_col             = 0xff000000,
            .input_col          = 0xffffffff,
            .active_col         = 0xffe0e0e0,
            .button_col         = 0xffd0d0d0,
            .button_active_col  = 0xffd0d0d0,
            .pressed_col        = 0xff909090,
            .border_col         = 0xff000000,
            .highlight_col      = 0xff00e0e0,
        },
        // Color palette for accept buttons, typically green.
        [PGUI_VARIANT_ACCEPT] = {
            .bg_col             = 0xffffffff,
            .fg_col             = 0xff000000,
            .input_col          = 0xffffffff,
            .active_col         = 0xffe0e0e0,
            .button_col         = 0xff60c060,
            .button_active_col  = 0xff60c060,
            .pressed_col        = 0xff009000,
            .border_col         = 0xff007000,
            .highlight_col      = 0xff00e0e0,
        },
        // Color palette for cancel buttons, typically red.
        [PGUI_VARIANT_CANCEL] = {
            .bg_col             = 0xffffffff,
            .fg_col             = 0xff000000,
            .input_col          = 0xffffffff,
            .active_col         = 0xffd00000,
            .button_col         = 0xffe06060,
            .button_active_col  = 0xffe06060,
            .pressed_col        = 0xffd01010,
            .border_col         = 0xff700000,
            .highlight_col      = 0xff00e0e0,
        },
        // Color palette for list buttons, typically blue background.
        [PGUI_VARIANT_LIST] = {
            .bg_col             = 0xffffffff,
            .fg_col             = 0xff000000,
            .input_col          = 0xffffffff,
            .active_col         = 0xffe0e0e0,
            .button_col         = 0xffd0d0d0,
            .button_active_col  = 0xffd0d0d0,
            .pressed_col        = 0xff909090,
            .border_col         = 0xff000000,
            .highlight_col      = 0xff00e0e0,
        },
        // Color palette for panels like docks toolbars and the text on them.
        [PGUI_VARIANT_PANEL] = {
            .bg_col             = 0xff000000,
            .fg_col             = 0xffffffff,
            .input_col          = 0xffffffff,
            .active_col         = 0xffe0e0e0,
            .button_col         = 0xffd0d0d0,
            .button_active_col  = 0xffd0d0d0,
            .pressed_col        = 0xff909090,
            .border_col         = 0xffffffff,
            .highlight_col      = 0xff00e0e0,
        },
    },
};



// Current default theme.
static pgui_theme_t const *const default_theme = &pgui_theme_light;

// Get default theme.
pgui_theme_t const *pgui_get_default_theme() {
    return default_theme;
}
