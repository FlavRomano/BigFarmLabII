TARGET=farm client

CC=gcc
CFLAGS=-pthread -g -Wall -O 
LDLIBS=-lm -lrt -pthread

SRCDIR=./src
OUTDIR=./bin
OBJDIR=./build
INCDIR=./include

objects=apilab.o farm.o client.o

apilab.o: $(SRCDIR)/apilab.c $(INCDIR)/apilab.h
		$(CC) $(CFLAGS) -c -o $(OBJDIR)/apilab.o $(SRCDIR)/apilab.c

farm.o: $(SRCDIR)/farm.c $(INCDIR)/apilab.h
		$(CC) $(CFLAGS) -c -o $(OBJDIR)/farm.o $(SRCDIR)/farm.c

client.o: $(SRCDIR)/client.c $(INCDIR)/apilab.h
		$(CC) $(CFLAGS) -c -o $(OBJDIR)/client.o $(SRCDIR)/client.c

farm: farm.o apilab.o
		$(CC) $(OBJDIR)/apilab.o $(OBJDIR)/farm.o  $(LDLIBS) -o $(OUTDIR)/farm

client:	client.o apilab.o
		$(CC) $(OBJDIR)/apilab.o $(OBJDIR)/client.o $(LDLIBS) -o $(OUTDIR)/client

all: farm client

clean:
	rm -rf $(OBJDIR)/* $(OUTDIR)/*