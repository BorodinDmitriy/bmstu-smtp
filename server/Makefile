CC=gcc

STD= -std=gnu99

RFLAGS= -Wall -O

DFLAGS= -Wall -g 

all: clean server test_client

server: 
	$(CC) $(STD) $(RFLAGS) socket.c smtp.c server.c process.c message.c -o server

test_client: 
	$(CC) $(STD) $(DFLAGS) test_client.c -o test_client
	

clean: 
	rm -rf *.o server test_client
