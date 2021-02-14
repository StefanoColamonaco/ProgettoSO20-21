SHELL = /bin/sh
VPATH = /usr/local/share/umps3:/usr/share/umps3
CC = mipsel-linux-gnu-gcc
umps_flags = -ffreestanding -ansi -mips1 -mabi=32 -mno-gpopt -G 0 -mno-abicalls -fno-pic -mfp32
umps_headers = -I/usr/include  -I/usr/local/include/umps3 -I.

CFLAGS = $(umps_flags) $(umps_headers) -std=c17 -Wall -O0
LD = mipsel-linux-gnu-ld
LDFLAGS = -G 0 -nostdlib -T/usr/share/umps3/umpscore.ldscript
AS = mipsel-linux-gnu-gcc
.SUFFIXES:
.SUFFIXES: .h .c .o .S .umps

pandos_headers = pandos_const.h pandos_types.h
objects = pcb.o p1test.o

kernel.core.umps kernel.stab.umps : kernel
	umps3-elf2umps -k kernel

kernel : crtso.o libumps.o $(objects)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

p1test.o : p1test.c
	$(CC) $(CFLAGS) -c -o $@ $^

pcb.o : pcb.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^
libumps.o : libumps.S
	$(CC) $(CFLAGS) -c -o $@ $^
crtso.o : crtso.S
	$(CC) $(CFLAGS) -c -o $@ $^


