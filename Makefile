#elan trackpoint updater Makefile
CC ?= gcc

CFLAGS += -g -Wall -fexceptions

main: main.o 
	${CC} ${CFLAGS} ${LDFLAGS} main.o -o epstps2_updater

main.o: main.c
	${CC} ${CFLAGS} ${CPPFLAGS} main.c -c

clean:
	rm -rf main.o
