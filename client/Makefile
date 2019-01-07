CC=gcc

COMMON_FLAGS= -D_XOPEN_SOURCE

PARALLELS_FLAGS= -lrt -lpthread

RFLAGS= -Wall -O
DFLAGS= -Wall -g 

all: debug_client

client: 
	$(CC) $(COMMON_FLAGS) $(RFLAGS) main.c controller.c smtp_socket.c file_viewer.c worker.c logger.c dictionary.c -o client $(PARALLELS_FLAGS)

debug_client: 
	$(CC) $(COMMON_FLAGS) $(DFLAGS) main.c controller.c smtp_socket.c file_viewer.c worker.c logger.c dictionary.c -o client $(PARALLELS_FLAGS)

clean: 
	rm -rf *.o client
