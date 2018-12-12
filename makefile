CC = mpicc
CFLAGS = -g -O3 -xHost -fno-alias -std=c99 -I$(TIMINGDIR) -c

BINARIES = serial

all: $(BINARIES)

serial: helpers.o serial.o
	$(CC) -o $@ $(CFLAGS) $^


