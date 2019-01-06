#ifndef SOCKET_H
#define SOCKET_H

#include "../common/header.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include "message.h"

#define SOCKET_STATE_DEFAULT 7
#define SOCKET_STATE_SEND_STUB 8
#define SOCKET_STATE_RECEIVE_DATA 9

#define SOCKET_STATE_INIT 7
#define SOCKET_STATE_WAIT 8
#define SOCKET_STATE_MAIL_CREATED_NO_RECEPIENTS 9
#define SOCKET_STATE_RECEPIENTS_SET 10
#define SOCKET_STATE_WRITING_DATA 11
#define SOCKET_STATE_DELIVERING 12
#define SOCKET_STATE_CLOSED 13

// список файловых дескрипторов сокетов
struct fd_linked_list {
	int fd;
	struct fd_linked_list *next;
};

struct client_socket {
	int fd;
	char *buffer;
	int buffer_offset;
	int input_message;
	int state;
	struct msg *message;
};

struct client_socket_list {
	struct client_socket c_sock;
	struct client_socket_list *next;
};

struct fd_linked_list * init_sockets(void);
struct fd_linked_list * init_sockets_using_clients(int client_number);
#endif