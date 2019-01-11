CC=gcc

COMMON_FLAGS= -D_XOPEN_SOURCE

POSTFIX_FLAGS= -lrt -lpthread -lresolv

STD= -std=gnu99

RFLAGS= -Wall -O
DFLAGS= -Wall -g 

TESTS= tests/utils.c tests/test_smtp.c tests/tests.c dictionary.c smtp_interface.c logger.c file_viewer.c controller.c worker.c

all: debug_client

client: 
	$(CC) $(COMMON_FLAGS) $(RFLAGS) main.c controller.c smtp_interface.c file_viewer.c worker.c logger.c dictionary.c -o client $(POSTFIX_FLAGS)

debug_client: 
	$(CC) $(COMMON_FLAGS) $(DFLAGS) main.c controller.c smtp_interface.c file_viewer.c worker.c logger.c dictionary.c -o client $(POSTFIX_FLAGS)

unit_tests:
	$(CC) $(STD) $(RFLAGS) $(TESTS) -o unit_tests -lrt -lcunit  $(POSTFIX_FLAGS)

clean_test:
	rm -rf unit_tests

clean: 
	rm -rf *.o client unit_tests
