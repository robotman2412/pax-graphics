#!/usr/bin/env python3

import math

def precalcCircle(fd_c, fd_h, resolution, id):
    delta = math.pi * 2 / resolution
    
    # Precalculate points.
    fd_h.write("// Circle: " + str(resolution) + " segments\n")
    fd_h.write("extern const pax_vec2f pax_precalc_" + id + "[" + str(resolution + 1) + "];\n")
    fd_c.write("// Circle: " + str(resolution) + " segments\n")
    fd_c.write("const pax_vec2f pax_precalc_" + id + "[" + str(resolution + 1) + "] = {\n")
    for i in range(resolution + 1):
        angle = i * delta + delta / 2
        fd_c.write("\t{" + str(math.cos(angle)) + ", " + str(math.sin(angle)) + "},\n")
    fd_c.write("};\n\n")
    
    # Precalculate UVs.
    fd_h.write("// Circle UVs: " + str(resolution) + " segments\n")
    fd_h.write("extern const pax_trif pax_precalc_uv_" + id + "[" + str(resolution - 1) + "];\n\n")
    fd_c.write("// Circle UVs: " + str(resolution) + " segments\n")
    fd_c.write("const pax_trif pax_precalc_uv_" + id + "[" + str(resolution - 1) + "] = {\n")
    for i in range(resolution - 1):
        a1 =  i      * delta + delta / 2
        a2 = (i + 1) * delta + delta / 2
        fd_c.write("\t{1, 0.5, ")
        fd_c.write(str(math.cos(a1)/2+0.5) + ", " + str(math.sin(a1)/2+0.5) + ", ")
        fd_c.write(str(math.cos(a2)/2+0.5) + ", " + str(math.sin(a2)/2+0.5))
        fd_c.write("},\n")
    fd_c.write("};\n\n")

if __name__ == "__main__":
    out_c = open("src/helpers/pax_precalculated.c", "w")
    out_c.write("// WARNING: This is a generated file, do not edit it!\n")
    out_c.write("// This file contains precalculated math operations.\n")
    out_c.write("// clang-format off\n\n")
    out_c.write("#include \"pax_internal.h\"\n\n")
    out_h = open("src/helpers/pax_precalculated.h", "w")
    out_h.write("// WARNING: This is a generated file, do not edit it!\n")
    out_h.write("// This file contains precalculated math operations.\n")
    out_c.write("// clang-format off\n\n")
    out_h.write("#include \"pax_types.h\"\n\n")
    precalcCircle(out_c, out_h,  8, "circle_8")
    precalcCircle(out_c, out_h, 16, "circle_16")
    precalcCircle(out_c, out_h, 24, "circle_24")
    precalcCircle(out_c, out_h, 32, "circle_32")
    out_c.flush()
    out_c.close()
    out_h.flush()
    out_h.close()
