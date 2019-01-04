#ifndef SOCKET_H
#define SOCKET_H

#include "../common/header.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#define SOCKET_STATE_DEFAULT 7
#define SOCKET_STATE_SEND_STUB 8
#define SOCKET_STATE_RECEIVE_DATA 9

// список файловых дескрипторов сокетов
struct fd_linked_list {
	int fd;
	struct fd_linked_list *next;
};

struct client_socket {
	int fd;
	char *buffer;
	int state;
};

struct client_socket_list {
	struct client_socket c_sock;
	struct client_socket_list *next;
};

struct fd_linked_list * init_sockets(void);
struct fd_linked_list * init_sockets_using_clients(int client_number);
#endif