
.PHONY: all build clean

all: build

build:
	mkdir -p build
	cd build && PAX_CMAKE_STANDALONE=Linux cmake ..
	cd build && PAX_CMAKE_STANDALONE=Linux make -j$(shell nproc)

clean:
	rm -rf build
