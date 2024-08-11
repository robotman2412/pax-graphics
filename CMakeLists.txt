
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.13)

if(ESP_PLATFORM)
    set(PAX_STATIC true)
endif()

if(PAX_STATIC)
    set(_PAX_STATIC_OR_SHARED STATIC)
else()
    set(_PAX_STATIC_OR_SHARED SHARED)
endif()

add_subdirectory(core)
add_subdirectory(gui)

if(ESP_PLATFORM)
    # ESP-IDF integration.
    target_link_libraries(pax_gfx PRIVATE esp_system esp_timer esp_pm)
    target_compile_definitions(pax_gfx PUBLIC -DPAX_COMPILE_MCR=1)
endif()
