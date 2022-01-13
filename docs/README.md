# PAX graphics documentation

The PAX graphics stack is being developed for the [MCH2022 badge](https://bodge.team/docs/badges/mch2022/).
It's goal is to allow anyone to, in C, use a powerful list of drawing features with good optimisation.

This library is the successor of the revised graphics API for [the old badge.team firmware](https://github.com/badgeteam/ESP32-platform-firmware).

## PAX Docs overview
- [Getting started](#getting-started)
- [API overview](#api-overview)
- [API reference](#api-reference)
    - [Setup](#api-reference-setup)
    - [Basic drawing](#api-reference-basic-drawing)
    - [Colors](#api-reference-colors)
    - [Text](#api-reference-text)
- [Advanced API reference](#api-reference-advanced-features)
    - [Matrix transformations](#api-reference-matrix-transformations)
    - [Shaders](#api-reference-shaders)
    - [Clipping](#api-reference-clipping)

# Getting started

Let's get started with a simple example.

In this example, we'll make a green background with a red circle in the middle and some white text in the top left corner.

First, the setup.<br>
In each file you'd like to use PAX graphics in, you'll need to include the `pax_gfx` header: 
```c
// Include the PAX graphics library.
#include <pax_gfx.h>
```

The [`pax_buf_init`](#api-reference-setup) function is used to create a new graphics environment.
Here, we use it with the type PAX_BUF_16_565RGB, which is appropriate for the screen used by the MCH2022 badge.
```c
void my_graphics_function() {
    // Setup.
    pax_buf_t buffer;
    pax_buf_init(&buffer, NULL, 320, 240, PAX_BUF_16_565RGB);
}
```
This tells PAX to create a framebuffer for the screen, which is 320 by 240 pixels in size.

Next, let's make a nice, green background.
You can use [many methods](#api-reference-colors) of picking a color,
but we'll go with [`pax_col_rgb`](#api-reference-colors "Combines RGB channels").
The [`pax_background`](#api-reference-basic-drawing) method fills the background with the desired color, like so:
```c
void my_graphics_function() {
    // Setup.
    pax_buf_t buffer;
    pax_buf_init(&buffer, NULL, 320, 240, PAX_BUF_16_565RGB);
    
    // Green background.
    pax_background(&buffer, pax_col_rgb(0, 255, 0));
}
```
Here's what it looks like if you write it to the screen:
![A green background](images/getting_started_i_background.jpg "A green background")

To draw a circle, you use [`pax_draw_circle`](#api-reference-basic-drawing) or [`pax_simple_circle`](#api-reference-basic-drawing)
with a midpoint co-ordinates and the radius:
```c
void my_graphics_function() {
    // Setup.
    pax_buf_t buffer;
    pax_buf_init(&buffer, NULL, 320, 240, PAX_BUF_16_565RGB);
    
    // Green background.
    pax_background(&buffer, pax_col_rgb(0, 255, 0));
    
    // Red circle.
    float midpoint_x = buffer.width  / 2.0; // Middle of the screen horizontally.
    float midpoint_y = buffer.height / 2.0; // Middle of the screen vertically.
    float radius     = 50;                  // Nice, big circle.
    pax_simple_circle(&buffer, pax_col_rgb(255, 0, 0), midpoint_x, midpoint_y, radius);
}
```
Here's what it looks like if you write it to the screen:
![And a red circle](images/getting_started_i_circle.jpg "And a red circle")

Text is a bit more complicated, because you get to pick a font.
However, you can pick any font for now. We'll go with the default (and currently only font) "7x9".<br>
It's point size is 9, which means that a text size of 9 is it's normal look. We'll pick 18.<br>
The [`pax_draw_text`](#api-reference-text) method is used to draw text.
It accepts a font, a point size, a top left corner position and the text to draw:
```c
void my_graphics_function() {
    // Setup.
    pax_buf_t buffer;
    pax_buf_init(&buffer, NULL, 320, 240, PAX_BUF_16_565RGB);
    
    // Green background.
    pax_background(&buffer, pax_col_rgb(0, 255, 0));
    
    // Red circle.
    float midpoint_x = buffer.width  / 2.0; // Middle of the screen horizontally.
    float midpoint_y = buffer.height / 2.0; // Middle of the screen vertically.
    float radius     = 50;                  // Nice, big circle.
    pax_simple_circle(&buffer, pax_col_rgb(255, 0, 0), midpoint_x, midpoint_y, radius);
    
    // White text.
    float text_x     = 0;                   // Top left corner of the screen.
    float text_y     = 0;                   // Top left corner of the screen.
    char *my_text    = "Hello, World!";     // You can pick any message you'd like.
    float text_size  = 18;                  // Twice the normal size for "7x9".
    pax_draw_text(&buffer, pax_col_rgb(255, 255, 255), PAX_FONT_DEFAULT, text_size, text_x, text_y, my_text);
}
```
Here's what it looks like if you write it to the screen:
![And some text](images/getting_started_i_circle.jpg "And some text")

Next, you'll want to draw this to the screen.<br>
This differs per screen type, but for the MCH2022 badge's screen you use the
[`ili9341_write`](https://github.com/Nicolai-Electronics/esp32-component-spi-ili9341/ "Link on github") method.
```c
void my_graphics_function() {
    // Setup.
    pax_buf_t buffer;
    pax_buf_init(&buffer, NULL, 320, 240, PAX_BUF_16_565RGB);
    
    // Green background.
    pax_background(&buffer, pax_col_rgb(0, 255, 0));
    
    // Red circle.
    float midpoint_x = buffer.width  / 2.0; // Middle of the screen horizontally.
    float midpoint_y = buffer.height / 2.0; // Middle of the screen vertically.
    float radius     = 50;                  // Nice, big circle.
    pax_simple_circle(&buffer, pax_col_rgb(255, 0, 0), midpoint_x, midpoint_y, radius);
    
    // White text.
    float text_x     = 0;                   // Top left corner of the screen.
    float text_y     = 0;                   // Top left corner of the screen.
    char *my_text    = "Hello, World!";     // You can pick any message you'd like.
    float text_size  = 18;                  // Twice the normal size for "7x9".
    pax_draw_text(&buffer, pax_col_rgb(255, 255, 255), PAX_FONT_DEFAULT, text_size, text_x, text_y, my_text);
    
    // Put it on the screen.
    if (ili9341_write(&display, buffer.buf)) {
        ESP_LOGE("my_tag", "Display write failed.");
    } else {
        ESP_LOGI("my_tag", "Display write success.");
    }
}
```
(TODO: Image of this)
If you use a different screen, you'll need to find it's documentation for which type of buffer it wants and how you write to it.

Finally, there's cleanup.<br>
If you don't want to use the buffer you made during setup, you can clean it up with
[`pax_buf_destroy`](#api-reference-setup "Frees any memory assigned to the buffer").
```c
void my_graphics_function() {
    // Setup.
    pax_buf_t buffer;
    pax_buf_init(&buffer, NULL, 320, 240, PAX_BUF_16_565RGB);
    
    // Green background.
    pax_background(&buffer, pax_col_rgb(0, 255, 0));
    
    // Red circle.
    float midpoint_x = buffer.width  / 2.0; // Middle of the screen horizontally.
    float midpoint_y = buffer.height / 2.0; // Middle of the screen vertically.
    float radius     = 50;                  // Nice, big circle.
    pax_simple_circle(&buffer, pax_col_rgb(255, 0, 0), midpoint_x, midpoint_y, radius);
    
    // White text.
    float text_x     = 0;                   // Top left corner of the screen.
    float text_y     = 0;                   // Top left corner of the screen.
    char *my_text    = "Hello, World!";     // You can pick any message you'd like.
    float text_size  = 18;                  // Twice the normal size for "7x9".
    pax_draw_text(&buffer, pax_col_rgb(255, 255, 255), PAX_FONT_DEFAULT, text_size, text_x, text_y, my_text);
    
    // Put it on the screen.
    if (ili9341_write(&display, buffer.buf)) {
        ESP_LOGE("my_tag", "Display write failed.");
    } else {
        ESP_LOGI("my_tag", "Display write success.");
    }
    
    // Cleanup.
    pax_buf_destroy(&buffer);
}
```

# API overview
# API reference
## API reference: Setup
## API reference: Colors
## API reference: Basic drawing
## API reference: Text
# API reference: Advanced features
## API reference: Matrix transformations
## API reference: Shaders
## API reference: Clipping
