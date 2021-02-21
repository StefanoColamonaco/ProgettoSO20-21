CC = gcc

CFLAGS = -I/usr/share/include -I/usr/share/include/umps3 -I. -g -Wall -O0
objects = pcb.o asl.o

.PHONY: clean run debug


exec: ../main.o $(objects)
	$(CC) $(CFLAGS) -o $@ $^

p1test : ../p1test.o
	$(CC) $(CFLAGS) -o $@ $^


pcb.o : ../src/pcb.c src/pcb.h
main.o : main.c
asl.o: ../src/asl.c src/asl.h
p1test.o : ../src/p1test.c

clean:
	rm exec $(objects) main.o

debug:
	gdb ./exec
run:
	./exec
