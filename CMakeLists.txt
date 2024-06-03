
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.13)

# Source files.
set(PAX_SRCS
    ${CMAKE_CURRENT_LIST_DIR}/src/pax_matrix.c
    ${CMAKE_CURRENT_LIST_DIR}/src/pax_gfx.c
    ${CMAKE_CURRENT_LIST_DIR}/src/pax_shaders.c
    ${CMAKE_CURRENT_LIST_DIR}/src/pax_shapes.c
    ${CMAKE_CURRENT_LIST_DIR}/src/pax_fonts.c
    ${CMAKE_CURRENT_LIST_DIR}/src/pax_text.c
    ${CMAKE_CURRENT_LIST_DIR}/src/pax_setters.c
    ${CMAKE_CURRENT_LIST_DIR}/src/pax_orientation.c
    
    ${CMAKE_CURRENT_LIST_DIR}/src/helpers/pax_mcr_dummy.c
    ${CMAKE_CURRENT_LIST_DIR}/src/helpers/pax_mcr_esp32.c
    ${CMAKE_CURRENT_LIST_DIR}/src/helpers/pax_mcr_pthread.c
    ${CMAKE_CURRENT_LIST_DIR}/src/helpers/pax_precalculated.c
    
    ${CMAKE_CURRENT_LIST_DIR}/src/fonts/font_bitmap_7x9.c
    ${CMAKE_CURRENT_LIST_DIR}/src/fonts/font_bitmap_sky.c
    ${CMAKE_CURRENT_LIST_DIR}/src/fonts/font_bitmap_permanentmarker.c
    ${CMAKE_CURRENT_LIST_DIR}/src/fonts/font_bitmap_sairaregular.c
    ${CMAKE_CURRENT_LIST_DIR}/src/fonts/font_bitmap_sairacondensed.c
    
    ${CMAKE_CURRENT_LIST_DIR}/src/gui/pax_gui_calc.c
    ${CMAKE_CURRENT_LIST_DIR}/src/gui/pax_gui_draw.c
    ${CMAKE_CURRENT_LIST_DIR}/src/gui/pax_gui_event.c
    ${CMAKE_CURRENT_LIST_DIR}/src/gui/pax_gui_themes.c
    
    ${CMAKE_CURRENT_LIST_DIR}/src/helpers/pax_dh_mcr_shaded.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/helpers/pax_dh_mcr_unshaded.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/helpers/pax_dh_shaded.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/helpers/pax_dh_unshaded.cpp
)

# Include directories.
set(PAX_INCLUDE
    ${CMAKE_CURRENT_LIST_DIR}/src
)

if(ESP_PLATFORM)
    # ESP-IDF integration.
    idf_component_register(
        SRCS ${PAX_SRCS}
        INCLUDE_DIRS ${PAX_INCLUDE}
        REQUIRES esp_system esp_timer esp_pm
    )
    target_compile_definitions(${COMPONENT_LIB} PUBLIC -DPAX_COMPILE_MCR=1)
    target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-unused-const-variable)
else()
    # Just a CMake library.
    add_library(pax_graphics STATIC ${PAX_SRCS})
    target_compile_definitions(pax_graphics PUBLIC -DPAX_COMPILE_MCR=0)
    target_include_directories(pax_graphics PUBLIC ${PAX_INCLUDE})
endif()
