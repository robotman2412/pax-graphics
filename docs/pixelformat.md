# PAX Docs: Pixel format

This document focusses on the details of how PAX internally stores image data:
- [Summary](#summary)
- [Pixel types](#pixel-types)
  - [Greyscale](#pixel-types-greyscale)
  - [Palette](#pixel-types-palette)
  - [RGB](#pixel-types-rgb)
  - [ARGB](#pixel-types-argb)
- [Image data format](#image-data-format)
  - [Storage methods](#storage-methods)
  - [Endianness](#endianness)



# Summary

The *image data* is the binary representation of the image that has been drawn.
It consists of an array of *pixels*, enough to fill the area of the buffer.

A *pixel* has some amount of bits per pixel (1, 2, 4, 8, 16 or 32).
The pixels are arranges left-to-right, then top-to-bottom, packed.
Packed means that there are no gaps anywhere in the buffer; neither between rows nor columns.



# Pixel types

PAX currently supports a small handfull of pixel types, which you can choose at runtime.
For color math, 32-bit ARGB is used with the exception of palette types, for which no color math is done.
The colors are converted to the appropriate pixel type before storing them to the image data.

For all but one of the pixel types, there is conversion required, which is a form of overhead.
Additionally, pixel types less than 8 bits per pixel have the overhead of needing sub-byte addressing.

These fall into four categories:
- [Greyscale](#pixel-types-greyscale)
- [Palette](#pixel-types-palette)
- [RGB](#pixel-types-rgb)
- [ARGB](#pixel-types-argb)


## Pixel types: Greyscale

Greyscale pixel types can have between 1 and 8 bits per pixel:
| type           | bits per pixel
| :------------- | :-------------
| PAX_BUF_1_GREY | 1
| PAX_BUF_2_GREY | 2
| PAX_BUF_4_GREY | 4
| PAX_BUF_8_GREY | 8

The convertion from ARGB to greyscale is `greyscale = (red + green + blue) / 3 * resolution / 255`,
where resolution is the maximum unsigned integer value that fits in the amount of bits per pixel.

The convertion from greyscale to ARGB is `alpha = 255`, `red = green = blue = greyscale * 255 / resolution`.


## Pixel types: Palette

Palette pixel types can have between 1 and 8 bits per pixel:
| type           | bits per pixel
| :------------- | :-------------
| PAX_BUF_1_PAL  | 1
| PAX_BUF_2_PAL  | 2
| PAX_BUF_4_PAL  | 4
| PAX_BUF_8_PAL  | 8
| PAX_BUF_16_PAL | 16

There is no color conversion done, simply tructation.

However, this does not mean that palette colors are simply one integer;
Palette colors are treated as *fully opaque* the color index in within the range in the palette (which means N < M where N is color index and M is amount of palette entries). If it is not in this range, the color is treated as *fully transparent*. This means that a `pax_col_t` value of 0x10000 to 0xff0000 is treated as transparent by both ARGB and palette color math.

## Pixel types: RGB

RGB pixel types can have 8 or 16 bits per pixel:
| type              | bits per pixel | bits R | bits G | bits B
| :---------------- | :------------- | :----- | :----- | :-----
| PAX_BUF_8_332RGB  | 8              | 3      | 3      | 2
| PAX_BUF_16_565RGB | 16             | 5      | 6      | 5
| PAX_BUF_24_888RGB | 24             | 8      | 8      | 8

The conversion is a simple scaling of the color values to fit the new bits for the respective channel.
Additionally, alpha is *ignored* when converting to the pixel format, and set to 255 when converting to ARGB.

## Pixel types: ARGB

ARGB pixel types can have 4 to 32 bits per pixel
| type                | bits per pixel | bits per channel
| :------------------ | :------------- | :---------------
| PAX_BUF_4_1111ARGB  | 4              | 1
| PAX_BUF_8_2222ARGB  | 8              | 2
| PAX_BUF_16_4444ARGB | 16             | 4
| PAX_BUF_32_8888ARGB | 32             | 8

As ARGB is the type in which color math is performed, conversion is minimal; simply scaling the values to fit the new bits per channel.
Additionally, `PAX_BUF_32_8888ARGB` represents the same pixel type as the ARGB colors in `pax_col_t` and no conversion is required.



# Image data format

The *image data* is the binary representation of the image that has been drawn.
It consists of an array of *pixels*, enough to fill the area of the buffer.

The data of the pixels is stored in memory left-to-right, then top-to-bottom.
This means that the Nth row has pixel index `N*width`.
Extrapolating, to convert co-ordinates to pixel indices: `index = x + y * width`.

This type of storage is achieved though index-based *getters* and *setters*,
which have the task of retrieving and storing raw pixel data respectively.


## Storage methods

There is a pair of one *getter* and one *setter* for each valid amount of *bits per pixel*:
| bits per pixel | getter/setter strategy
| :------------- | :---------------------
| 1              | Sub-byte (8 pixels/byte)
| 2              | Sub-byte (4 pixels/byte)
| 4              | Sub-byte (2 pixels/byte)
| 8              | Byte units
| 16             | Two-byte units, endianness conversion
| 32             | Four-byte units, endianness conversion

The sub-byte addressing method packs the first pixel as the first bits in the byte,
making the sub-byte addressing *little-endian*, which cannot be changed.
There is no exceptions for crossing rows; If the amount of columns is not in integer multiple of pixels per byte, then there are bytes in the image data which have pixels from multiple rows.

The single-byte addressing method simply indexes bytes without further conversion.

Finally, more than one byte means potential *endianness conversion*.
When `pax_buf_reversed(true)` is called,
endianness conversion will happen on both reads from and writes to the pixel buffer.
Depending on architecture, this can incur a few instructions worth of overhead.


## Endianness

As mentioned earlier, *endianness conversion* is an option in the buffer set-up functions.
This option applies only to 16- and 32-bit pixel formats, for less then that it is ignored.

When enabled, the pixel data is stored in the opposite endianness when compared to the CPU:
Endianness is converted by reversing the order of the bytes making up the pixel data, which makes this a CPU-agnostic operation.
