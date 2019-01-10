#ifndef PROCESS_H
#define PROCESS_H

#include "../common/header.h"
#include "socket.h"
#include <sys/ipc.h> 
#include <sys/msg.h> 
#include <stdlib.h>
#include <mqueue.h>

// structure for message queue 
struct mesg_buffer { 
    long mesg_type; 
    int fd;
};

struct process {
	pid_t pid;
	struct sockaddr_in serv_address;
	size_t addrlen;
	mqd_t *mq;
	char *queue_name;
	int state_worked;
	char *logger_name;
	int extra;

	fd_set listener_set; // reader_set
	fd_set socket_set; // reader_set
	fd_set writer_set; // writer_set
	fd_set exception_set; // exception_set

	int max_fd;
	struct client_socket_list *listeners_list;
	struct client_socket_list *sock_list;
};

struct process * init_process(pid_t pid, struct fd_linked_list *socket_fds, struct sockaddr_in serv_address, int logger_pid);
int * init_processes(int count, struct fd_linked_list *socket_fds, struct sockaddr_in serv_address, int logger_pid);
void init_signal_catch(sigset_t *empty, sigset_t *block);
void handle_process_signal(int signum);

#endif