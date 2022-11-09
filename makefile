TARGET=farm client

CC=gcc
CFLAGS=-pthread -g -Wall -O 
LDLIBS=-lm -lrt -pthread

SRCDIR=./src
OUTDIR=./bin
OBJDIR=./build
INCDIR=./include

objects=apilab.o farm.o client.o

.PHONY: clean

apilab.o: $(SRCDIR)/apilab.c $(INCDIR)/apilab.h
		$(CC) $(CFLAGS) -c -o $(OBJDIR)/apilab.o $(SRCDIR)/apilab.c

farm.o: $(SRCDIR)/farm.c $(INCDIR)/apilab.h
		$(CC) $(CFLAGS) -c -o $(OBJDIR)/farm.o $(SRCDIR)/farm.c

client.o: $(SRCDIR)/client.c $(INCDIR)/apilab.h
		$(CC) $(CFLAGS) -c -o $(OBJDIR)/client.o $(SRCDIR)/client.c

farm: $(OBJDIR)/farm.o $(OBJDIR)/apilab.o
		$(CC) $(OBJDIR)/apilab.o $(OBJDIR)/farm.o  $(LDLIBS) -o $(OUTDIR)/farm

client:	$(OBJDIR)/client.o $(OBJDIR)/apilab.o
		$(CC) $(OBJDIR)/apilab.o $(OBJDIR)/client.o $(LDLIBS) -o $(OUTDIR)/client

all: farm client

clean:
	rm -rf $(OBJDIR)/* $(OUTDIR)/*