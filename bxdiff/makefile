

COMMON = $(HOME)/proj/src/lib
INCLUDE = -I. -I$(COMMON)/include
COMODIR = $(COMMON)/$(MACHTYPE)

CFLAGS = -O $(INCLUDE) -L/usr/X11R6/lib

all: nproto.h bxdiff 

nproto.h: bxdiff.c
	cproto $(INCLUDE) -e bxdiff.c > nproto.h
	cmp -s nproto.h proto.h || cp nproto.h proto.h

bxdiff: bxdiff.c
	gcc $(CFLAGS) $(INCLUDE) -o bxdiff bxdiff.c $(COMODIR)/Xroutines.o -lX11
