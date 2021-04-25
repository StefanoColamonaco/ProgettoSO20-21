SHELL = /bin/sh
VPATH = /usr/local/share/umps3 : /usr/share/umps3 : ./src : ./obj
CC = mipsel-linux-gnu-gcc

src_dir = src
obj_dir = obj

umps_flags = -ffreestanding -mips1 -mabi=32 -mno-gpopt -G 0 -mno-abicalls -fno-pic -mfp32
umps_headers = -I/usr/include -I/usr/share/include -I/usr/share -I/usr/local -I/usr/local/include 
include_dirs = -I. $(umps_headers) $(addsuffix /umps3, $(umps_headers)) 
compile_flags = -std=c11 -Wall -O0
CFLAGS = $(umps_flags) $(compile_flags) $(include_dirs)

LD = mipsel-linux-gnu-ld
LDFLAGS = -G 0 -nostdlib -T$(shell find /usr/ -name umpscore.ldscript 2> /dev/null)
.SUFFIXES:
.SUFFIXES: .h .c .o .S .umps

phase1_objs = pcb.o asl.o
phase2_objs = exceptions.o init.o interrupts.o scheduler.o stateUtil.o systemCalls.o
phase3_objs = vmSupport.o
test_obj = p3test.o
pandos_headers = pandos_const.h pandos_types.h
objects = $(addprefix obj/, crtso.o libumps.o $(test_obj) $(phase1_objs) $(phase2_objs) $(phase3_objs))

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
	-rm  $(objects) kernel kernel.core.umps kernel.stab.umps 2> /dev/null

withstd: $(objects)
	$(LD) -v -G 0 -L/usr/include -T$(shell find /usr/ -name umpscore.ldscript 2> /dev/null) -o kernel $^

