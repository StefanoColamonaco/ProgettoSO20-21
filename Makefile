SHELL = /bin/sh
VPATH = /usr/local/share/umps3:/usr/share/umps3
CC = mipsel-linux-gnu-gcc

umps_flags = -ffreestanding -ansi -mips1 -mabi=32 -mno-gpopt -G 0 -mno-abicalls -fno-pic -mfp32
umps_headers = -I/usr/include/umps3 -I/usr/include -I/usr/share/include -I/usr/share/include/umps3 -I/usr/local/include/umps3/ -I.
compile_flags = -std=c17 -Wall -O0
CFLAGS = $(umps_flags) $(compile_flags) $(umps_headers)

LD = mipsel-linux-gnu-ld
LDFLAGS = -G 0 -nostdlib -T/usr/share/umps3/umpscore.ldscript

.SUFFIXES:
.SUFFIXES: .h .c .o .S .umps

pandos_headers = pandos_const.h pandos_types.h
objects = pcb.o asl.o p1test.o

kernel.core.umps kernel.stab.umps : kernel
	umps3-elf2umps -k kernel

kernel : crtso.o libumps.o $(objects)
	$(CC) $(LDFLAGS) -o $@ $^

p1test.o : p1test.c
	$(CC) $(CFLAGS)  -c $^ -o $@

asl.o : asl.c asl.h
	$(CC) $(CFLAGS)  -c $< -o $@
pcb.o : pcb.c pcb.h
	$(CC) $(CFLAGS)  -c $< -o $@
libumps.o : libumps.S
	$(CC) $(CFLAGS)  -c $< -o $@
crtso.o : crtso.S
	$(CC) $(CFLAGS)  -c $< -o $@

.PHONY = clean
clean:
	rm $(objects) kernel kernel.core.umps kernel.stab.umps
