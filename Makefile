CC=gcc
CFLAGS=${EXTRA_CFLAGS} -Wall -g -fno-builtin -lwiringPi -lm -I../hexfont
CFLAGS_EXTRA=-lkulm -lhexfont -L. -L../hexfont
utilObjsMatrix=kulm_matrix.o
utilObjsSegment=kulm_segment.o

example: example.c libkulm.a
	$(CC) $(CFLAGS) -o $@ $< $(CFLAGS_EXTRA)

example_simple: example_simple.c libkulm.a
	$(CC) $(CFLAGS) -o $@ $< $(CFLAGS_EXTRA)

libkulm.a: $(utilObjsMatrix) $(utilObjsSegment)
	ar rc $@ $(utilObjsMatrix) $(utilObjsSegment)
	ranlib $@

$(utilObjsMatrix): kulm_matrix.c kulm_matrix.h
	$(CC) $(CFLAGS) -c $<

$(utilObjsSegment): kulm_segment.c kulm_segment.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm -rf *.o libkulm.a example example_simple

all: example example_simple
.PHONY: all clean
