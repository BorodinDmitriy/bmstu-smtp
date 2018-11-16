#ifndef PROCESS_H
#define PROCESS_H

#include "../common/header.h"
#include "socket.h"
#include <sys/ipc.h> 
#include <sys/msg.h> 
#include <stdlib.h>

// structure for message queue 
struct mesg_buffer { 
    long mesg_type; 
    int fd;
};

struct process {
	pid_t pid;
	char key_type[6];
	key_t key; 
    int msgid; 

	fd_set socket_set;
	int max_fd;
	struct client_socket_list *sock_list;
};

struct process * init_process(pid_t pid);
struct process * init_processes(int count);

#endif