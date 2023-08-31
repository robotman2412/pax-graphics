
#include "pax_internal.h"
#include "pax_shaders.h"

/* ===== GETTERS AND SETTERS ===== */

// Gets the index getters and setters for the given buffer.
void pax_get_setters(const pax_buf_t *buf, pax_index_getter_t *getter, pax_index_setter_t *setter) {
	switch (buf->bpp) {
		case (1):
			*getter = pax_index_getter_1bpp;
			*setter = pax_index_setter_1bpp;
			break;
			
		case (2):
			*getter = pax_index_getter_2bpp;
			*setter = pax_index_setter_2bpp;
			break;
			
		case (4):
			*getter = pax_index_getter_4bpp;
			*setter = pax_index_setter_4bpp;
			break;
			
		case (8):
			*getter = pax_index_getter_8bpp;
			*setter = pax_index_setter_8bpp;
			break;
			
		case (16):
			if (buf->reverse_endianness) {
				*getter = pax_index_getter_16bpp_rev;
				*setter = pax_index_setter_16bpp_rev;
			} else {
				*getter = pax_index_getter_16bpp;
				*setter = pax_index_setter_16bpp;
			}
			break;
			
		case (32):
			if (buf->reverse_endianness) {
				*getter = pax_index_getter_32bpp_rev;
				*setter = pax_index_setter_32bpp_rev;
			} else {
				*getter = pax_index_getter_32bpp;
				*setter = pax_index_setter_32bpp;
			}
			break;
	}
}

// Gets a raw value from a 1BPP buffer.
pax_col_t pax_index_getter_1bpp(const pax_buf_t *buf, int index) {
	return (buf->buf_8bpp[index >> 3] >> (index & 7)) & 1;
}

// Gets a raw value from a 2BPP buffer.
pax_col_t pax_index_getter_2bpp(const pax_buf_t *buf, int index) {
	return (buf->buf_8bpp[index >> 2] >> (index & 3) * 2) & 3;
}

// Gets a raw value from a 4BPP buffer.
pax_col_t pax_index_getter_4bpp(const pax_buf_t *buf, int index) {
	return (buf->buf_8bpp[index >> 1] >> (index & 1) * 4) & 15;
}

// Gets a raw value from a 8BPP buffer.
pax_col_t pax_index_getter_8bpp(const pax_buf_t *buf, int index) {
	return buf->buf_8bpp[index];
}

// Gets a raw value from a 16BPP buffer.
pax_col_t pax_index_getter_16bpp(const pax_buf_t *buf, int index) {
	return buf->buf_16bpp[index];
}

// Gets a raw value from a 32BPP buffer.
pax_col_t pax_index_getter_32bpp(const pax_buf_t *buf, int index) {
	return buf->buf_32bpp[index];
}

// Gets a raw value from a 16BPP buffer, reversed endianness.
pax_col_t pax_index_getter_16bpp_rev(const pax_buf_t *buf, int index) {
	return pax_rev_endian_16(buf->buf_16bpp[index]);
}

// Gets a raw value from a 32BPP buffer, reversed endianness.
pax_col_t pax_index_getter_32bpp_rev(const pax_buf_t *buf, int index) {
	return pax_rev_endian_32(buf->buf_32bpp[index]);
}


// Sets a raw value from a 1BPP buffer.
void pax_index_setter_1bpp(pax_buf_t *buf, pax_col_t color, int index) {
	uint8_t *ptr = &buf->buf_8bpp[index >> 3];
	switch (index & 7) {
		case (0): *ptr = (*ptr & 0xfe) | (color << 0); break;
		case (1): *ptr = (*ptr & 0xfd) | (color << 1); break;
		case (2): *ptr = (*ptr & 0xfb) | (color << 2); break;
		case (3): *ptr = (*ptr & 0xf7) | (color << 3); break;
		case (4): *ptr = (*ptr & 0xef) | (color << 4); break;
		case (5): *ptr = (*ptr & 0xdf) | (color << 5); break;
		case (6): *ptr = (*ptr & 0xbf) | (color << 6); break;
		case (7): *ptr = (*ptr & 0x7f) | (color << 7); break;
	}
}

