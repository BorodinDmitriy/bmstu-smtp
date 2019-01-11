#include "socket.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

// производит связывание с портом максимальное 
// число локальных сокетов
struct fd_linked_list * init_sockets(void) {
	int error_code;				// код ошибки для getaddrinfo()
	int opt = 1;
	int socket_fd;
	struct addrinfo hints;		// структура, определяющая параметры локальных сокетов
	struct addrinfo *hostinfo;	// список - результирующий набор сокетов
	struct addrinfo *p;			// переменная цикла
	struct fd_linked_list *result = NULL;

	// инициализируем hints
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;		// пока AF_INET, потом надо сделать AF_UNSPEC для IPv6
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;	// если не указать, полученные сокеты будут доступны 
									// только для методов connect, sendmsg и какие-то еще клиентские

	error_code = getaddrinfo(NULL, SERVER_PORT_STRING, &hints, &hostinfo);
	if (error_code != 0) {
		printf("failed to get host addrinfo");
		return NULL;
	}

	// проходимся по списку сокетов, биндим, заносим в список
	for (p = hostinfo; p != NULL; p = p->ai_next) {
		void *addr;
		char ipstr[INET6_ADDRSTRLEN];

		// TODO: добавить IPv6
		if (p->ai_family == AF_INET) {
			addr = &((struct sockaddr_in*)p->ai_addr)->sin_addr; 
		} else {
			addr = &((struct sockaddr_in6*)p->ai_addr)->sin6_addr; 
		}

		// получение ip-адреса в человекочитаемом виде - для связывания
		inet_ntop(p->ai_family, addr, ipstr, sizeof(ipstr));

		// создаем файловый дескриптор сокета
		socket_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if (socket_fd == -1) {
			printf("failed to create ipv4 socket");
			continue;
		}

		// принудительное связывание сокета с портом (8080)
		// установка опций
		// SOL_SOCKET - 
		// SO_REUSEADDR - 
		// SO_REUSEPORT - 
		if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEPORT | SO_REUSEADDR, &opt, sizeof(opt))) {
			printf("setsockopt() failed"); 
        	continue;
		}

		int file_flags = fcntl(socket_fd, F_GETFL, 0);
    	if (file_flags == -1)
    	{
        	printf("Fail to receive socket flags");
        	continue;
    	}

    	if (fcntl(socket_fd, F_SETFL, file_flags | O_NONBLOCK))
    	{	
        	printf("Fail to set flag 'O_NONBLOCK' for socket");
        	continue;
    	}

		// непосредственное связывание сокета с портом
		if (bind(socket_fd, p->ai_addr, p->ai_addrlen) < 0) 
    	{ 
        	printf("bind() failed"); 
        	continue;
    	} 
		
		// говорим, что "слушаем" соединения на связанном порту 
    	if (listen(socket_fd, SERVER_BACKLOG_SIZE) < 0) 
    	{ 
        	printf("listen() failed"); 
        	return NULL;
    	} 

    	printf("listener = %d\n", socket_fd);
    	// добавить новый сокет в список сокетов
    	struct fd_linked_list *new_socket = malloc(sizeof(struct fd_linked_list));
    	new_socket->fd = socket_fd;
    	new_socket->next = result;
    	result = new_socket;
	}
	if (result == NULL) {
		printf("couldnt connect to any of the local sockets"); 
		return NULL;
	}
	freeaddrinfo(hostinfo);

	return result;
}

// производит связывание с портом максимальное 
// число локальных сокетов
struct fd_linked_list * init_sockets_using_clients(int client_number) {
	int error_code;				// код ошибки для getaddrinfo()
	int opt = 1;
	int socket_fd;
	struct addrinfo hints;		// структура, определяющая параметры локальных сокетов
	struct addrinfo *hostinfo;	// список - результирующий набор сокетов
	struct addrinfo *p;			// переменная цикла
	struct fd_linked_list *result = NULL;

