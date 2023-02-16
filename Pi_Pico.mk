
.PHONY: all clean

# PAX options: Disable Multi-Core Rendering support.
defs	=	-DPAX_COMPILE_MCR=0
# Compiler options: Exceptions disabled, no system libraries.
cflags	=	-mcpu=cortex-m0plus -mthumb -nostdlib -nodefaultlibs -fno-exceptions

# Linker options: No system libraries
ldflags	=	-nostdlib -nodefaultlibs

# Environment variables: Tell CMake what compiler to use and such.
env		=	PAX_CMAKE_STANDALONE=Pi_Pico \
			CC=arm-none-eabi-gcc \
			CXX=arm-none-eabi-g++

all:
	mkdir -p build
	cd build; $(env) cmake -D "CMAKE_C_FLAGS=$(defs)" -D "CMAKE_CXX_FLAGS=$(defs)" $(cflags) $(ldflags) ..
	make -j$(shell nproc) -C build

clean:
	rm -rf build/*
