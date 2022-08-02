
# Options
CC            ?=/usr/bin/gcc
PAX_BUILD_DIR ?=build
PAX_CCOPTIONS ?=-c -fPIC -DPAX_STANDALONE -Isrc -Isrc/pthreadqueue/src
PAX_LDOPTIONS ?=-shared
PAX_LIBS      ?=-lpthread

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
				src/fonts/font_bitmap_sairacondensed.c\
				\
				src/pthreadqueue/src/ptq.c
HELPERS        =$(shell find src/helpers -type f -name '*.c')
HEADERS        =$(shell find src -type f -name '*.h')

# Outputs
OBJECTS        =$(shell echo " $(SOURCES)" | sed -e 's/ src/ $(PAX_BUILD_DIR)/g;s/\.c/.c.o/g')
OBJECTS_DEBUG  =$(shell echo " $(SOURCES)" | sed -e 's/ src/ $(PAX_BUILD_DIR)/g;s/\.c/.c.debug.o/g')
PAX_LIB_PATH  ?=build/libpax.so

# Actions
.PHONY: all debug clean

all: build/pax_gfx_lib.so
	@mkdir -p build
	@cp build/pax_gfx_lib.so $(PAX_LIB_PATH)

debug: build/pax_gfx_lib.debug.so
	@mkdir -p build
	@cp build/pax_gfx_lib.debug.so $(PAX_LIB_PATH)

clean:
	rm -rf build

# Regular files
build/pax_gfx_lib.so: $(OBJECTS)
	@mkdir -p $(shell dirname $@)
	$(CC) $(PAX_LDOPTIONS) -o $@ $^ $(PAX_LIBS)

build/%.o: src/% $(HEADERS) $(HELPERS)
	@mkdir -p $(shell dirname $@)
	$(CC) $(PAX_CCOPTIONS) -o $@ $< $(PAX_LIBS)

# Debug files
build/pax_gfx_lib.debug.so: $(OBJECTS_DEBUG)
	@mkdir -p $(shell dirname $@)
	$(CC) $(PAX_LDOPTIONS) -o $@ $^ $(PAX_LIBS)

build/%.debug.o: src/% $(HEADERS) $(HELPERS)
	@mkdir -p $(shell dirname $@)
	$(CC) $(PAX_CCOPTIONS) -ggdb -o $@ $< $(PAX_LIBS)
