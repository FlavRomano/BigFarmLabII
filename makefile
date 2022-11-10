SRCDIR=./src
BINDIR=./bin
OBJDIR=./build
HEADERDIR=./include

CC=gcc
CFLAGS=-I$(HEADERDIR) -pthread -g -Wall
LDLIBS=-lm -lrt -pthread # Library flags or linker stuffs

TARGET=farm client

apilab.o: $(SRCDIR)/apilab.c $(HEADERDIR)/apilab.h
	$(CC) $(CFLAGS) -c -o $(OBJDIR)/apilab.o $(SRCDIR)/apilab.c

farm.o: $(SRCDIR)/farm.c $(HEADERDIR)/apilab.h
	$(CC) $(CFLAGS) -c -o $(OBJDIR)/farm.o $(SRCDIR)/farm.c

client.o: $(SRCDIR)/client.c $(HEADERDIR)/apilab.h
	$(CC) $(CFLAGS) -c -o $(OBJDIR)/client.o $(SRCDIR)/client.c

farm: farm.o apilab.o
	$(CC) $(OBJDIR)/apilab.o $(OBJDIR)/farm.o  $(LDLIBS) -o $(BINDIR)/farm

client:	client.o apilab.o
	$(CC) $(OBJDIR)/apilab.o $(OBJDIR)/client.o $(LDLIBS) -o $(BINDIR)/client

.PHONY: clean all
all: $(TARGET)

clean:
	rm -rf $(OBJDIR)/* $(BINDIR)/farm $(BINDIR)/client 