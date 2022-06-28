
ifneq ($(wildcard lib/include),)
  COMMON = lib
  INCLUDE = -I. -I$(COMMON)/include
  COMODIR = $(COMMON)/$(MACHTYPE)
  LIBO = $(COMODIR)/Xroutines.o
  LIBS = $(COMMON)/src
else
  COMMON = $(HOME)/proj/src/lib
  INCLUDE = -I. -I$(COMMON)/include
  COMODIR = $(COMMON)/$(MACHTYPE)
  LIBO = 0
endif


CFLAGS = -g $(INCLUDE) -L/usr/X11R6/lib

all: nproto.h bxdiff 

nproto.h: bxdiff.c search.c
	if [ ! -f proto.h ]; then touch proto.h; fi
	cproto $(INCLUDE) -e $^ > nproto.h
	cmp -s nproto.h proto.h || cp nproto.h proto.h

search.o: search.c Xlocal.h proto.h

bxdiff.o: bxdiff.c Xlocal.h proto.h

bxdiff: bxdiff.o Xlocal.h search.o $(LIBO)
	gcc $(CFLAGS) $(INCLUDE) -o bxdiff bxdiff.o search.o $(COMODIR)/Xroutines.o -lX11 -lpcre2-8

$(COMODIR)/Xroutines.o: $(LIBS)/Xroutines.c
	mkdir -p $(COMODIR)
	gcc -c $(CFLAGS) $(INCLUDE) -o $@ $<

clean:
	rm -f bxdiff *.o $(LIBO) nproto.h proto.h
