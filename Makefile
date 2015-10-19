CC = gcc
CFLAGS = -Wall -g -std=c99 -fno-builtin -Iinclude -lwiringPi -lm
utilObjs = kulm.o

libkulm.a: $(utilObjs)
	ar rc $@ $(utilObjs)
	ranlib $@

$(utilObjs): kulm.c include/kulm.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm -rf *.o libkulm.a

all: libkulm.a
.PHONY: all clean
