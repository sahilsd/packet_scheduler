CC=g++
CFLAGS=-I. 

all: sim_fifo 

debug: CFLAGS += -DDEBUG -g
debug: sim_fifo 

sim_fifo: fifo.o
	$(CC) fifo.o sim.hpp -o sim_fifo $(CFLAGS)

clean:
	rm -f sim_fifo *dat *o 
