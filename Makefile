CC = g++
CFLAGS = -std=c++14 -Wall

all: test.cc
	$(CC) $(CFLAGS) -o test test.cc

clean:
	rm -f *.o test
