CC=gcc

STD= -std=gnu99

RFLAGS= -Wall -O

DFLAGS= -Wall -g 

TESTS= tests/utils.c tests/test_smtp.c tests/tests.c socket.c smtp.c message.c
# latex -> pdf
PDFLATEX = pdflatex -interaction=nonstopmode

all: clean server test_client unit_tests 

server: 
	$(CC) $(STD) $(RFLAGS) socket.c smtp.c server.c process.c message.c logger.c -o server -lrt

test_client: 
	$(CC) $(STD) $(DFLAGS) test_client.c -o test_client


unit_tests:
	$(CC) $(STD) $(RFLAGS) $(TESTS) -o unit_tests -lrt -lcunit
	

clean: 
	rm -rf *.o server test_client unit_tests

fsm/server-fsm.c: fsm/server.def
	cd fsm && autogen server.def
fsm/server_dot.dot: fsm/server.def
	fsm/fsm2dot $< > $@

fsm/server_dot.tex: fsm/server_dot.dot
	dot2tex -ftikz --autosize --crop  $< > $@

fsm/server_dot.pdf: fsm/server_dot.tex
	$(PDFLATEX) -output-directory ../report/tex/static $<