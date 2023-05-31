
cmake_minimum_required(VERSION 3.13)

# C source files.
set(PAX_SRCS_C
	${CMAKE_CURRENT_LIST_DIR}/src/pax_matrix.c
	${CMAKE_CURRENT_LIST_DIR}/src/pax_gfx.c
	${CMAKE_CURRENT_LIST_DIR}/src/pax_shaders.c
	${CMAKE_CURRENT_LIST_DIR}/src/pax_shapes.c
	${CMAKE_CURRENT_LIST_DIR}/src/pax_fonts.c
	${CMAKE_CURRENT_LIST_DIR}/src/pax_text.c
	${CMAKE_CURRENT_LIST_DIR}/src/pax_setters.c
	${CMAKE_CURRENT_LIST_DIR}/src/pax_orientation.c
	
	${CMAKE_CURRENT_LIST_DIR}/src/helpers/pax_dh_mcr_shaded.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/helpers/pax_dh_mcr_unshaded.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/helpers/pax_dh_shaded.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/helpers/pax_dh_unshaded.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/helpers/pax_mcr_dummy.c
	${CMAKE_CURRENT_LIST_DIR}/src/helpers/pax_mcr_esp32.c
	${CMAKE_CURRENT_LIST_DIR}/src/helpers/pax_mcr_pthread.c
	${CMAKE_CURRENT_LIST_DIR}/src/helpers/pax_precalculated.c
	
	${CMAKE_CURRENT_LIST_DIR}/src/fonts/font_bitmap_7x9.c
	${CMAKE_CURRENT_LIST_DIR}/src/fonts/font_bitmap_sky.c
	${CMAKE_CURRENT_LIST_DIR}/src/fonts/font_bitmap_permanentmarker.c
	${CMAKE_CURRENT_LIST_DIR}/src/fonts/font_bitmap_sairaregular.c
	${CMAKE_CURRENT_LIST_DIR}/src/fonts/font_bitmap_sairacondensed.c
)

# C include directories.
set(PAX_INCLUDE_C
	${CMAKE_CURRENT_LIST_DIR}/src
)

# C++ source files.
set(PAX_SRCS_CXX
	${CMAKE_CURRENT_LIST_DIR}/src/cpp/pax_cxx.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/cpp/pax_cxx_shape.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/cpp/pax_cxx_text.cpp
)

# C include directories.
set(PAX_INCLUDE_CXX
	${CMAKE_CURRENT_LIST_DIR}/src/cpp
)
