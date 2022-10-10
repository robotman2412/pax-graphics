#!/usr/bin/env python


def read_file(path):
	""" Read a file, excluding the license text. """
	fd = open("src/" + path, "r")
	str = fd.read().splitlines()
	fd.close()
	return str


def pack_file(in_path: str, out_path: str):
	data = read_file(in_path)


if __name__ == "__main__":
	pack_file("pax_config.h", "pax_packed_header.h")
