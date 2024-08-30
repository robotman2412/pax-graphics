
# SPDX-License-Identidier: MIT

cmake_minimum_required(VERSION 3.13)

set(src ${CMAKE_CURRENT_LIST_DIR}/src)
set(include ${CMAKE_CURRENT_LIST_DIR}/include)



set(pax_gfx_srcs
    ${src}/pax_matrix.c
    ${src}/pax_gfx.c
    ${src}/pax_shaders.c
    ${src}/pax_shapes.c
    ${src}/pax_fonts.c
    ${src}/pax_text.c
    ${src}/pax_setters.c
    ${src}/pax_orientation.c

    ${src}/helpers/pax_mcr_dummy.c
    ${src}/helpers/pax_mcr_esp32.c
    ${src}/helpers/pax_mcr_pthread.c
    ${src}/helpers/pax_precalculated.c

    ${src}/fonts/font_bitmap_7x9.c
    ${src}/fonts/font_bitmap_sky.c
    ${src}/fonts/font_bitmap_permanentmarker.c
    ${src}/fonts/font_bitmap_sairaregular.c
    ${src}/fonts/font_bitmap_sairacondensed.c

    ${src}/helpers/pax_dh_mcr_shaded.cpp
    ${src}/helpers/pax_dh_mcr_unshaded.cpp
    ${src}/helpers/pax_dh_shaded.cpp
    ${src}/helpers/pax_dh_unshaded.cpp
)

set(pax_gfx_include ${include})

set(pax_gfx_options
    -D__PAX_VERSION_IS_SNAPSHOT=${PAX_VERSION_IS_SNAPSHOT}
    -D__PAX_VERSION_MAJOR=${PAX_VERSION_MAJOR}
    -D__PAX_VERSION_MINOR=${PAX_VERSION_MINOR}
    -D__PAX_VERSION_PATCH=${PAX_VERSION_PATCH}
    -D__PAX_VERSION_NUMBER=${PAX_VERSION_MAJOR}${PAX_VERSION_MINOR}${PAX_VERSION_PATCH}
    -D__PAX_VERSION_STR="${PAX_VERSION}"
)



if(NOT ESP_PLATFORM)
    add_library(pax_gfx ${_PAX_STATIC_OR_SHARED} ${pax_gfx_srcs})
    target_include_directories(pax_gfx PUBLIC ${pax_gfx_include})
    target_compile_options(pax_gfx PUBLIC ${pax_gfx_options})
    target_compile_options(pax_gfx PRIVATE -O2 -Wno-unused-const-variable)
    target_link_options(pax_gfx PRIVATE -Wl,-Bsymbolic)
    target_link_libraries(pax_gfx PUBLIC m)
endif()
