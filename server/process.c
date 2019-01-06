#include "process.h"

struct process * init_process(pid_t pid, struct fd_linked_list *socket_fds, struct sockaddr_in serv_address) {
	struct process *result = (struct process *) malloc(sizeof(struct process));
	result->pid = pid;
	
    result->sock_list = NULL;
    result->listeners_list = NULL;
    result->max_fd = -1;
    result->serv_address = serv_address;
    result->addrlen = sizeof(serv_address);
    FD_ZERO(&(result->listener_set));
    FD_ZERO(&(result->socket_set));
    FD_ZERO(&(result->writer_set));
    FD_ZERO(&(result->exception_set));

    printf("serv_sock = %d\n", socket_fds);
    struct fd_linked_list *p;
    for (p = socket_fds; p != NULL; p = p->next) {
        printf("%d\n", p->fd);

        struct client_socket cl_sock;
        cl_sock.fd = p->fd;
        cl_sock.buffer = NULL;
        cl_sock.state = SOCKET_STATE_WAIT;
        cl_sock.buffer_offset = 0;
        cl_sock.message = NULL;

        // добавить новый сокет в список сокетов процесса
        struct client_socket_list *new_socket = malloc(sizeof(struct client_socket_list));
        new_socket->c_sock = cl_sock;
        new_socket->next = result->listeners_list;
        result->listeners_list = new_socket;

        if (result->max_fd < cl_sock.fd) {
            result->max_fd = cl_sock.fd;
        }
    }

    return result;
}

struct process * init_processes(int count, struct fd_linked_list *socket_fds, struct sockaddr_in serv_address) {
	int i;
	struct process *result = NULL;
	for (i = 0; i < count; i++) {
		pid_t pid;
    	switch (pid = fork()) {
    		case -1:
    			printf("fork() failed"); 
        		return NULL;
        	case 0:  // процесс - потомок
        		printf("child process forked with pid: %d\n", getpid());
        		printf("parent pid: %d\n", getppid());
        		struct process *pr = init_process(getpid(),socket_fds, serv_address);
        		while (1) {
        			int a = run_process(pr);
        			//printf("rr\n");

        			if (a > 1) {
        				break;
        			}
        		}
        		
        		//smtp_handler(sock_fd, getpid());
        	default: // процесс - родитель
        		//return init_process(getpid(),socket_fds);
                continue;
    	}
	}
}