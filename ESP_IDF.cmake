# This file is required by ESP-IDF to use pax-graphics as a component.
idf_component_register(
	SRCS
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
	"src/pax_orientation.c"
	
	"src/helpers/pax_dh_mcr_shaded.cpp"
	"src/helpers/pax_dh_mcr_unshaded.cpp"
	"src/helpers/pax_dh_shaded.cpp"
	"src/helpers/pax_dh_unshaded.cpp"
	"src/helpers/pax_mcr_dummy.c"
	"src/helpers/pax_mcr_esp32.c"
	"src/helpers/pax_mcr_pthread.c"
	"src/helpers/pax_precalculated.c"
	
	"src/fonts/font_bitmap_7x9.c"
	"src/fonts/font_bitmap_sky.c"
	"src/fonts/font_bitmap_permanentmarker.c"
	"src/fonts/font_bitmap_sairaregular.c"
	"src/fonts/font_bitmap_sairacondensed.c"
	INCLUDE_DIRS src src/cpp
)
set_target_properties(${COMPONENT_LIB} PROPERTIES COMPILE_FLAGS "-Wno-unused-const-variable")
