
# SPDX-License-Identidier: MIT

cmake_minimum_required(VERSION 3.13)

add_library(pax_gui ${_PAX_STATIC_OR_SHARED}
    src/pax_gui_button.c
    src/pax_gui_dropdown.c
    src/pax_gui_grid.c
    src/pax_gui_text.c
    src/pax_gui_textbox.c
    src/pax_gui_util.c
    src/pax_gui.c
)
target_include_directories(pax_gui PUBLIC include)
target_compile_options(pax_gui PRIVATE -O2 -Wno-unused-const-variable)
target_link_libraries(pax_gui PUBLIC pax_gfx)