// Sets a raw value from a 2BPP buffer.
void pax_index_setter_2bpp(pax_buf_t *buf, pax_col_t color, int index) {
	uint8_t *ptr = &buf->buf_8bpp[index >> 2]; color &= 3;
	switch (index & 3) {
		case (0): *ptr = (*ptr & 0xfc) | (color << 0); break;
		case (1): *ptr = (*ptr & 0xf3) | (color << 2); break;
		case (2): *ptr = (*ptr & 0xcf) | (color << 4); break;
		case (3): *ptr = (*ptr & 0x3f) | (color << 6); break;
	}
}

// Sets a raw value from a 4BPP buffer.
void pax_index_setter_4bpp(pax_buf_t *buf, pax_col_t color, int index) {
	uint8_t *ptr = &buf->buf_8bpp[index >> 1];
	if (index & 1) {
		*ptr = (*ptr & 0x0f) | (color << 4);
	} else {
		*ptr = (*ptr & 0xf0) | color;
	}
}

// Sets a raw value from a 8BPP buffer.
void pax_index_setter_8bpp(pax_buf_t *buf, pax_col_t color, int index) {
	buf->buf_8bpp[index] = color;
}

// Sets a raw value from a 16BPP buffer.
void pax_index_setter_16bpp(pax_buf_t *buf, pax_col_t color, int index) {
	buf->buf_16bpp[index] = color;
}

// Sets a raw value from a 32BPP buffer.
void pax_index_setter_32bpp(pax_buf_t *buf, pax_col_t color, int index) {
	buf->buf_32bpp[index] = color;
}

// Sets a raw value from a 16BPP buffer, reversed endianness.
void pax_index_setter_16bpp_rev(pax_buf_t *buf, pax_col_t color, int index) {
	buf->buf_16bpp[index] = pax_rev_endian_16(color);
}

// Sets a raw value from a 32BPP buffer, reversed endianness.
void pax_index_setter_32bpp_rev(pax_buf_t *buf, pax_col_t color, int index) {
	buf->buf_32bpp[index] = pax_rev_endian_32(color);
}


// Gets the most efficient index setter for the occasion.
// Also converts the color, if applicable.
// Returns NULL when setting is not required.
pax_index_setter_t pax_get_setter(const pax_buf_t *buf, pax_col_t *col_ptr, const pax_shader_t *shader) {
	pax_col_t col = *col_ptr;
	
	if (PAX_IS_PALETTE(buf->type)) {
		return pax_do_draw_col(buf, col) ? pax_set_index : NULL;
	}
	
	if (shader && (shader->callback == pax_shader_texture || shader->callback == pax_shader_texture_aa)) {
		// We can determine whether to factor in alpha based on buffer type.
		if (PAX_IS_ALPHA(((pax_buf_t *)shader->callback_args)->type)) {
			// If alpha needs factoring in, return the merging setter.
			return col & 0xff000000 ? pax_merge_index : NULL;
		} else {
			// If alpha doesn't need factoring in, return the converting setter.
			return col & 0xff000000 ? pax_set_index_conv : NULL;
		}
		
	} else if (shader) {
		// More generic shaders, including text.
		if (!(col & 0xff000000) && shader->alpha_promise_0) {
			// When a shader promises to have 0 alpha on 0 alpha tint, return NULL.
			return NULL;
		} else if ((col & 0xff000000) == 0xff000000 && shader->alpha_promise_255) {
			// When a shader promises to have 255 alpha on 255 alpha tint, return converting setter.
			return pax_set_index_conv;
		} else {
			// When no promises are made, fall back to the merging setter.
			return pax_merge_index;
		}
		
	} else if (!(col & 0xff000000)) {
		// If no shader and alpha is 0, don't set.
		return NULL;
		
	} else if ((col & 0xff000000) == 0xff000000) {
		// If no shader and 255 alpha, convert color and return raw setter.
		*col_ptr = buf->col2buf(buf, col);
		return buf->setter;
		
	} else {
		// If no shader and partial alpha, return merging setter.
		return pax_merge_index;
		
	}
}

// Gets based on index instead of coordinates.
// Does no bounds checking nor color conversion.
pax_col_t pax_get_index(const pax_buf_t *buf, int index) {
	return buf->getter(buf, index);
}

// Gets based on index instead of coordinates.
// Does no bounds checking.
pax_col_t pax_get_index_conv(const pax_buf_t *buf, int index) {
	return buf->buf2col(buf, buf->getter(buf, index));
}

