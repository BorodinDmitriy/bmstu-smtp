#ifndef SERVER_H
#define SERVER_H

#include "../common/header.h"
#include "smtp.h"
#include "socket.h"

#define SERVER_START_INIT 1
#define SERVER_FINISH_INIT 2
#define SERVER_START_WORK 3
#define SERVER_FINISH_WORK 4
#define SERVER_FAIL_INIT -1
#define SERVER_FAIL_WORK -2

int server_init();
int server_run();

struct server {
	struct fd_linked_list *socket_fds;
	int socket_fd_max;
	struct sockaddr_in address;	
	size_t addrlen;
	char *domain;
	int state;
};

#endif