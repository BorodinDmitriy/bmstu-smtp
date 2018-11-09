#include "../common/header.h"
#include "smtp.h"
#include "socket.h"

int main(int argc, char **argv) {

	int new_socket;								// файловый дескриптор сокета, соединяющегося с сервером
	

	struct sockaddr_in address;					// структура для обработки интернет-адресов
	size_t addrlen = sizeof(address);

	struct fd_linked_list *sockets;				// список всех возможных локальных сокетов
	int max_fd_value = -1; 						// максимальное значение файлового дескриптора для select()
	struct fd_linked_list *p;	

	sockets = init_sockets();
	printf("%p\n",sockets);

	// узнаем значение файлового дескриптора для select()
	for (p = sockets; p != NULL; p = p->next) {
		(p->fd > max_fd_value) ? (max_fd_value = p->fd) : 1;
	}

	// вечно слушающий цикл в поисках новых соединений
	// и создающий по процессу на каждое соединение
	while (1) {
		fd_set socket_set;
		FD_ZERO(&socket_set);

		for (p = sockets; p != NULL; p = p->next) {
			FD_SET(p->fd, &socket_set);
		}

		// вечное ожидание соединения по одному из связанных сокетов
		select(max_fd_value + 1, &socket_set, NULL, NULL, NULL);

		// проходим по списку сокетов в поисках установленного соединения
		for (p = sockets; p != NULL; p = p->next) {
			if (FD_ISSET(p->fd, &socket_set)) {
				// принимаем соединение
    			new_socket = accept(p->fd, (struct sockaddr *) &address,  (socklen_t*) &addrlen);
    			if (new_socket < 0) 
    			{ 
        			printf("accept() failed"); 
        			continue;
    			} 

    			// соединение принято - можно делать обработку smtp
    			int *sock_fd = (int *) malloc(sizeof(int));
    			*sock_fd = new_socket;

    			// создание дочернего процесса, где будет происходить обработка
    			pid_t pid;
    			switch (pid = fork()) {
    				case -1:
    					perror("accept() failed"); 
        				continue;
        			case 0:  // процесс - потомок
        				printf("child process forked with pid: %d\n", getpid());
        				printf("parent pid: %d\n", getppid());
        				smtp_handler(sock_fd);
        			default: // процесс - родитель
        				continue;
    			}
			}
		}
	}

	return 0;
}