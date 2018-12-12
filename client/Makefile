CC=gcc

RFLAGS= -Wall -O

DFLAGS= -Wall -g 

all: client

client: 
	$(CC) $(RFLAGS) main.c controller.c smtp_socket.c file_viewer.c worker.c -o client -lpthread

debug_client: 
	$(CC) $(DFLAGS) main.c controller.c smtp_socket.c file_viewer.c worker.c -o client -lpthread
	

clean: 
	rm -rf *.o client
