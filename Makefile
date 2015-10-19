CC = gcc
CFLAGS = -Wall -g -std=c99 -fno-builtin -Iinclude -lwiringPi -lm
utilObjs = ulm.o

libulm.a: $(utilObjs)
	ar rc $@ $(utilObjs)
	ranlib $@

ulm.o: ulm.c include/ulm.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm -rf *.o libulm.a

all: libulm.a
.PHONY: all clean
