SHELL = /bin/sh
.SUFFIXES:
.SUFFIXES: .h .c .o
CFLAGS =

pandos_headers = pandos_const.h pandos_types.h
objects =



.PHONY: clean
clean:
	rm $(objects)