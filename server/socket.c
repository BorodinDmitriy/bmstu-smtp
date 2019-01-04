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
	hints.ai_family = AF_INET;		// пока AF_INET, потом надо сделать AF_UNSPEC для IPv6
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
			continue;
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