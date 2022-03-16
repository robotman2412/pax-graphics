# PAX docs: Image codecs
The image codecs are currently work-in-development, but PNG decode has been released as test.

## Where to get it
PAX codecs are not a standard part of PAX.
To use the codecs, you must have [pax-codecs](https://github.com/robotman2412/pax-codecs) installed.

Then, to access the extended API, include `pax_codecs.h`:
```c
// The PAX image codec library.
#include <pax_codecs.h>
```

## PNG API
Currently, a very limited PNG api exists:
| returns | name               | arguments
| :------ | :----------------- | :--------
| bool    | pax_decode_png_fd  | pax_buf_t \*buf, FILE \*fd, pax_buf_type_t buf_type
| bool    | pax_decode_png_buf | pax_buf_t \*buf, void \*png, size_t png_len, pax_buf_type_t buf_type

Both `pax_decode_png_` methods will decode the PNG into `buf`, which must be uninitialised.
The `buf_type` argument is merely a hint, it is not always met perfectly.

The methods return `true` on success, in which case they are like a more complex `pax_buf_init`
because they initialise the buffer and then fill it with an image.
This means that when you're done, you must call `pax_buf_destroy` to free up the allocated memory.

An example of drawing a PNG in PAX:
```c
void draw_my_png(pax_buf_t *draw_to, void *png_data, size_t png_data_len) {
    // Decode a PNG.
    pax_buf_t image;
    pax_decode_png_buf(&image, png_data, png_data_len, PAX_BUF_32_8888ARGB);
    // Draw the PNG.
    pax_draw_image(draw_to, &image, 0, 0);
    // Clean up.
    pax_buf_destroy(&image);
}
```
