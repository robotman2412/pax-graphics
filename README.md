# PAX graphics
An MIT graphics library for the MCH2022 badge firmware.
ESP-IDF module and Arduino library.

Version: v1.1.0-snapshot

# Documentation
Documentation can be found [here](docs).

# Target v1.1.0 TODO list
- [x] Fix all remaining build warnings
- [x] More advanced text options
- [x] Comprehensive C++ wrapper
- [x] Triangulation that can handle simple inputs
- [x] Add `const` to all read-only accesses
- [ ] Add special case to text functions for buffers with palette types
- [ ] Check for other buffer type inconsistencies
- [ ] Complete C++ docs
  - [ ] Images
  - [ ] Examples
  - [x] `buffer.md`
  - [x] `drawing.md`
  - [x] `matrix.md`
  - [ ] `README.md`
  - [ ] `shaders.md`
  - [ ] `shapes.md`
  - [ ] `textbox.md`
- [x] Leave this box unchecked

# Feature wishlist
- GPU acceleration
- Antialiasing
- Import more fonts
- Support for holes in triangulated shapes
- Support for outline-based (.ttf or similar) fonts
- Built-in simple GUI library
- 3D rendering

# MCH2022?
[MCH2022](https://mch2022.org/) was a hackercamp in the summer of 2022.
MCH is the successor of SHA2017, where [Badge.Team](https://badge.team/) made their first badge.

The badge is an electronic event badge which can do much more than just show your name.
To accomplish all the cool graphics people can draw, this graphics stack was made.

# See also
- https://github.com/badgeteam/mch2022-firmware-esp32 (firmware for which this was made)
- https://github.com/robotman2412/mch2022-badge-tests (firmware i use to test changes)
- https://mch2022.org/ (the associated hacker camp)
- https://badge.team/  (the team for which i volunteer)
