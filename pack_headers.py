#!/usr/bin/env python

# This file sux lol

def read_file(path):
	""" Read a file, excluding the license text. """
	fd = open("src/" + path, "r")
	str = fd.read().splitlines()
	try:
		if str.index('/*') == 0:
			str = str[str.index('*/') + 1:]
	except ValueError:
		pass
	fd.close()
	return str


def pack_file(in_path: str, out_fd, included: list):
	if in_path not in included:
		included.append(in_path)
	data = read_file(in_path)
	
	for line in data:
		if line.startswith("#include "):
			if (line[9] == "<" and line[-1] != ">") and (line[9] != '"' or line[-1] != '"'):
				raise ValueError("Syntax error in an #include statement")
			next = line[10:-1]
			if next not in included:
				included.append(next)
				try:
					pack_file(next, out_fd, included)
					print(next + " included")
				except FileNotFoundError:
					print(next + " skipped")
					out_fd.write(line + "\n")
		else:
			out_fd.write(line + "\n")
				


if __name__ == "__main__":
	license_fd = open("LICENSE", "r")
	out_fd = open("pax_packed_header.h", "w")
	out_fd.write(license_fd.read())
	out_fd.write("\n// WARNING: This is a generated file, do not edit it!\n")
	out_fd.write("// This file serves as an API to use for any applications which want to use PAX but do not want to include all it's headers.\n")
	out_fd.write("// The version ID in this file corresponds to the version of PAX it was generated from.\n")
	pack_file("pax_gfx.h", out_fd, [])
