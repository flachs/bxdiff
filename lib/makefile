# make all	creates all executables for the programs in this directory

SRC  = src
INC  = include
DEST = $(MACHTYPE)
CFLAGS = -g -I $(INC)

all: $(DEST)/Xroutines.o $(DEST)/mathpac.o $(DEST)/zopen.o $(DEST)/pstring.o $(DEST)/rbtree.o

clean:
	rm -f *.o *.lst

$(DEST)/zopen.o: $(SRC)/zopen.c $(INC)/common.h
	$(CC) $(CFLAGS) -o $@ -c $<

$(DEST)/mathpac.o: $(SRC)/mathpac.c $(INC)/mathpac.h $(INC)/common.h
	$(CC) $(CFLAGS) -o $@ -c $<

$(DEST)/Xroutines.o: $(SRC)/Xroutines.c $(INC)/Xstuff.h $(INC)/common.h
	$(CC) $(CFLAGS) -o $@ -c $<

$(DEST)/pstring.o: $(SRC)/pstring.c $(INC)/space_shift.h  $(INC)/pstring.h 
	$(CC) $(CFLAGS) -o $@ -c $<

$(DEST)/rbtree.o: $(SRC)/rbtree.c $(INC)/space_shift.h  $(INC)/rbtree.h 
	$(CC) $(CFLAGS) -o $@ -c $<
