# PAX graphics
An advanced high-performance 2D graphics library for games and GUIs on embedded systems.
Supports affine transformations, shaders, text scaling, and multi-CPU software rendering.

Made for [Badge.team](https://badge.team/) and licensed MIT.

Version: v2.0.0-snapshot

# Documentation
Documentation can be found [here](docs).

# Short-term TODO list (v2.0.0)
- Re-work CMakeLists to be cleaner
- Finish the GUI library

# Long-term TODO list (v2.1.0)
- Abstract the API layer from the rendering layer
- Create a source file per group of shapes (e.g. `pax_shape_circle.c` for circle and arc variants)
- Add support for compiled or dynamically switched alternative renderers
- Add support for ESP32-P4 hardware acceleration
- Completely re-build shaders from the ground up

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
- https://why2025.org/ (the associated hacker camp)
- https://badge.team/  (the team for which i volunteer)
