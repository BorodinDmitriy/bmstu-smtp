#include "process.h"

struct process * init_process(pid_t pid) {
	struct process *result = (struct process *) malloc(sizeof(struct process));
	result->pid = pid;
	sprintf(result->key_type, "%d", (int)pid);

	key_t k;
	// ftok to generate unique key 
    k = ftok(result->key_type, (int)pid);
    // msgget creates a message queue 
    // and returns identifier 
    int msgid = msgget(k, 0666 | IPC_CREAT); 

    result->key = k;
    result->msgid = msgid;
    result->sock_list = NULL;
    result->max_fd = -1;
    FD_ZERO(&(result->socket_set));

    return result;
}

struct process * init_processes(int count) {
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
        		struct process *pr = init_process(getpid());
        		run_process(pr);
        		//smtp_handler(sock_fd, getpid());
        	default: // процесс - родитель
        		return init_process(getpid());
    	}
	}
}