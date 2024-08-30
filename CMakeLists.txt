
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.13)

# Version information.
set(PAX_VERSION_IS_SNAPSHOT true)
set(PAX_VERSION_MAJOR 2)
set(PAX_VERSION_MINOR 0)
set(PAX_VERSION_PATCH 0)

set(PAX_VERSION ${PAX_VERSION_MAJOR}.${PAX_VERSION_MINOR}.${PAX_VERSION_PATCH})
if(PAX_VERSION_IS_SNAPSHOT)
    set(PAX_VERSION ${PAX_VERSION}-snapshot)
endif()

set(PAX_VERSION_IS_SNAPSHOT ${PAX_VERSION_IS_SNAPSHOT} PARENT_SCOPE)
set(PAX_VERSION_MAJOR ${PAX_VERSION_MAJOR} PARENT_SCOPE)
set(PAX_VERSION_MINOR ${PAX_VERSION_MINOR} PARENT_SCOPE)
set(PAX_VERSION_PATCH ${PAX_VERSION_PATCH} PARENT_SCOPE)
set(PAX_VERSION ${PAX_VERSION} PARENT_SCOPE)



if(ESP_PLATFORM)
    set(PAX_STATIC true)
endif()

if(PAX_STATIC)
    set(_PAX_STATIC_OR_SHARED STATIC)
else()
    set(_PAX_STATIC_OR_SHARED SHARED)
endif()

include(${CMAKE_CURRENT_LIST_DIR}/core/CMakeLists.txt)
include(${CMAKE_CURRENT_LIST_DIR}/gui/CMakeLists.txt)

if(ESP_PLATFORM)
    # ESP-IDF integration.
    idf_component_register(
        SRCS ${pax_gfx_srcs} ${pax_gui_srcs}
        INCLUDE_DIRS ${pax_gfx_include} ${pax_gui_include}
        REQUIRES esp_timer esp_system esp_pm
    )
    target_compile_options(${COMPONENT_LIB} PUBLIC ${pax_gfx_options} ${pax_gui_options})
    target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-unused-const-variable)
endif()