// Sets based on index instead of coordinates.
// Does no bounds checking nor color conversion.
void pax_set_index(pax_buf_t *buf, pax_col_t color, int index) {
	buf->setter(buf, color, index);
}

// Sets based on index instead of coordinates.
// Does no bounds checking.
void pax_set_index_conv(pax_buf_t *buf, pax_col_t col, int index) {
	buf->setter(buf, buf->col2buf(buf, col), index);
}

// Merges based on index instead of coordinates. Does no bounds checking.
void pax_merge_index(pax_buf_t *buf, pax_col_t col, int index) {
	pax_col_t base = buf->buf2col(buf, buf->getter(buf, index));
	pax_col_t res  = buf->col2buf(buf, pax_col_merge(base, col));
	buf->setter(buf, res, index);
}




/* ======= COLOR CONVERSION ====== */

// Get the correct color conversion methods for the buffer type.
void pax_get_col_conv(const pax_buf_t *buf, pax_col_conv_t *col2buf, pax_col_conv_t *buf2col) {
	switch (buf->type) {
	case (PAX_BUF_1_PAL):
		*col2buf = pax_trunc_to_1;
		*buf2col = pax_pal_lookup;
		break;
		
	case (PAX_BUF_2_PAL):
		*col2buf = pax_trunc_to_2;
		*buf2col = pax_pal_lookup;
		break;
		
	case (PAX_BUF_4_PAL):
		*col2buf = pax_trunc_to_4;
		*buf2col = pax_pal_lookup;
		break;
		
	case (PAX_BUF_8_PAL):
		*col2buf = pax_trunc_to_8;
		*buf2col = pax_pal_lookup;
		break;
		
	case (PAX_BUF_16_PAL):
		*col2buf = pax_trunc_to_16;
		*buf2col = pax_pal_lookup;
		break;
		
		
	case (PAX_BUF_1_GREY):
		*col2buf = pax_col_to_1_grey;
		*buf2col = pax_1_grey_to_col;
		break;
		
	case (PAX_BUF_2_GREY):
		*col2buf = pax_col_to_2_grey;
		*buf2col = pax_2_grey_to_col;
		break;
		
	case (PAX_BUF_4_GREY):
		*col2buf = pax_col_to_4_grey;
		*buf2col = pax_4_grey_to_col;
		break;
		
	case (PAX_BUF_8_GREY):
		*col2buf = pax_col_to_8_grey;
		*buf2col = pax_8_grey_to_col;
		break;
		
		
	case (PAX_BUF_8_332RGB):
		*col2buf = pax_col_to_332_rgb;
		*buf2col = pax_332_rgb_to_col;
		break;
		
	case (PAX_BUF_16_565RGB):
		*col2buf = pax_col_to_565_rgb;
		*buf2col = pax_565_rgb_to_col;
		break;
		
		
	case (PAX_BUF_4_1111ARGB):
		*col2buf = pax_col_to_1111_argb;
		*buf2col = pax_1111_argb_to_col;
		break;
		
	case (PAX_BUF_8_2222ARGB):
		*col2buf = pax_col_to_2222_argb;
		*buf2col = pax_2222_argb_to_col;
		break;
		
	case (PAX_BUF_16_4444ARGB):
		*col2buf = pax_col_to_4444_argb;
		*buf2col = pax_4444_argb_to_col;
		break;
		
	case (PAX_BUF_32_8888ARGB):
		*col2buf = pax_col_conv_dummy;
		*buf2col = pax_col_conv_dummy;
		break;
		
	}
}


// Dummy color converter, returns color input directly.
pax_col_t pax_col_conv_dummy(const pax_buf_t *buf, pax_col_t color) {
	return color;
}

// Truncates input to 1 bit.
pax_col_t pax_trunc_to_1(const pax_buf_t *buf, pax_col_t color) {
	return color & 1;
}

// Truncates input to 2 bit.
pax_col_t pax_trunc_to_2(const pax_buf_t *buf, pax_col_t color) {
	return color & 3;
}

// Truncates input to 4 bit.
pax_col_t pax_trunc_to_4(const pax_buf_t *buf, pax_col_t color) {
	return color & 15;
}

// Truncates input to 8 bit.
pax_col_t pax_trunc_to_8(const pax_buf_t *buf, pax_col_t color) {
	return color & 255;
}

// Truncates input to 16 bit.
pax_col_t pax_trunc_to_16(const pax_buf_t *buf, pax_col_t color) {
	return color & 65535;
}



