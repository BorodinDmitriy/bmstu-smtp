CC=gcc

RFLAGS= -Wall -O

DFLAGS= -Wall -g 

all: client

client: 
	$(CC) $(RFLAGS) main.c -o client

debug_client: 
	$(CC) $(DFLAGS) main.c -o client

clean: 
	rm -rf *.o client
