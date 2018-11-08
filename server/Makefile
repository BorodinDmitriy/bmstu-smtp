CC=gcc

STD= -std=c99

RFLAGS= -Wall -O

DFLAGS= -Wall -g 

all: clean server test_client

server: 
	$(CC) $(STD) $(RFLAGS) server.c -o server

test_client: 
	$(CC) $(STD) $(DFLAGS) test_client.c -o test_client
	

clean: 
	rm -rf *.o server test_client