// Converts ARGB to 1-bit greyscale (AKA black/white).
pax_col_t pax_col_to_1_grey(const pax_buf_t *buf, pax_col_t color) {
	uint_fast16_t total =  (color & 0x0000ff)
						+ ((color & 0x00ff00) >> 8)
						+ ((color & 0xff0000) >> 16);
	return total > 128*3;
}

// Converts ARGB to 2-bit greyscale.
pax_col_t pax_col_to_2_grey(const pax_buf_t *buf, pax_col_t color) {
	uint_fast8_t total = ((color & 0x0000c0) >> 6)
					   + ((color & 0x00c000) >> 14)
					   + ((color & 0xc00000) >> 22);
	return total / 3;
}

// Converts ARGB to 4-bit greyscale.
pax_col_t pax_col_to_4_grey(const pax_buf_t *buf, pax_col_t color) {
	uint_fast8_t total = ((color & 0x0000f0) >> 4)
					   + ((color & 0x00f000) >> 12)
					   + ((color & 0xf00000) >> 20);
	return total / 3;
}

// Converts ARGB to 8-bit greyscale.
pax_col_t pax_col_to_8_grey(const pax_buf_t *buf, pax_col_t color) {
	uint_fast16_t total =  (color & 0x0000ff)
						+ ((color & 0x00ff00) >> 8)
						+ ((color & 0xff0000) >> 16);
	return total / 3;
}



// Converts ARGB to 3, 3, 2 bit RGB.
pax_col_t pax_col_to_332_rgb(const pax_buf_t *buf, pax_col_t color) {
	// 8BPP 332-RGB
	// From: Aaaa aaaa Rrrr rrrr Gggg gggg Bbbb bbbb
	// To:                                 RrrG ggBb
	uint16_t value = ((color >> 16) & 0xe0) | ((color >> 11) & 0x1c) | ((color >> 6) & 0x03);
	return value;
}

// Converts ARGB to 5, 6, 5 bit RGB.
pax_col_t pax_col_to_565_rgb(const pax_buf_t *buf, pax_col_t color) {
	// 16BPP 565-RGB
	// From: Aaaa aaaa Rrrr rrrr Gggg gggg Bbbb bbbb
	// To:                       Rrrr rGgg gggB bbbb
	return ((color >> 8) & 0xf800) | ((color >> 5) & 0x07e0) | ((color >> 3) & 0x001f);
}



// Converts ARGB to 1 bit per channel ARGB.
pax_col_t pax_col_to_1111_argb(const pax_buf_t *buf, pax_col_t color) {
	// 4BPP 1111-ARGB
	// From: Aaaa aaaa Rrrr rrrr Gggg gggg Bbbb bbbb
	// To:                                      ARGB
	uint16_t value = ((color >> 28) & 0x8) | ((color >> 21) & 0x4) | ((color >> 14) & 0x2) | ((color >> 7) & 0x1);
	return value;
}

// Converts ARGB to 2 bit per channel ARGB.
pax_col_t pax_col_to_2222_argb(const pax_buf_t *buf, pax_col_t color) {
	// 8BPP 2222-ARGB
	// From: Aaaa aaaa Rrrr rrrr Gggg gggg Bbbb bbbb
	// To:                                 AaRr GgBb
	uint16_t value = ((color >> 24) & 0xc0) | ((color >> 18) & 0x30) | ((color >> 12) & 0x0c) | ((color >> 6) & 0x03);
	return value;
}

// Converts ARGB to 4 bit per channel ARGB.
pax_col_t pax_col_to_4444_argb(const pax_buf_t *buf, pax_col_t color) {
	// 16BPP 4444-ARGB
	// From: Aaaa aaaa Rrrr rrrr Gggg gggg Bbbb bbbb
	// To:                       Aaaa Rrrr Gggg Bbbb
	return ((color >> 16) & 0xf000) | ((color >> 12) & 0x0f00) | ((color >> 8) & 0x00f0) | ((color >> 4) & 0x000f);
}


// Performs a palette lookup based on the input.
pax_col_t pax_pal_lookup(const pax_buf_t *buf, pax_col_t index) {
	return (index >= buf->palette_size)
		   ? *buf->palette
		   :  buf->palette[index];
}



// Converts 1-bit greyscale (AKA black/white) to ARGB.
pax_col_t pax_1_grey_to_col(const pax_buf_t *buf, pax_col_t color) {
	return color ? 0xffffffff : 0xff000000;
}

