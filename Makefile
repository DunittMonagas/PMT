
IDIR= include
CC= gcc
CFLAGS= -I$(IDIR)

SDIR= src
_ODIR= obj
ODIR= $(SDIR)/$(_ODIR)

_DEPS= queue.h pmt.h
DEPS= $(patsubst %, $(IDIR)/%, $(_DEPS))

_OBJ= queue.o pmt.o
OBJ= $(patsubst %, $(ODIR)/%, $(_OBJ))

$(ODIR)/%.o: $(SDIR)/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

example: $(OBJ) $(ODIR)/example.o
	$(CC) -o $@.out $^ $(CFLAGS)

example5: $(OBJ) $(ODIR)/example5.o
	$(CC) -o $@.out $^ $(CFLAGS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(IDIR)/*~

