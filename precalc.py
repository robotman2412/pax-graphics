#!/usr/bin/env python3

import math

def precalcCircle(fd, resolution, id):
	delta = math.pi * 2 / resolution
	
	# Precalculate points.
	fd.write("// Circle: " + str(resolution) + " segments\n")
	fd.write("const pax_vec1_t pax_precalc_" + id + "[" + str(resolution + 1) + "] = {\n")
	for i in range(resolution + 1):
		angle = i * delta
		fd.write("\t{" + str(math.cos(angle)) + ", " + str(math.sin(angle)) + "},\n")
	fd.write("};\n\n")
	
	# Precalculate UVs.
	fd.write("// Circle UVs: " + str(resolution) + " segments\n")
	fd.write("const pax_tri_t pax_precalc_uv_" + id + "[" + str(resolution - 2) + "] = {\n")
	for i in range(resolution - 1):
		a1 =  i      * delta
		a2 = (i + 1) * delta
		fd.write("\t{1, 0.5, ")
		fd.write(str(math.cos(a1)/2+0.5) + ", " + str(math.sin(a1)/2+0.5) + ", ")
		fd.write(str(math.cos(a2)/2+0.5) + ", " + str(math.sin(a2)/2+0.5))
		fd.write("},\n")
	fd.write("};\n\n")

if __name__ == "__main__":
	out_fd = open("src/helpers/precalculated.c", "w")
	out_fd.write("// WARNING: This is a generated file, do not edit it!\n")
	out_fd.write("// This file contains precalculated math operations.\n\n")
	out_fd.write("#define PAX_GFX_C\n")
	out_fd.write("#ifndef PAX_GFX_C\n")
	out_fd.write("#ifndef ARDUINO\n")
	out_fd.write("#pragma message \"This file should not be compiled on it's own.\"\n")
	out_fd.write("#endif\n")
	out_fd.write("#else\n\n")
	out_fd.write("#include \"../pax_internal.h\"\n\n")
	precalcCircle(out_fd,  4, "circle_4")
	precalcCircle(out_fd, 16, "circle_16")
	precalcCircle(out_fd, 24, "circle_24")
	precalcCircle(out_fd, 32, "circle_32")
	out_fd.write("#endif\n")
