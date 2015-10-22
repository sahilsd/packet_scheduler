CC=g++
CFLAGS=-I. 

all: sim_rr

debug: CFLAGS += -DDEBUG=ON -g
debug: sim_rr 

sim_rr: prr.o
	$(CC) prr.o sim.hpp -o sim_rr $(CFLAGS)

clean:
	rm -f sim_rr *dat *o 
