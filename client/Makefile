CC=gcc

RFLAGS= -Wall -O

DFLAGS= -Wall -g 

all: client

client: 
	$(CC) $(RFLAGS) main.c controller.c smtp_socket.c file_viewer.c -o client

debug_client: 
	$(CC) $(DFLAGS) main.c controller.c smtp_socket.c file_viewer.c -o client
	

clean: 
	rm -rf *.o client