// Converts 2-bit greyscale to ARGB.
pax_col_t pax_2_grey_to_col(const pax_buf_t *buf, pax_col_t color) {
	static const pax_col_t arr[] = {
		0xff000000,
		0xff555555,
		0xffaaaaaa,
		0xffffffff,
	};
	return arr[color];
}

// Converts 4-bit greyscale to ARGB.
pax_col_t pax_4_grey_to_col(const pax_buf_t *buf, pax_col_t color) {
	return 0xff000000 | (color * 0x00111111);
}

// Converts 8-bit greyscale to ARGB.
pax_col_t pax_8_grey_to_col(const pax_buf_t *buf, pax_col_t color) {
	return 0xff000000 | (color * 0x00010101);
}



// Converts 3, 3, 2 bit RGB to ARGB.
pax_col_t pax_332_rgb_to_col(const pax_buf_t *buf, pax_col_t value) {
	// 8BPP 332-RGB
	// From:                               RrrG ggBb
	// To:   .... .... Rrr. .... Ggg. .... .... ....
	// Add:  .... .... ...R rrRr ...Gg gGg .... ....
	// Add:  .... .... .... .... .... .... BbBb BbBb
	pax_col_t color = ((value << 16) & 0x00e00000) | ((value << 11) & 0x0000e000);
	color |= (color >> 3) | ((color >> 6) & 0x000f0f00);
	pax_col_t temp  = (value & 0x03);
	temp |= temp << 2;
	color |= temp | (temp << 4);
	return color | 0xff000000;
}

// Converts 5, 6, 5 bit RGB to ARGB.
pax_col_t pax_565_rgb_to_col(const pax_buf_t *buf, pax_col_t value) {
	// 16BPP 565-RGB
	// From:                     Rrrr rGgg gggB bbbb
	// To:   .... .... Rrrr r... Gggg gg.. Bbbb b...
	// Add:  .... .... .... .Rrr .... ..Gg .... .Bbb
	// Take the existing information.
	pax_col_t color = ((value << 8) & 0x00f80000) | ((value << 5) & 0x0000fc00) | ((value << 3) & 0x000000f8);
	// Now, fill in some missing bits.
	color |= ((value << 3) & 0x00070000) | ((value >> 1) & 0x00000300) | ((value >> 2) & 0x00000007);
	return color | 0xff000000;
}



// Converts 1 bit per channel ARGB to ARGB.
pax_col_t pax_1111_argb_to_col(const pax_buf_t *buf, pax_col_t value) {
	// 4BPP 1111-ARGB
	// From:                                    ARGB
	// To:   Aaaa aaaa Rrrr rrrr Gggg gggg Bbbb bbbb
	pax_col_t color = ((value << 28) & 0x80000000) | ((value << 21) & 0x00800000) | ((value << 14) & 0x00008000) | ((value << 7) & 0x00000080);
	color |= color >> 1;
	color |= color >> 2;
	color |= color >> 4;
	return color;
}

// Converts 2 bit per channel ARGB to ARGB.
pax_col_t pax_2222_argb_to_col(const pax_buf_t *buf, pax_col_t value) {
	// 8BPP 2222-ARGB
	// From:                               AaRr GgBb
	// To:   Aaaa aaaa Rrrr rrrr Gggg gggg Bbbb bbbb
	pax_col_t color = ((value << 24) & 0xc0000000) | ((value << 18) & 0x00c00000) | ((value << 12) & 0x0000c000) | ((value << 6) & 0x000000c0);
	color |= color >> 2;
	color |= color >> 4;
	return color;
}

// Converts 4 bit per channel ARGB to ARGB.
pax_col_t pax_4444_argb_to_col(const pax_buf_t *buf, pax_col_t value) {
	// 16BPP 4444-ARGB
	// From:                     Aaaa Rrrr Gggg Bbbb
	// To:   Aaaa .... Rrrr .... Gggg .... Bbbb ....
	// Add:  .... Aaaa .... Rrrr .... Gggg .... Bbbb
	pax_col_t color = ((value << 16) & 0xf0000000) | ((value << 12) & 0x00f00000) | ((value << 8) & 0x0000f000) | ((value << 4) & 0x000000f0);
	// Now fill in some missing bits.
	color |= color >> 4;
	return color;
}
