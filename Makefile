CC=gcc
CFLAGS=${ENV_CFLAGS} -Wall -g -std=c99 -fno-builtin -lwiringPi -lm -I../hexfont -I../tinyutf8
CFLAGS_EXTRA=-lkulm -lhexfont -ltinyutf8 -L. -L../hexfont -L../tinyutf8
utilObjsMatrix=kulm_matrix.o
utilObjsSegment=kulm_segment.o
utilObjsSegmentList=kulm_segment_list.o

example: example.c libkulm.a
	$(CC) $(CFLAGS) -o $@ $< $(CFLAGS_EXTRA)

example_simple: example_simple.c libkulm.a
	$(CC) $(CFLAGS) -o $@ $< $(CFLAGS_EXTRA)

test_example: test_example.c libkulm.a
	$(CC) $(CFLAGS) -o $@ $< $(CFLAGS_EXTRA) -pg

libkulm.a: $(utilObjsMatrix) $(utilObjsSegment) $(utilObjsSegmentList)
	ar rc $@ $(utilObjsMatrix) $(utilObjsSegment) $(utilObjsSegmentList)
	ranlib $@

$(utilObjsMatrix): kulm_matrix.c kulm_matrix.h
	$(CC) $(CFLAGS) -c $<

$(utilObjsSegment): kulm_segment.c kulm_segment.h
	$(CC) $(CFLAGS) -c $<

$(utilObjsSegmentList): kulm_segment_list.c kulm_segment_list.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm -rf *.o libkulm.a example example_simple test_example

all: example example_simple test_example
.PHONY: all clean
