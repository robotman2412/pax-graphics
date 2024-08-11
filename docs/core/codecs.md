# PAX Docs: Image codecs
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
| bool    | pax_insert_png_fd  | pax_buf_t \*buf, FILE \*fd, int x, int y, int flags
| bool    | pax_insert_png_buf | pax_buf_t \*buf, const void \*png, size_t png_len, int x, int y, int flags

Both `pax_decode_png_` methods will decode the PNG into `buf`, which must be uninitialised.
The `buf_type` argument is merely a hint, it is not always met perfectly.

The methods return `true` on success, in which case they are like a more complex `pax_buf_init`
because they initialise the buffer and then fill it with an image.
This means that when you're done, you must call `pax_buf_destroy` to free up the allocated memory.

The `pax_insert_png_` methods will do the same, but they draw straight into an existing buffer.
You can of course set the position of where it does this.

An example of drawing a PNG in PAX:
```c
void draw_my_png(pax_buf_t *draw_to, void *png_data, size_t png_data_len) {
    // Decode a PNG straight into the buffer.
    pax_insert_png_buf(draw_to, png_data, png_data_len, 0, 0, 0);
}
```
