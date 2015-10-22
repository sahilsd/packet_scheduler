CC=g++
CFLAGS=-I. 

all: sim_drr

debug: CFLAGS += -DDEBUG=1 -g
debug: sim_drr

sim_drr: drr.o
	$(CC) drr.o sim.hpp -o sim_drr $(CFLAGS)

clean:
	rm -f sim_drr *dat *o 
