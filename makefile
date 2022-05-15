CC=gcc
CFLAGS=-pthread -g -Wall -O -std=c99
TARGET=farm

all:
	$(CC) main.c $(CFLAGS) -o $(TARGET)

run:
	./$(TARGET) -n 4 -q 8 -t 0 arg1 arg2 arg3

clean:
	rm $(TARGET)