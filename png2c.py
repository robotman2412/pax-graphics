#!/usr/bin/env python3

import re
from sys import argv
from PIL import Image



def f2i(f: float, range: int) -> int:
	"""Converts float to int where 0<=f<=1."""
	return range-1 if f >= 1 else int(f * range)



def cf2i(f: tuple, bpc: int) -> int:
	"""Converts a float ARGB color into a packed integer."""
	# Convert individual channels.
	a = f2i(f[3]/255, 1 << bpc)
	r = f2i(f[0]/255, 1 << bpc)
	g = f2i(f[1]/255, 1 << bpc)
	b = f2i(f[2]/255, 1 << bpc)
	# Pack together components.
	return  (a << (3*bpc)) |\
			(r << (2*bpc)) |\
			(g << (1*bpc)) |\
			(b << (0*bpc))



def imageBits(img: Image.Image, bpc: int = 4) -> list:
	"""Reads image data from `img` and converts into a PAX ARGB image data."""
	# BPC check.
	if bpc not in [2, 4, 8]:
		raise ValueError("Bits per channel must be 2, 4 or 8.")
	
	# Iterate over pixel data.
	img_data = img.load()
	out = []
	for y in range(img.height):
		for x in range(img.width):
			out.append(cf2i(img_data[x, y], bpc))
	
	# Output packed image data.
	return out



def process(in_path, out_path=None, name=None, bpc=4, cols=32, is_const=True):
	"""Reads a PNG image from `in_path` and turns it into a C constant in `out_path`."""
	bpc=int(bpc)
	cols=int(cols)
	is_const=bool(is_const)
	
	# Open the file handles.
	if out_path == "-":
		out_path = "/proc/self/fd/1"
		out_fd = open(out_path, "a")
	elif out_path == None:
		out_path = in_path + ".c"
		out_fd = open(out_path, "w")
	else:
		out_fd = open(out_path, "w")
	
	# Convert name.
	if name == None:
		name = in_path
	name = re.sub(r'\W+', '_', name)
	
	# Read input data.
	img  = Image.open(in_path)
	bits = imageBits(img, bpc)
	if bpc == 2:
		itype = "uint8_t"
		buf = "PAX_BUF_8_2222ARGB"
	elif bpc == 4:
		itype = "uint16_t"
		buf = "PAX_BUF_16_4444ARGB"
	else: #  bpc == 8
		itype = "uint32_t"
		buf = "PAX_BUF_32_8888ARGB"
	
	# Write headers.
	out_fd.write("\n")
	out_fd.write("// WARNING: This is a generated file, do not edit it!\n")
	out_fd.write("\n")
	out_fd.write("#include <pax_gfx.h>\n")
	out_fd.write("\n")
	
	# Write the array.
	out_fd.write("const size_t {}_width = {};\n".format(name, img.width))
	out_fd.write("const size_t {}_height = {};\n".format(name, img.height))
	if is_const:
		out_fd.write("const ")
	out_fd.write("{} {}_bits[{}] = {{\n".format(itype, name, img.width*img.height))
	
	# Write raw data.
	for y in range(0, len(bits), cols):
		out_fd.write('\t')
		for x in range(y, min(y+cols, len(bits))):
			out_fd.write("{},".format(hex(bits[x])))
		out_fd.write('\n')
	
	# End of the array.
	out_fd.write("};\n\n")
	
	# Write initialiser.
	out_fd.write("void {}_init(pax_buf_t *out) {{\n".format(name))
	out_fd.write("\tpax_buf_init(out, {}_bits, {}, {}, {});\n".format(name, img.width, img.height, buf))
	out_fd.write("}\n")
	
	# End of file.
	out_fd.close()



if __name__ == "__main__":
	if len(argv) >= 3 and len(argv) <= 7:
		process(*(argv[1:]))
	else:
		print("Usage: png2c.py <infile> (outfile.c) (name=infile) (bpc=4) (columns=32) (is_const=true)")
