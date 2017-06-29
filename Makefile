
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

all: example1 example2 example3 example4

example1: $(OBJ) $(ODIR)/example1.o
	$(CC) -o $@.out $^ $(CFLAGS)

example2: $(OBJ) $(ODIR)/example2.o
	$(CC) -o $@.out $^ $(CFLAGS)

example3: $(OBJ) $(ODIR)/example3.o
	$(CC) -o $@.out $^ $(CFLAGS)

example4: $(OBJ) $(ODIR)/example4.o
	$(CC) -o $@.out $^ $(CFLAGS)

.PHONY: clean

clean:
	rm -f *.out $(ODIR)/*.o *~ core $(IDIR)/*~

