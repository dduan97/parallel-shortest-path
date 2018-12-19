CC = mpicc
LD = mpicc
CFLAGS = -g -O -xHost -fno-alias -std=c99 -I$(TIMINGDIR) -c -lmpi

BINARIES = serial serial.debug tests parallel_dijkstra parallel_dijkstra.debug async_bf sync_bf
COMMON_O = helpers.o min_queue.o benchmarks.o flat_matrix.o resultr.o

all: $(BINARIES)

serial: serial.o $(COMMON_O)
	$(CC) -o $@ $(CFLAGS) $^

serial.debug:
	echo 'DEBUG=1 ./serial $$@' > serial.debug
	chmod +x serial.debug

parallel_dijkstra: parallel_dijkstra.o $(COMMON_O)
	$(CC) -o $@ $(CFLAGS) $^

parallel_dijkstra.debug:
	echo 'DEBUG=1 ./parallel_dijkstra $$@' > $@
	chmod +x $@

async_bf: async_bf.o $(COMMON_O)
	$(CC) -o $@ $(CFLAGS) $^

sync_bf: sync_bf.o $(COMMON_O)
	$(CC) -o $@ $(CFLAGS) $^

tests: tests.o min_queue.o
	$(CC) -o $@ $(CFLAGS) $^

clean:
	rm $(BINARIES) *.o core.* vgcore.*
