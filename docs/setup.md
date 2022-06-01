# PAX Docs: Setup

The first thing you'll need to do is to set up a framebuffer.

There are two steps to do this:
1. [Pick a format](#buffer-formats)
2. [Create the buffer](#buffer-creation)

# Buffer formats

PAX allows you to choose which color format to use.
This allows you to pick one that fits your screen nicely, or just to save on memory.

Greyscale (black/white) formats:
| name            | bits per pixel
| :-------------- | :-------------
| PAX_BUF_1_GREY  | 1
| PAX_BUF_2_GREY  | 2
| PAX_BUF_4_GREY  | 4
| PAX_BUF_8_GREY  | 8

RGB (color without transparency) formats:
| name                | bits per pixel | bits red | bits green | bits blue
| :------------------ | :------------- | :------- | :--------- | :--------
| PAX_BUF_8_332RGB    | 8              | 3        | 3          | 2
| PAX_BUF_16_565RGB   | 16             | 5        | 6          | 5

ARGB (color and transparency) formats:
| name                | bits per pixel | bits per channel
| :------------------ | :------------- | :---------------
| PAX_BUF_4_1111ARGB  | 4              | 1
| PAX_BUF_8_2222ARGB  | 8              | 2
| PAX_BUF_16_4444ARGB | 16             | 4
| PAX_BUF_32_8888ARGB | 32             | 8

# Buffer creation

After picking a format, it's very simple to create your buffer.
To make a new buffer, use `pax_but_init`:
```c
pax_but_t buffer;
// Create a buffer and let PAX allocate memory for you.
pax_buf_init(&buffer, NULL, width, height, format);
```

Or allocate memory yourself:
```c
// 256x256 with 32 bits per pixel (4 bytes)
uint8_t my_memory[256*256*4];
pax_but_t buffer;
// You must be more careful with what you tell PAX about the buffer here.
pax_buf_init(&buffer, NULL, 256, 256, PAX_BUF_32_8888ARGB);
```

If you don't need the buffer anymore, use `pax_buf_destroy`:
```c
// This works regardless of whether you allocated memory manually or not.
pax_buf_destroy(&buffer);
```
