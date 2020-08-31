#elan trackpoint updater Makefile
CC ?= gcc

CFLAGS += -g -Wall -fexceptions

main: epstps2_updater.o 
	${CC} ${CFLAGS} ${LDFLAGS} epstps2_updater.o -o epstps2_updater

epstps2_updater.o: main.c
	${CC} ${CFLAGS} ${CPPFLAGS} main.c -c

clean:
	rm -rf epstps2_updater.o
