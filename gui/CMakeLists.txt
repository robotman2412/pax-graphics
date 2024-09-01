
# SPDX-License-Identidier: MIT

cmake_minimum_required(VERSION 3.13)

set(src ${CMAKE_CURRENT_LIST_DIR}/src)
set(include ${CMAKE_CURRENT_LIST_DIR}/include)



set(pax_gui_srcs
    ${src}/elem/pax_gui_button.c
    ${src}/elem/pax_gui_dropdown.c
    ${src}/elem/pax_gui_grid.c
    ${src}/elem/pax_gui_image.c
    ${src}/elem/pax_gui_overlay.c
    ${src}/elem/pax_gui_text.c
    ${src}/elem/pax_gui_textbox.c
    ${src}/pax_gui_util.c
    ${src}/pax_gui.c
)

set(pax_gui_include ${include})

set(pax_gui_options)



if(NOT ESP_PLATFORM)
    add_library(pax_gui ${_PAX_STATIC_OR_SHARED} ${pax_gui_srcs})
    target_include_directories(pax_gui PUBLIC ${pax_gui_include})
    target_compile_options(pax_gui PUBLIC ${pax_gui_options})
    target_compile_options(pax_gui PRIVATE -O2 -Wno-unused-const-variable)
    target_link_libraries(pax_gui PUBLIC pax_gfx)
endif()
