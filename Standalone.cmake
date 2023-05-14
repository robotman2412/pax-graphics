# CMake file for compiling PAX as a library.

cmake_minimum_required(VERSION 3.13)

# Set language standards requested.
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Helpful for IDE users.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(DEFINED PAX_COMPILE_CXX)
	add_library(pax_cxx STATIC
		${CMAKE_CURRENT_LIST_DIR}/src/cpp/pax_cxx.cpp
		${CMAKE_CURRENT_LIST_DIR}/src/cpp/pax_cxx_shape.cpp
		${CMAKE_CURRENT_LIST_DIR}/src/cpp/pax_cxx_text.cpp
	)
endif()

add_library(pax_graphics STATIC
	${CMAKE_CURRENT_LIST_DIR}/src/pax_matrix.c
	${CMAKE_CURRENT_LIST_DIR}/src/pax_gfx.c
	${CMAKE_CURRENT_LIST_DIR}/src/pax_shaders.c
	${CMAKE_CURRENT_LIST_DIR}/src/pax_shapes.c
	${CMAKE_CURRENT_LIST_DIR}/src/pax_fonts.c
	${CMAKE_CURRENT_LIST_DIR}/src/pax_text.c
	${CMAKE_CURRENT_LIST_DIR}/src/pax_setters.c

	${CMAKE_CURRENT_LIST_DIR}/src/fonts/font_bitmap_7x9.c
	${CMAKE_CURRENT_LIST_DIR}/src/fonts/font_bitmap_sky.c
	${CMAKE_CURRENT_LIST_DIR}/src/fonts/font_bitmap_permanentmarker.c
	${CMAKE_CURRENT_LIST_DIR}/src/fonts/font_bitmap_sairaregular.c
	${CMAKE_CURRENT_LIST_DIR}/src/fonts/font_bitmap_sairacondensed.c
)
if(PAX_COMPILE_CXX)
	target_link_libraries(pax_graphics PUBLIC pax_cxx)
endif()

target_include_directories(pax_graphics PUBLIC
	${CMAKE_CURRENT_LIST_DIR}/src
	${CMAKE_CURRENT_LIST_DIR}/src/cpp
)
