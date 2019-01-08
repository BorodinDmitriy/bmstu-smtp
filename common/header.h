#ifndef HEADER_H
#define HEADER_H

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <signal.h>
#include <fcntl.h>

#define CLIENT_MAIL_DIR_PATH "./maildir/"
#define STR_EQUAL(a,b)	(strcmp(a, b) == 0)
// данные сервера
#define SERVER_PORT 15565
#define SERVER_PORT_STRING "8080"
#define SERVER_BUFFER_SIZE 1024
#define SERVER_BACKLOG_SIZE 3
#define SERVER_MAX_RECIPIENTS 10

#define SERVER_EXIT_FAILURE -1

// заголовки сервера
#define HEADER_221_OK "221 Ok\r\n"
#define HEADER_250_OK "250 Ok\r\n"
#define HEADER_250_OK_NOOP "250 Ok noop\r\n"
#define HEADER_250_OK_RECIPIENT "250 Ok recipient\r\n"
#define HEADER_250_OK_RESET "250 Ok reset\r\n"
#define HEADER_252_OK "252 Ok\r\n"
#define HEADER_354_START "354 Start mail input; end with <CRLF>.<CRLF>\r\n"
#define HEADER_354_CONTINUE "354 Continue\r\n"
#define HEADER_450_MAILBOX_UNAVAILABLE "450 Requested mail action not taken: mailbox unavailable\r\n"
#define HEADER_451_EXCEEDED_RECIPIENTS "451 Unable to complete command: number of recipients exceeded\r\n"
#define HEADER_502_NOT_IMPLEMENTED "502 Command Not Implemented\r\n"


// данные тестового клиента
#define TEST_CLIENT_BUFFER_SIZE 1024
#define TEST_CLIENT_EXIT_FAILURE -2

struct Mail
{
	char *from;
	char *to;
	char *message;
};

#endif
