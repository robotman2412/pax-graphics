#!/usr/bin/env python3

import os



def merge_dicts(a: dict, b: dict) -> dict:
	"""Merges two tables, raising ValueError if there is a duplicate key."""
	out = {}
	for key in a:
		out[key] = a[key]
	for key in b:
		if key in out:
			raise ValueError("Duplicate key " + str(key))
		out[key] = b[key]
	return out

def is_space_char(input: str) -> bool:
	return ord(input) <= 0x20

def is_space(input: str) -> bool:
	for char in input:
		if not is_space_char(char):
			return False
	return True

def is_word_char(input: str) -> bool:
	return (input >= 'a' and input <= 'z') \
		or (input >= 'A' and input <= 'A') \
		or (input >= '0' and input <= '9') \
		or  input == '_'

def is_word(input: str) -> bool:
	for char in input:
		if not is_word_char(char):
			return False
	return True

def read_word(fd) -> str:
	out: str = ''
	while True:
		char: str = fd.read(1)
		if not is_word_char(char) and not (char is '' and is_space_char(char)):
			if char is not '':
				fd.seek(fd.tell()-1)
			return out
		elif not is_space_char(char):
			out += char

def read_until_char(fd, what: str) -> str:
	out: str = ''
	while True:
		char: str = fd.read(1)
		if char is what:
			return out
		elif char is '':
			raise SyntaxError("Found End-Of-File before " + what)
		else:
			out += char



def process_if(in_fd, vars: dict, out_fd, end: str = None) -> None:
	"""Process an if directive."""
	key = read_word(in_fd)
	if key is '':
		raise SyntaxError("Expected KEY after if")
	if vars[key]:
		process_code(in_fd, vars, out_fd, end)
	else:
		read_until_char(in_fd, end)

def process_each(in_fd, vars: dict, out_fd, end: str = None) -> None:
	"""Process an each directive."""

def process_code(in_fd, vars: dict, out_fd, end: str = None) -> None:
	"""Process a bunch of code."""
	while True:
		char: str = in_fd.read(1)
		if char is '' or char is end:
			# End condition.
			return
			
		elif char is '$':
			# Replacements.
			char = in_fd.read(1)
			
			if char is '{':
				# Lookup replacement.
				key = read_word(in_fd)
				if not is_space(read_until_char(in_fd, '}')):
					raise SyntaxError("Non-word character in key")
				out_fd.write(str(vars[key]))
				
			elif char is '[':
				# Complex replacement.
				mode = read_word(in_fd)
				if mode is 'if':
					process_if(in_fd, vars, out_fd, ']')
				elif mode is 'each':
					process_each(in_fd, vars, out_fd, ']')
				elif mode is '':
					raise SyntaxError("Expected a mode after $[")
				else:
					raise SyntaxError("Unknown directive " + mode)
			
		else:
			# Normal stuff.
			out_fd.write(char)

def process(in_fd, out_fd) -> None:
	vars = {
		'pre':			'',
		'post':			'',
		'inc_y':		1,
		'interp_x':		[['u0','u1','u']],
		'interp_y':		[['v0','v1','v']],
		'update_pixel':	'',
		'mcr':			False
	}
	in_str = in_fd.read()
	process_code(in_fd, vars, out_fd)



if __name__ == "__main__":
	in_fd  = open("dh_test.c", "r")
	out_fd = open("/tmp/dh_test.c", "w")
	process(in_fd, out_fd)
	in_fd.close()
	out_fd.close()
	os.system("cat /tmp/dh_test.c")
