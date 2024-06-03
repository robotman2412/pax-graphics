
// SPDX-License-Identifier: MIT

#include "pax_gui.h"
#include "pax_internal.h"

static char const TAG[] = "pax-gui";

// Default theme.
pgui_theme_t const pgui_theme_default = {
    // Element styles.
    .bg_col                 = 0xff303030,
    .fg_col                 = 0xffffffff,
    .input_col              = 0xff404040,
    .pressed_col            = 0xff202020,
    .border_col             = 0xff000000,
    .highlight_col          = 0xff005fcf,
    .rounding               = 7,
    .input_padding          = 4,
    .text_padding           = 4,
    .box_padding            = 4,
    // Text style.
    .font                   = pax_font_saira_regular,
    .font_size              = 18,
    // Dropdown style.
    .dropdown_segmented     = false,
    .dropdown_solid_arrow   = false,
    .dropdown_covering_menu = false,
    // Scrollbar style.
    .scroll_bg_col          = 0x3f000000,
    .scroll_fg_col          = 0x7fffffff,
    .scroll_width           = 6,
    .scroll_min_size        = 12,
    .scroll_offset          = 4,
    .scroll_rounding        = 3,
};
