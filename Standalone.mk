
# Options
CC            ?=/usr/bin/cc
LD            ?=/usr/bin/ld
PAX_BUILD_DIR ?=build
PAX_CCOPTIONS ?=-c -fPIC -DPAX_STANDALONE -Isrc
PAX_LDOPTIONS ?=-shared

# Sources
SOURCES        =src/matrix.c \
				src/pax_gfx.c \
				src/pax_shaders.c \
				src/pax_shapes.c \
				src/pax_fonts.c \
				src/pax_text.c \
				\
				src/fonts/font_bitmap_7x9.c \
				src/fonts/font_bitmap_sky.c \
				src/fonts/font_bitmap_permanentmarker.c \
				src/fonts/font_bitmap_sairaregular.c \
				src/fonts/font_bitmap_sairacondensed.c
HEADERS        =$(shell find src -type f -name '*.h')

# Outputs
OBJECTS        =$(shell echo $(SOURCES) | sed -e 's/src/build/g;s/\.c/.c.o/g')
OBJECTS_DEBUG  =$(shell echo $(SOURCES) | sed -e 's/src/build/g;s/\.c/.c.debug.o/g')
PAX_LIB_PATH  ?=build/libpax.so

# Actions
.PHONY: all debug clean

all: build/pax_gfx_lib.so
	@mkdir -p build
	@cp build/pax_gfx_lib.so $(PAX_LIB_PATH)

debug: build/pax_gfx_lib.debug.so
	@mkdir -p build
	@cp build/pax_gfx_lib.debug.so $(PAX_LIB_PATH)

# Regular files
build/pax_gfx_lib.so: $(OBJECTS)
	@mkdir -p $(shell dirname $@)
	$(LD) $(LDFLAGS) -o $@ $<

build/%.o: src/% $(HEADERS)
	@mkdir -p $(shell dirname $@)
	$(CC) $(PAX_CCOPTIONS) -o $@ $<

# Debug files
build/pax_gfx_lib.debug.so: $(OBJECTS_DEBUG)
	@mkdir -p $(shell dirname $@)
	$(LD) $(LDFLAGS) -o $@ $<

build/%.debug.o: src/% $(HEADERS)
	@mkdir -p $(shell dirname $@)
	$(CC) $(PAX_CCOPTIONS) -ggdb -o $@ $<
