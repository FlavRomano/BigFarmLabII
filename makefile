CC=gcc
CFLAGS=-pthread -g -Wall -O 
TARGET=farm

all:
	$(CC) main.c $(CFLAGS) -o $(TARGET)

run:
	./$(TARGET) -q 2 -t 1 z0.dat z1.dat z2.dat z3.dat z4.dat z5.dat z6.dat

clean:
	rm $(TARGET)

valgrind:
	valgrind -s --leak-check=full ./$(TARGET) z0.dat z1.dat