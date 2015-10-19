CC = gcc
CFLAGS = -Wall -g -std=c99 -fno-builtin -lwiringPi -lm
utilObjs = UltrathinLEDMatrix.o

libUltrathinLEDMatrix.a: $(utilObjs)
	ar rc $@ $(utilObjs)
	ranlib $@

UltrathinLEDMatrix.o: UltrathinLEDMatrix.c UltrathinLEDMatrix.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm -rf *.o libUltrathinLEDMatrix.a

all: libUltrathinLEDMatrix.a
.PHONY: all clean
