SHELL = /bin/sh
VPATH = /usr/local/share/umps3
CC = mipsel-linux-gnu-gcc
CFLAGS = $(umps_flags) $(umps_headers) -O0
LD = mipsel-linux-gnu-ld
LDFLAGS = -G 0 -nostdlib -T ldscript
AS = mipsel-linux-gnu-gcc
.SUFFIXES:
.SUFFIXES: .h .c .o .S .umps

umps_flags = -ffreestanding -ansi -mip1 -mabi=32 -mno-gpopt -G 0 -mno-abicalls -fno-pic -mfp32
umps_headers = -I /usr/local/include/umps3

pandos_headers = pandos_const.h pandos_types.h
objects = #qua vanno i file oggetto che compongono kernel e per ora non abbiamo

kernel.core.umps kernel.stab.umps &: kernel
	umps3-elf2umps -k kernel

kernel : crtso.o libumps.o $(objects)

libumps.o : libumps.S

crtso.o : crtso.S

.PHONY: clean
clean:
	rm kernel $(objects)