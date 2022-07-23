CC=gcc
CFLAGS=-pthread -g -Wall -O 
TARGET=farm

all:
	$(CC) farm.c xerrori.c $(CFLAGS) -o $(TARGET)

run:
	./$(TARGET) -t 5 z0.dat z1.dat

clean:
	rm -f $(TARGET) $(EXECS) *.o

valgrind:
	valgrind -s --leak-check=full ./$(TARGET) z0.dat z1.dat