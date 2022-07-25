CC=gcc
CFLAGS=-pthread -g -Wall -O 
LDLIBS=-lm -lrt -pthread
TARGET=farm

all:
	$(CC) farm.c xerrori.c $(CFLAGS) -O0 -o $(TARGET)

farm: farm.o xerrori.o
		$(CC) xerrori.o farm.o  $(LDLIBS) -o farm

farm.o: farm.c xerrori.h
		$(CC) $(CFLAGS) -c -o farm.o farm.c

xerrori.o: xerrori.c xerrori.h
		$(CC) $(CFLAGS) -c -o xerrori.o xerrori.c

run:
	./$(TARGET) -t 5 z0.dat z1.dat

clean:
	rm -f $(TARGET) $(EXECS) *.o

valgrind:
	valgrind -s --leak-check=full ./$(TARGET) z0.dat z1.dat