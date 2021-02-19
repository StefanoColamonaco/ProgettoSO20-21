CC = gcc

CFLAGS = -I/usr/share/include -g -Wall -O0
objects = pcb.o

.PHONY: clean run debug

exec: main.o $(objects)
	$(CC) $(CFLAGS) -o $@ $^

p1test : p1test.o
	$(CC) $(CFLAGS) -o $@ $^


pcb.o : pcb.c
main.o : main.c
p1test.o : p1test.c

clean:
	rm exec $(objects)

debug:
	gdb ./exec
run:
	./exec
