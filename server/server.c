#include "server.h"

static struct server serv;

int server_init() {
	struct fd_linked_list *p;	

	serv.state = SERVER_START_INIT;
	serv.socket_fd_max = -1;
	serv.addrlen = sizeof(serv.address);
	serv.socket_fds = init_sockets();
	serv.prcs = init_processes(2, serv.socket_fds);
	if (serv.socket_fds == NULL) {
		printf("SERVER INIT FAILED\n");
		serv.state = SERVER_FAIL_INIT;
	} else {
		printf("%p\n",serv.socket_fds);
		// узнаем значение файлового дескриптора для select()
		for (p = serv.socket_fds; p != NULL; p = p->next) {
			(p->fd > serv.socket_fd_max) ? (serv.socket_fd_max = p->fd) : 1;
		}
		serv.state = SERVER_FINISH_INIT;
	}
	
	return serv.state;
}

int server_run() {
	int new_socket;								// файловый дескриптор сокета, соединяющегося с сервером
	struct fd_linked_list *p;	

	serv.state = SERVER_START_WORK;
	// вечно слушающий цикл в поисках новых соединений
	// и создающий по процессу на каждое соединение
	while (1) {
		printf("SERVER_WORKS\n");
		printf("serv_sock = %d\n", serv.socket_fds);
		sleep(100);
		/*fd_set socket_set;
		FD_ZERO(&socket_set);

		for (p = serv.socket_fds; p != NULL; p = p->next) {
			FD_SET(p->fd, &socket_set);
		}

		// вечное ожидание соединения по одному из связанных сокетов
		select(serv.socket_fd_max + 1, &socket_set, NULL, NULL, NULL);

		// проходим по списку сокетов в поисках установленного соединения
		for (p = serv.socket_fds; p != NULL; p = p->next) {
			if (FD_ISSET(p->fd, &socket_set)) {

				// принимаем соединение
    			new_socket = accept(p->fd, (struct sockaddr *) &(serv.address),  (socklen_t*) &(serv.addrlen));
    			if (new_socket < 0) 
    			{ 
        			printf("accept() failed"); 
        			continue;
    			} 

    			FD_CLR(p->fd, &socket_set);
    			int file_flags = fcntl(new_socket, F_GETFL, 0);
    			if (file_flags == -1)
    			{
        			printf("Fail to receive socket flags");
        			continue;
    			}

    			if (fcntl(new_socket, F_SETFL, file_flags | O_NONBLOCK))
    			{	
        			printf("Fail to set flag 'O_NONBLOCK' for socket");
        			continue;
    			}

    			// соединение принято - можно делать обработку smtp
    			int *sock_fd = (int *) malloc(sizeof(int));
    			*sock_fd = new_socket;

    			struct mesg_buffer message;
    			message.fd = new_socket;
    			message.mesg_type = 1;


    			// создание дочернего процесса, где будет происходить обработка
    			/*pid_t pid;
    			switch (pid = fork()) {
    				case -1:
    					perror("accept() failed"); 
        				continue;
        			case 0:  // процесс - потомок
        				printf("child process forked with pid: %d\n", getpid());
        				printf("parent pid: %d\n", getppid());
        				smtp_handler(sock_fd, getpid());
        			default: // процесс - родитель
        				continue;
    			}*/
			//}
		//}
	}
	serv.state = SERVER_FINISH_WORK;
	return serv.state;
}

int main(int argc, char **argv) {
	if (server_init() == SERVER_FINISH_INIT) {
		server_run();
	}
	//init_process(getpid());
	
	return 0;
}