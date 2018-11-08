#include <stdio.h>
#include <unistd.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h> 

#define SERVER_PORT 8080
#define SERVER_BUFFER_SIZE 1024
#define SERVER_BACKLOG_SIZE 3

#define TEST_CLIENT_BUFFER_SIZE 1024

#define EXIT_FAILURE -1
#define TEST_CLIENT_EXIT_FAILURE -2