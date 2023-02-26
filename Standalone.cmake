# CMake file for compiling PAX as a library.

cmake_minimum_required(VERSION 3.13)

# Set language standards requested.
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Helpful for IDE users.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# The output must be a relocatable file.
add_link_options(-r)

add_library(pax_graphics
	"src/cpp/pax_cxx.cpp"
	"src/cpp/pax_cxx_shape.cpp"
	"src/cpp/pax_cxx_text.cpp"

	"src/pax_matrix.c"
	"src/pax_gfx.c"
	"src/pax_shaders.c"
	"src/pax_shapes.c"
	"src/pax_fonts.c"
	"src/pax_text.c"
	"src/pax_setters.c"

	"src/fonts/font_bitmap_7x9.c"
	"src/fonts/font_bitmap_sky.c"
	"src/fonts/font_bitmap_permanentmarker.c"
	"src/fonts/font_bitmap_sairaregular.c"
	"src/fonts/font_bitmap_sairacondensed.c"
)

target_include_directories(pax_graphics PUBLIC
	${CMAKE_CURRENT_LIST_DIR}/src
	${CMAKE_CURRENT_LIST_DIR}/src/cpp
)
