CC=gcc

COMMON_FLAGS= -D_XOPEN_SOURCE

POSTFIX_FLAGS= -lrt -lpthread -lresolv

RFLAGS= -Wall -O
DFLAGS= -Wall -g 

all: debug_client

client: 
	$(CC) $(COMMON_FLAGS) $(RFLAGS) main.c controller.c smtp_interface.c file_viewer.c worker.c logger.c dictionary.c -o client $(POSTFIX_FLAGS)

debug_client: 
	$(CC) $(COMMON_FLAGS) $(DFLAGS) main.c controller.c smtp_interface.c file_viewer.c worker.c logger.c dictionary.c -o client $(POSTFIX_FLAGS)

clean: 
	rm -rf *.o client
