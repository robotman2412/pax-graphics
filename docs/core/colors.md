# PAX Docs: Colors

In PAX, there are a few color utilities.
These include:
- [RGB (for combining RGB)](#color-rgb)
- [HSV (for color pickers and rainbows)](#color-hsv)
- [interpolation (for gradients)](#color-merging)
- [alpha-based merging (called overlaying in computer graphics)](#color-merging)

All colors are measured in 0-255 integers.

### Color: HSV

| returns   | name         | args               | description
| :------   | :---         | :---               | :---
| pax_col_t | pax_col_ahsv | uint8_t alpha, hue, saturation, brightness | Translates HSV into RGB and sets the alpha to A.
| pax_col_t | pax_col_hsv  | uint8_t hue, saturation, brightness        | Translates HSV into RGB and sets the alpha to fullly opaque.

### Color: RGB

| returns   | name         | args               | description
| :------   | :---         | :---               | :---
| pax_col_t | pax_col_argb | uint8_t alpha, red, green, blue | Combines ARGB from individual numbers.
| pax_col_t | pax_col_rgb  | uint8_t red, green, blue        | Combines RGB from individual numbers. Full opacity.

### Color: merging

| returns   | name          | args               | description
| :------   | :---          | :---               | :---
| pax_col_t | pax_col_lerp  | uint8_t part, pax_col_t from, pax_col_t to | Performs a linear interpolation (fade) between two colors.
| pax_col_t | pax_col_merge | pax_col_t base, pax_col_t top              | Overlays top over base, specified by alpha channels.
| pax_col_t | pax_col_tint  | pax_col_t col, pax_col_t tint              | Tints col by tint, commonly used for textures.
