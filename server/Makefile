CC=gcc

STD= -std=gnu99

RFLAGS= -Wall -O

DFLAGS= -Wall -g 

TESTS= tests/utils.c tests/test_smtp.c tests/tests.c socket.c smtp.c message.c

all: clean server test_client unit_tests

server: 
	$(CC) $(STD) $(RFLAGS) socket.c smtp.c server.c process.c message.c logger.c -o server -lrt

test_client: 
	$(CC) $(STD) $(DFLAGS) test_client.c -o test_client


unit_tests:
	$(CC) $(STD) $(RFLAGS) $(TESTS) -o unit_tests -lrt -lcunit
	

clean: 
	rm -rf *.o server test_client unit_tests
