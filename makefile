
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
  LIBO = 
endif


CFLAGS = -g $(INCLUDE) -L/usr/X11R6/lib
OFILES = bxdiff.o search.o xover.o map.o pane.o main.o
CFILES = $(OFILES:.o=.c)

all: nproto.h bxdiff 

nproto.h: $(CFILES)
	if [ ! -f proto.h ]; then touch proto.h; fi
	cproto $(INCLUDE) -e $^ > nproto.h
	cmp -s nproto.h proto.h || cp nproto.h proto.h

%.o: %.c Xlocal.h proto.h

bxdiff: $(OFILES) Xlocal.h $(LIBO)
	gcc $(CFLAGS) $(INCLUDE) -o bxdiff $(OFILES) $(COMODIR)/Xroutines.o -lX11 -lpcre2-8

$(COMODIR)/Xroutines.o: $(LIBS)/Xroutines.c
	mkdir -p $(COMODIR)
	gcc -c $(CFLAGS) $(INCLUDE) -o $@ $<

clean:
	rm -f bxdiff *.o $(LIBO) nproto.h proto.h
