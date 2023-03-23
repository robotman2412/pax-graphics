#!/usr/bin/env python3

import re
from sys import argv

def process(in_path, out_path=None, name=None, cols=32, is_const=True):
	"""Reads binary data from `in_path` and turns it into a C constant in `out_path`."""
	# Open the file handles.
	in_fd  = open(in_path, "rb")
	if out_path == "-":
		out_path = "/proc/self/fd/1"
	elif out_path == None:
		out_path = in_path + ".c"
	out_fd = open(out_path, "w")
	
	# Convert name.
	if name == None:
		name = in_path
	name = re.sub(r'\W+', '_', name)
	
	# Read input data.
	raw = in_fd.read()
	in_fd.close()
	
	# Write headers.
	out_fd.write("\n")
	out_fd.write("// WARNING: This is a generated file, do not edit it!\n")
	out_fd.write("\n")
	out_fd.write("#include <stdint.h>\n")
	out_fd.write("#include <stddef.h>\n")
	out_fd.write("\n")
	
	# Write the array.
	out_fd.write("const size_t {}_len = {};\n".format(name, len(raw)))
	if is_const:
		out_fd.write("const ")
	out_fd.write("uint8_t {}[{}] = {{\n".format(name, len(raw)))
	
	# Write raw data.
	for y in range(0, len(raw), cols):
		out_fd.write('\t')
		for x in range(y, min(y+cols, len(raw))):
			out_fd.write("{},".format(hex(raw[x])))
		out_fd.write('\n')
	
	# End of the file.
	out_fd.write("};\n")
	out_fd.close()

if __name__ == "__main__":
	if len(argv) >= 3 and len(argv) <= 6:
		process(*(argv[1:]))
	else:
		print("Usage: embed2c.py <infile> (outfile.c) (name=infile) (columns=32) (is_const=true)")
