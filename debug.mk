CC = gcc

CFLAGS = -I/usr/share/include -g -Wall -O0
objects = main.o pcb.o

.PHONY: clean run

exec: $(objects)
	$(CC) $(CFLAGS) -o $@ $^

pcb.o : pcb.c
main.o : main.c


clean:
	rm exec $(objects)

debug:
	gdb ./exec
run:
	./exec
