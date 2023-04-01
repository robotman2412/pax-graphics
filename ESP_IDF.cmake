# This file is required by ESP-IDF to use pax-graphics as a component.
idf_component_register(
	SRCS
	"src/cpp/pax_cxx.cpp"
	"src/cpp/pax_cxx_shape.cpp"
	"src/cpp/pax_cxx_text.cpp"
	
	"src/gui/pax_gui_base.cpp"
	"src/gui/icons.cpp"
	"src/gui/icons/icons.c"
	"src/gui/elements/pax_gui_button.cpp"
	"src/gui/elements/pax_gui_container.cpp"
	"src/gui/elements/pax_gui_dropdown.cpp"
	"src/gui/elements/pax_gui_image.cpp"
	"src/gui/elements/pax_gui_label.cpp"
	"src/gui/compound/pax_gui_colpicker.cpp"
	"src/gui/compound/pax_gui_filepicker.cpp"
	"src/gui/compound/pax_gui_keyboard.cpp"
	
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
	
	INCLUDE_DIRS
	"src"
	"src/cpp"
	"src/gui"
	"src/gui/elements"
	"src/gui/compound"
)
set_target_properties(${COMPONENT_LIB} PROPERTIES COMPILE_FLAGS "-Wno-unused-const-variable")
