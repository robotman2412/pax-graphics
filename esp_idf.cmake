# This file is required by ESP-IDF to use pax-graphics as a component.

cmake_minimum_required(VERSION 3.13)

# Add sources.
include(${CMAKE_CURRENT_LIST_DIR}/sources.cmake)
if(DEFINED PAX_COMPILE_CXX)
	set(PAX_SRCS ${PAX_SRCS_C})
	set(PAX_INCLUDE ${PAX_INCLUDE_C})
else()
	set(PAX_SRCS ${PAX_SRCS_C} ${PAX_SRCS_CXX})
	set(PAX_INCLUDE ${PAX_INCLUDE_C} ${PAX_INCLUDE_CXX})
endif()

# Register component.
idf_component_register(
	SRCS ${PAX_SRCS}
	INCLUDE_DIRS ${PAX_INCLUDE}
	REQUIRES esp_system esp_timer esp_pm
)

# Disable unused constant warning because of built-in fonts.
set_target_properties(${COMPONENT_LIB} PROPERTIES COMPILE_FLAGS "-Wno-unused-const-variable")
