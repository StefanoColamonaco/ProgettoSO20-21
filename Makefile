SHELL = /bin/sh
VPATH = /usr/local/share/umps3 : /usr/share/umps3 : ./src : ./obj
CC = mipsel-linux-gnu-gcc

src_dir = src
obj_dir = obj

umps_flags = -ffreestanding -mips1 -mabi=32 -mno-gpopt -G 0 -mno-abicalls -fno-pic -mfp32
umps_headers = -I/usr/include/umps3 -I/usr/include -I/usr/share/include -I/usr/share/include/umps3 -I/usr/local/include/umps3/ -I/usr/local/include/ -I.
compile_flags = -std=c11 -Wall -O0
CFLAGS = $(umps_flags) $(compile_flags) $(umps_headers)

LD = mipsel-linux-gnu-ld
LDFLAGS = -G 0 -nostdlib -T/usr/share/umps3/umpscore.ldscript

.SUFFIXES:
.SUFFIXES: .h .c .o .S .umps

pandos_headers = pandos_const.h pandos_types.h
objects = $(addprefix obj/, pcb.o asl.o p1test.o crtso.o libumps.o)

.PHONY = clean all

all : kernel.core.umps kernel.stab.umps

kernel.core.umps kernel.stab.umps &: kernel
	umps3-elf2umps -k kernel

kernel : $(objects)
	$(LD) $(LDFLAGS) -o $@ $^

$(obj_dir)/%.o : $(src_dir)/%.c | $(obj_dir)
	$(CC) $(CFLAGS) -c -o $@ $<

$(obj_dir)/crtso.o $(obj_dir)/libumps.o : $(obj_dir)/%.o : %.S | $(obj_dir)
	$(CC) $(CFLAGS) -o $@ -c $<

$(obj_dir) :
	mkdir "$(obj_dir)"

clean:
	rm $(objects) kernel kernel.core.umps kernel.stab.umps
