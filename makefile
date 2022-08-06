CC=gcc
CFLAGS=-pthread -g -Wall -O 
LDLIBS=-lm -lrt -pthread
TARGET=farm client

all: farm client

client:	client.o apilab.o
		$(CC) apilab.o client.o $(LDLIBS) -o client

client.o: client.c apilab.h
		$(CC) $(CFLAGS) -c -o client.o client.c

farm: farm.o apilab.o
		$(CC) apilab.o farm.o  $(LDLIBS) -o farm

farm.o: farm.c apilab.h
		$(CC) $(CFLAGS) -c -o farm.o farm.c

apilab.o: apilab.c apilab.h
		$(CC) $(CFLAGS) -c -o apilab.o apilab.c

clean:
	rm -f $(TARGET) $(EXECS) *.o