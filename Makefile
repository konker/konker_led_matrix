CC=gcc
CFLAGS=${ENV_CFLAGS} -Wall -g -std=c99 -D_POSIX_C_SOURCE=200112L -fno-builtin -lwiringPi -lm -I../hexfont -I../tinyutf8
CFLAGS_EXTRA=-lklm -lhexfont -ltinyutf8 -L. -L../hexfont -L../tinyutf8
utilObjsMatrix=klm_matrix.o
utilObjsSegment=klm_segment.o
utilObjsSegmentList=klm_segment_list.o

example: example.c libklm.a
	$(CC) $(CFLAGS) -o $@ $< $(CFLAGS_EXTRA)

example_simple: example_simple.c libklm.a
	$(CC) $(CFLAGS) -o $@ $< $(CFLAGS_EXTRA)

test_example: test_example.c libklm.a
	$(CC) $(CFLAGS) -o $@ $< $(CFLAGS_EXTRA) -pg

libklm.a: $(utilObjsMatrix) $(utilObjsSegment) $(utilObjsSegmentList)
	ar rc $@ $(utilObjsMatrix) $(utilObjsSegment) $(utilObjsSegmentList)
	ranlib $@

$(utilObjsMatrix): klm_matrix.c klm_matrix.h
	$(CC) $(CFLAGS) -c $<

$(utilObjsSegment): klm_segment.c klm_segment.h
	$(CC) $(CFLAGS) -c $<

$(utilObjsSegmentList): klm_segment_list.c klm_segment_list.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm -rf *.o libklm.a example example_simple test_example

all: example example_simple test_example
.PHONY: all clean
