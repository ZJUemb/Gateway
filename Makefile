CC=gcc
LIB1=config
LIB2=pthread
LIB3=m
IDIR=include
SDIR=src
ODIR=tmp
DFLAGS=-ggdb
CFLAGS=-I$(IDIR)
LDFLAGS=-l$(LIB1) -l$(LIB2) -l$(LIB3)
CLEANFILES=gateway tmp/*

_DEPS=EmbGW.h md5.h
_OBJ=md5.o wrapper.o myerr.o controller.o config.o\
	 init.o kernel.o signal.o thpool.o http.o binary.o\
	 cJSON.o
DEPS=$(patsubst %, $(IDIR)/%, $(_DEPS))
OBJ=$(patsubst %, $(ODIR)/%, $(_OBJ))

$(ODIR)/%.o: $(SDIR)/%.c $(DEPS)
	$(CC) $(DFLAGS) -c -o $@ $< $(CFLAGS)

gateway: $(OBJ)
	$(CC) $(DFLAGS) -o $@ $(OBJ) $(CFLAGS) $(LDFLAGS)

clean:
	rm -f $(CLEANFILES)
