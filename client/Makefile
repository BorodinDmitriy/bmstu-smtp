CC=gcc

RFLAGS= -Wall -O

DFLAGS= -Wall -g 

all: client

client: 
	$(CC) -D_XOPEN_SOURCE $(RFLAGS) main.c controller.c smtp_socket.c file_viewer.c worker.c logger.c -o client -lrt -lpthread 

debug_client: 
	$(CC) -D_XOPEN_SOURCE $(DFLAGS) main.c controller.c smtp_socket.c file_viewer.c worker.c logger.c -o client -lrt -lpthread
	

clean: 
	rm -rf *.o client
