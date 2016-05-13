CC=gcc
LIB=config
IDIR=include
SDIR=src
ODIR=tmp
DFLAGS=-ggdb
CFLAGS=-I$(IDIR)
LDFLAGS=-l$(LIB)
CLEANFILES=gateway tmp/*

_DEPS=EmbGW.h md5.h
_OBJ=md5.o wrapper.o myerr.o controller.o config.o init.o kernel.o signal.o
DEPS=$(patsubst %, $(IDIR)/%, $(_DEPS))
OBJ=$(patsubst %, $(ODIR)/%, $(_OBJ))

$(ODIR)/%.o: $(SDIR)/%.c $(DEPS)
	$(CC) $(DFLAGS) -c -o $@ $< $(CFLAGS)

server: $(OBJ)
	$(CC) $(DFLAGS) -o $@ $(OBJ) $(CFLAGS) $(LDFLAGS)

clean:
	rm -f $(CLEANFILES)
