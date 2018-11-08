CC=gcc

STD= -std=c99

RFLAGS= -Wall -O

DFLAGS= -Wall -g 

all: server test_client

server: 
	$(CC) $(STD) $(RFLAGS) server.c test_client.c -o server

debug_client: 
	$(CC) $(STD) $(DFLAGS) server.c test_client.c -o test_client
	

clean: 
	rm -rf *.o server test_client
