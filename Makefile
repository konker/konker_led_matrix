CC = gcc
CFLAGS = -Wall -g -fno-builtin -lwiringPi -lm -I../hexfont
CFLAGS_EXTRA = -lhexfont -lkulm -L. -L../hexfont
utilObjsMatrix = kulm_matrix.o
utilObjsSegment = kulm_segment.o

example: example.c libkulm.a
	$(CC) $(CFLAGS) -o $@ $< $(CFLAGS_EXTRA)

libkulm.a: $(utilObjsMatrix) $(utilObjsSegment)
	ar rc $@ $(utilObjsMatrix) $(utilObjsSegment)
	ranlib $@

$(utilObjsMatrix): kulm_matrix.c kulm_matrix.h
	$(CC) $(CFLAGS) -c $<

$(utilObjsSegment): kulm_segment.c kulm_segment.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm -rf *.o libkulm.a example

all: example
.PHONY: all clean
