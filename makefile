CC = mpicc
CFLAGS = -g -O -xHost -fno-alias -std=c99 -I$(TIMINGDIR) -c

BINARIES = serial serial.debug tests

all: $(BINARIES)

serial: helpers.o serial.o min_queue.o
	$(CC) -o $@ $(CFLAGS) $^

serial.debug:
	echo 'DEBUG=1 ./serial $$@' > serial.debug
	chmod +x serial.debug

tests: tests.o min_queue.o
	$(CC) -o $@ $(CFLAGS) $^

clean:
	rm $(BINARIES) *.o
