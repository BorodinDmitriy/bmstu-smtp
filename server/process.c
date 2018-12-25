#include "process.h"

struct process * init_process(pid_t pid, struct fd_linked_list *socket_fds) {
	struct process *result = (struct process *) malloc(sizeof(struct process));
	result->pid = pid;
	
    result->sock_list = NULL;
    result->max_fd = -1;
    FD_ZERO(&(result->socket_set));
    printf("serv_sock = %d\n", socket_fds);

    return result;
}

struct process * init_processes(int count, struct fd_linked_list *socket_fds) {
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
        		struct process *pr = init_process(getpid(),socket_fds);
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
                return NULL;
    	}
	}
}