	int i;
	for (i = 0; i < client_number; i++) {

		socket_fd = 0;
		if( (socket_fd = socket(AF_INET , SOCK_STREAM , 0)) == 0) {
        	perror("socket failed");
        	exit(0);
    	}


		// принудительное связывание сокета с портом (8080)
		// установка опций
		// SOL_SOCKET - 
		// SO_REUSEADDR - 
		// SO_REUSEPORT - 
		if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEPORT | SO_REUSEADDR, &opt, sizeof(opt))) {
			printf("setsockopt() failed"); 
        	continue;
		}

		int file_flags = fcntl(socket_fd, F_GETFL, 0);
    	if (file_flags == -1)
    	{
        	printf("Fail to receive socket flags");
        	continue;
    	}

    	if (fcntl(socket_fd, F_SETFL, file_flags | O_NONBLOCK))
    	{	
        	printf("Fail to set flag 'O_NONBLOCK' for socket");
        	continue;
    	}

    	struct sockaddr_in sock_address;

    	sock_address.sin_family = AF_INET;
    	sock_address.sin_addr.s_addr = INADDR_ANY;
    	sock_address.sin_port = htons( 8080 );
      
    	if (bind(socket_fd, (struct sockaddr *)&(sock_address), sizeof(sock_address)) < 0) {
        	perror("bind failed");
        	exit(0);
    	}
     
    	if (listen(socket_fd, 3) < 0) {
        	perror("listen");
        	exit(0);
    	}

    	struct fd_linked_list *new_socket = malloc(sizeof(struct fd_linked_list));
    	new_socket->fd = socket_fd;
    	new_socket->next = result;
    	result = new_socket;
	}

	if (result == NULL) {
		printf("couldnt connect to any of the local sockets"); 
		return NULL;
	}

	return result;
}

struct client_socket init_client_socket(int fd, int buffer_size, int state, int max_recepients, int needs_message) {
	struct client_socket result;

	result.fd = fd;
	result.buffer = NULL;
	if (buffer_size > 0) {
		result.buffer = malloc(buffer_size * sizeof(int));
	}
	result.state = state;
	result.buffer_offset = 0;
	result.message = NULL;
	if (needs_message) {
		result.message = malloc(sizeof(struct msg));
		result.message->to = malloc(max_recepients * sizeof(char*));
		result.message->from = NULL;
		result.message->body = NULL;
		result.message->recepients_num = 0;
	}
    return result;
}

int count_list_elems(struct client_socket_list *root) {
	int result = 0;
	while (root != NULL) {
		result++;
		root = root->next;
	}
	return result;
}


struct client_socket_list* delete_elem(struct client_socket_list* curr, int state_value)
{
  	/* See if we are at end of list. */
  	if (curr == NULL)
    	return NULL;

  	/*
  	 * Check to see if current node is one
   	 * to be deleted.
   	*/
  	if (curr->c_sock.state == state_value) {
  		//close(curr->c_sock.fd);
    	struct client_socket_list* temp;

    	/* Save the next pointer in the node. */
    	temp= curr->next;

    	/* Deallocate the node. */
    	free_client_socket(curr->c_sock);
    	//free(curr);

    	/*
     	* Return the NEW pointer to where we
     	* were called from.  I.e., the pointer
     	* the previous call will use to "skip
     	* over" the removed node.
     	*/
    	return temp;
  	}

  	/*
   	* Check the rest of the list, fixing the next
   	* pointer in case the next node is the one
   	* removed.
   	*/
  	curr->next = delete_elem(curr->next, state_value);


  	/*
   	* Return the pointer to where we were called
   	* from.  Since we did not remove this node it
   	* will be the same.
   	*/
  	return curr;
}

void free_client_socket(struct client_socket *c_sock) {
	close(c_sock->fd);
	free(c_sock->buffer);
	if (c_sock->message != NULL) {
		int i = 0;
		for (i = 0; i < c_sock->message->recepients_num; i++) {
			free(c_sock->message->to[i]);
		}
		free(c_sock->message->from);
		free(c_sock->message->body);
		free(c_sock->message);
	}
	free(c_sock);
}