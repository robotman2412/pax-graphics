# PAX Docs: Miscellaneous

This file is about some of the features that deserve explanation, but don't need an entire file to themselves.

This covers:
 - [Endianness](#endianness)
 - [Clipping](#clipping)
 - [Dirty area](#dirty-area)
 - [Rotation and orientation](#rotation-and-orientation)
 - [Scrolling](#scrolling)
 - [Pixel setting](#pixel-setting)
 - [Multi-core rendering](#multi-core-rendering)



# Endianness

Sometimes, the screen has a differing endianness from the processor. For this reason, `pax_buf_reversed` exists.

| name             | arguments
| :--------------- | :----------------------------------------
| pax_buf_reversed | pax_buf_t \*buf, bool reversed_endianness
This function will make PAX interpret the buffer with reversed endianness, at a small performance cost.

Examples of this feature in use include the MCH2022 badge, which has a big-endian screen but a little-endian processor.

## Example code

Creating a buffer with reversed endianness:
```c
/* Example code by Julian Scheffers: Public domain */

// Your buffer object.
extern pax_buf_t *buf;

// This is analogous to the initialisation for the MCH2022 badge.
void set_up_my_buf() {
	// Set up a 320x240 buffer with the display's format.
	buf = pax_buf_init(NULL, 320, 240, PAX_BUF_565RGB);
	// Enable reversed endianness to comply with the display.
	pax_buf_reversed(buf, true);
}
```



# Clipping

Although a little-used feature, clipping can come in handy at times.
When you apply a clip rectangle, drawing will only happen in said rectangle, the remainder unaffected.

| returns   | name         | arguments
| :-------- | :----------- | :--------
| void      | pax_clip     | pax_buf_t \*buf, int x, int y, int width, int height
| pax_recti | pax_get_clip | pax_buf_t \*buf
| void      | pax_noclip   | pax_buf_t \*buf

The function `pax_clip` sets and enables the clip rectangle.
This rectangle is not affected by matrix transformations, and there can only be one.

Similarly, `pax_get_clip` obtains the current clip rectangle. If disabled, this rectangle fills the entire buffer.

Finally, `pax_noclip` disables the clip rectangle.

## Exceptions

These functions are not affected by clipping:
 - [`pax_buf_scroll`](#scrolling)
 - [`pax_background`](drawing.md#background)
 - [`pax_set_pixel` and `pax_merge_pixel`](#pixel-setting)

## Example code

Using clipping to confine text drawing:
```c
/* Example code by Julian Scheffers: Public domain */

// This is an example for code which might need to limit
// the extents of some text.
void draws_text(pax_buf_t *buf) {
	// Make sure not to exceed some arbitrary size limit.
	pax_clip(buf, 0, 0, 100, 20);
	
	// Draw very long text.
	pax_draw_text(buf, pax_font_sky, 18, 0, 0,
		"This is a nice and long string of text");
	
	// Disable clipping again.
	pax_noclip(buf);
	
	// And now we can draw everywhere again.
	pax_draw_rect(buf, 0xffff0000, 10, 30, 50, 50);
}
```



# Dirty area

PAX automatically keeps track of the part of the screen which has been changed.
This can be used to tell display drivers to update only a subset of the display, accelerating transfer times.

| returns   | name            | arguments
| :-------- | :-------------- | :--------
| bool      | pax_is_dirty    | pax_buf_t \*buf
| pax_recti | pax_get_dirty   | pax_buf_t \*buf
| void      | pax_mark_clean  | pax_buf_t \*buf
| void      | pax_mark_dirty0 | pax_buf_t \*buf
| void      | pax_mark_dirty1 | pax_buf_t \*buf, int x, int y
| void      | pax_mark_dirty2 | pax_buf_t \*buf, int x, int y, int width, int height

The most used functions are `pax_is_dirty`, `pax_get_dirty` and `pax_mark_clean`.
The first, `pax_is_dirty` tells whether there has been drawing at all.
The second, `pax_get_dirty` obtains a rectangle which is the dirty area.
Finally, `pax_mark_clean` marks the buffer as not dirty (calling `pax_is_dirty` immediately afterwards will return `false`).

## Exceptions

These functions do not update the dirty area:
 - [`pax_buf_scroll`](#scrolling)
 - [`pax_background`](drawing.md#background)
 - [`pax_set_pixel` and `pax_merge_pixel`](#pixel-setting)

## Example code

Partially updating a display:
```c
/* Example code by Julian Scheffers: Public domain */

// This is the display driver used by the MCH2022 badge:
// https://github.com/Nicolai-Electronics/esp32-component-spi-ili9341
#include <ili9341.h>

// Partially updates the display using dirty area.
// This is an example of how an MCH2022 app updates the display.
void disp_sync(pax_buf_t *buf, ILI9341 *display) {
	// Test whether the buffer is dirty.
	if (!pax_is_dirty(buf)) return;
	
	// Obtain dirty area if so.
	pax_recti dirty = pax_get_dirty(buf);
	
	// Tell the ILI9341 driver to partially update.
	ili9341_write_partial(
		// Display to update.
		display,
		// Source framebuffer.
		pax_buf_get_pixels(buf),
		// Area to update.
		dirty.x, dirty.y,
		dirty.w, dirty.h
	);
	
	// Mark the buffer as clean so new drawing doen't
	// include the old dirty area.
	pax_mark_clean(buf);
}
```



# Rotation and orientation

Sometimes, the display isn't mounted right-side up.
To accomodate this, PAX has an orientation feature to account for this.

In quarter-turn counter-clockwise increments, PAX can be told to rotate everything.
Calls to almost all functions will act as if the buffer has been rotated accordingly.

The following functions handle the orientation setting:
| returns   | name                    | arguments
| :-------- | :---------------------- | :--------
| void      | pax_buf_set_orientation | pax_buf_t \*buf, int rotation
| int       | pax_buf_get_orientation | pax_buf_t \*buf
| pax_vec2f | pax_orient_det_vec2f    | pax_buf_t \*buf, pax_vec2f raw
| pax_rectf | pax_orient_det_rectf    | pax_buf_t \*buf, pax_rectf raw
| pax_vec2i | pax_orient_det_vec2i    | pax_buf_t \*buf, pax_vec2i raw
| pax_vec2f | pax_unorient_det_vec2f  | pax_buf_t \*buf, pax_vec2f raw
| pax_rectf | pax_unorient_det_rectf  | pax_buf_t \*buf, pax_rectf raw
| pax_vec2i | pax_unorient_det_vec2i  | pax_buf_t \*buf, pax_vec2i raw

The first function, `pax_buf_set_orientation` applies this behaviour.
Similarly, `pax_buf_get_orientation` returns the current orientation.

These functions use one of the values from the following table.
| name                  | description
| :-------------------- | :----------
| PAX_O_UPRIGHT         | No change in orientation.
| PAX_O_ROT_CCW         | Counter-clockwise rotation.
| PAX_O_ROT_HALF        | Half turn rotation.
| PAX_O_ROT_CW          | Clockwise rotation.
| PAX_O_FLIP_H          | Flip horizontally.
| PAX_O_ROT_CCW_FLIP_H  | Counter-clockwise rotation then flip horizontally.
| PAX_O_ROT_HALF_FLIP_H | Half turn rotation then flip horizontally.
| PAX_O_ROT_CW_FLIP_H   | Clockwise rotation then flip horizontally.
| PAX_O_FLIP_V          | Flip vertically. Alias to PAX_O_ROT_HALF_FLIP_H.
| PAX_O_ROT_CCW_FLIP_V  | Counter-clockwise rotation then flip vertically. Alias to PAX_O_ROT_CW_FLIP_H.
| PAX_O_ROT_HALF_FLIP_V | Half turn rotation then flip vertically. Alias to PAX_O_FLIP_H.
| PAX_O_ROT_CW_FLIP_V   | Clockwise rotation then flip vertically. Alias to PAX_O_ROT_CCW_FLIP_H.
| PAX_O_FLIP_H_ROT_CCW  | Flip horizontally then counter-clockwise rotation. Alias to PAX_O_ROT_CW_FLIP_H.
| PAX_O_FLIP_H_ROT_HALF | Flip horizontally then half turn rotation. Alias to PAX_O_ROT_HALF_FLIP_H.
| PAX_O_FLIP_H_ROT_CW   | Flip horizontally then clockwise rotation. Alias to PAX_O_ROT_CCW_FLIP_H.
| PAX_O_FLIP_V_ROT_CCW  | Flip vertically then counter-clockwise rotation. Alias to PAX_O_ROT_CCW_FLIP_H.
| PAX_O_FLIP_V_ROT_HALF | Flip vertically then half turn rotation. Alias to PAX_O_FLIP_H.
| PAX_O_FLIP_V_ROT_CW   | Flip vertically then clockwise rotation. Alias to PAX_O_ROT_CW_FLIP_H.

The `pax_orient_det_` functions are used to convert co-ordinates from input ("pretended") co-ordinates into real co-ordinates.
The `pax_unorient_det_` functions convert in the opposite direction, and are made for shaders to use.

These functions are used internally, and can come in handy if you have a shader that needs rotated co-ordinates:

## Exceptions

Notable exceptions to the orientation system are:
 - `pax_get_pixel`
 - `pax_set_pixel`
 - `pax_merge_pixel`
 - `pax_get_pixel_raw`
 - `pax_set_pixel_raw`
 - `pax_get_dirty`
 - `pax_mark_dirty0`
 - `pax_mark_dirty1`
 - `pax_mark_dirty2`

Because the screen doesn't know about the orientation,
all [the dirty area functions](#dirty-area) will not be affected by orientation settings.

Similarly, the pixel co-ordinates passed to [shader](#shaders.md) are not affected, this time because it would have a performance impact.
To account for this, you can call the `pax_orient_det_` functions ahead of time for effetcs that are sensitive to real pixel co-ordinates.

**Note: The UV co-ordinates passed to shaders are still affected by rotation, but real pixel co-ordinates are not.**

## Example code

Setting up a buffer with orientation:
```c
/* Example code by Julian Scheffers: Public domain */

// Your buffer object.
extern pax_buf_t *buf;

// An example of how you might initialise the buffer
// if the screen has been rotated.
void set_up_my_buf() {
	// Same thing as other example, set up framebuffer.
	buf = pax_buf_init(NULL, 320, 240, PAX_BUF_565RGB);
	pax_buf_reversed(buf, true);
	
	// Apply a quarter-turn counter-clockwise rotation.
	pax_buf_set_orientation(buf, PAX_O_ROT_CCW);
}
```



# Scrolling

For some things, like 2D games or terminals, you might want to scroll the contents of a buffer around a few pixels.
Because of feature requests like these, `pax_buf_scroll` was created.

| returns | name           | arguments
| :------ | :------------- | :--------
| void    | pax_buf_scroll | pax_buf_t \*buf, pax_col_t placeholder, int x, int y

When you call `pax_buf_scroll`, the contents of the buffer are scrolled according to `x` and `y`.
The areas "uncovered" by this scrolling are set to the given placeholder color. You can disable the placeholder color by setting its alpha to zero.

## Exceptions

If the placeholder color's alpha is zero, the "uncovered" areas are left as is (which is usually garbage).

## Example code

Bare-bones terminal using scrolling:
```c
/* Example code by Julian Scheffers: Public domain */

// Scrolls the screen up a bit and draws text at the bottom afterwards.
void my_simple_terminal(pax_buf_t *buf, const char *text) {
	// Scroll everything up, leaving a black background.
	pax_buf_scroll(buf, 0xff000000, 0, -20);
	
	// Draw white text at the bottom of the screen.
	pax_draw_text(
		buf, 0xffffffff,
		pax_font_sky_mono, 18,
		0, pax_buf_get_widthf(buf) - 19,
		text
	);
}
```



# Pixel setting

PAX features a very simple API to get and set pixels directly.

| returns   | name              | arguments
| :-------- | :---------------- | :--------
| void      | pax_merge_pixel   | pax_buf_t \*buf, pax_col_t color, int x, int y
| void      | pax_set_pixel     | pax_buf_t \*buf, pax_col_t color, int x, int y
| pax_col_t | pax_get_pixel     | const pax_buf_t \*buf, int x, int y
| void      | pax_set_pixel_raw | pax_buf_t \*buf, pax_col_t color, int x, int y
| pax_col_t | pax_get_pixel_raw | const pax_buf_t \*buf, int x, int y

These allow you to directly access the pixels in a buffer.

Like the name, `pax_get_pixel` and `pax_set_pixel` get and set pixel data respectively.
The `pax_get_pixel_raw` and `pax_set_pixel_raw` functions do the same, but no color conversion; you will get the raw data stored in the buffer.

The move advanced `pax_merge_pixel` is for when you want to "draw a pixel" with semi-transparent colors.

## Example code

Simple pixel manipulation:
```c
/* Example code by Julian Scheffers: Public domain */

// Does some direct pixel manipulation.
void my_fancy_code(pax_buf_t *buf) {
	// Get the color of pixel (0, 0).
	pax_col_t col = pax_get_pixel(buf, 0, 0);
	
	// Copy this pixel to it's neighbour.
	pax_set_pixel(buf, color, 1, 0);
	
	// Set a pixel to 50% between its original color and red.
	pax_merge_pixel(buf, 0x7fff0000, 10, 10);
}
```



# Multi-core rendering

The MCR (Multi-Core Rendering) feature takes 50% of the load of drawing and moves it to another core.
On platforms like dual-code ESP32s, this is basically free performance if you're not using to other core.

| returns | name                  | arguments
| :------ | :-------------------- | :--------
| void    | pax_join              |
| void    | pax_enable_multicore  | int core
| void    | pax_disable_multicore |

Called first, `pax_enable_multicore` enables the MCR feature, offloading 50% of the drawing work to the CPU specified by `core`.

Doing multi-core things means the need for synchronisation. This is the function of `pax_join`. It will wait until `core` is done rendering the current queue of tasks.
You will need to call this after shaders, before destroying your buffer, sending the buffer to a display, etc.

Finally, `pax_disable_multicore` disables the MCR feature again.

When MCR is enabled, the even scanlines are done by one core and the odd scanlines are done by the other.

## Exceptions

Some functions can't benefit from MCR because they cannot be split over the scanlines properly:
 - [`pax_draw_line`](drawing.md#outline-drawing) and other outlines / line drawings
 - [`pax_background`](drawing.md#background) because it's faster to fill everything at once
 - [`pax_buf_scroll`](#scrolling) due to a lack of pixel alignment gaurantees

## Example code

Using MCR to draw a circle:
```c
/* Example code by Julian Scheffers: Public domain */

// Enables MCR, draws a circle, and disables it again.
// In reality, you'd want to do much more drawing in MCR
// due to the overhead of starting a task.
void my_fancy_code(pax_buf_t *buf) {
	// Enable MCR, using core 1 to offload drawing to.
	pax_enable_mcr(1);
	
	// Draw a circle.
	// This can be split over two cores, and so it will.
	pax_draw_circle(buf, 0x7fff7f00, 50, 50, 25);
	
	// Draw a line through the circle.
	// This calls `pax_join` internally due to not being
	// splittable over the two cores.
	// This line is drawn on the calling core.
	pax_draw_line(buf, 0x7f0000ff, 25, 25, 75, 75);
	
	// Waits for the other core to finish, then disables MCR.
	pax_disable_mcr();
}
```
