
// SPDX-License-Identifier: MIT

#include "pax_gui.h"



// Light theme.
pgui_theme_t const pgui_theme_light = {
    // Element styles.
    .palette = {
        // Default color palette.
        [PGUI_VARIANT_DEFAULT] = {
            .bg_col         = 0xffffffff,
            .fg_col         = 0xff000000,
            .input_col      = 0xffffffff,
            .active_col     = 0xffe0e0e0,
            .button_col     = 0xffd0d0d0,
            .pressed_col    = 0xff909090,
            .border_col     = 0xff000000,
            .highlight_col  = 0xff00e0e0,
        },
        // Color palette for accept buttons, typically green.
        [PGUI_VARIANT_ACCEPT] = {
            .bg_col         = 0xffffffff,
            .fg_col         = 0xff000000,
            .input_col      = 0xffffffff,
            .active_col     = 0xffe0e0e0,
            .button_col     = 0xffd0d0d0,
            .pressed_col    = 0xff909090,
            .border_col     = 0xff000000,
            .highlight_col  = 0xff00e0e0,
        },
        // Color palette for cancel buttons, typically red.
        [PGUI_VARIANT_CANCEL] = {
            .bg_col         = 0xffffffff,
            .fg_col         = 0xff000000,
            .input_col      = 0xffffffff,
            .active_col     = 0xffe0e0e0,
            .button_col     = 0xffd0d0d0,
            .pressed_col    = 0xff909090,
            .border_col     = 0xff000000,
            .highlight_col  = 0xff00e0e0,
        },
        // Color palette for list buttons, typically blue background.
        [PGUI_VARIANT_LIST] = {
            .bg_col         = 0xffffffff,
            .fg_col         = 0xff000000,
            .input_col      = 0xffffffff,
            .active_col     = 0xffe0e0e0,
            .button_col     = 0xffd0d0d0,
            .pressed_col    = 0xff909090,
            .border_col     = 0xff000000,
            .highlight_col  = 0xff00e0e0,
        },
    },
    
    // Size parameters.
    .min_size               = {100, 30},
    .min_input_size         = {100, 30},
    .min_label_size         = {22, 22},
    .border_thickness       = 1,
    .highlight_thickness    = 2,
    .rounding               = 7,
    .padding                = 4,
    // Text style.
    .font                   = pax_font_saira_regular,
    .font_size              = 18,
    // Dropdown style.
    .dropdown_segmented     = false,
    .dropdown_solid_arrow   = false,
    .dropdown_covering_menu = true,
    // Scrollbar style.
    .scroll_bg_col          = 0x3f000000,
    .scroll_fg_col          = 0x7fffffff,
    .scroll_width           = 6,
    .scroll_min_size        = 12,
    .scroll_offset          = 4,
    .scroll_rounding        = 3,
};



// Current default theme.
static pgui_theme_t const *const default_theme = &pgui_theme_light;

// Get default theme.
pgui_theme_t const *pgui_get_default_theme() {
    return default_theme;
}
