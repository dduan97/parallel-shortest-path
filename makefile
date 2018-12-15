CC = mpicc
CFLAGS = -g -O -xHost -fno-alias -std=c99 -I$(TIMINGDIR) -c

BINARIES = serial tests

all: $(BINARIES)

serial: helpers.o serial.o min_queue.o
	$(CC) -o $@ $(CFLAGS) $^

tests: tests.o min_queue.o
	$(CC) -o $@ $(CFLAGS) $^

clean:
	rm $(BINARIES) *.o
