#include "logger.h"

int init_logger( struct sockaddr_in serv_address) {
	pid_t pid;
    switch (pid = fork()) {
    	case -1:
    		printf("fork() failed"); 
        	return NULL;
        case 0:  // процесс - потомок
        	printf("logger process forked with pid: %d\n", getpid());
        	printf("parent pid: %d\n", getppid());
        	struct process *pr = init_process(getpid(),NULL, serv_address, getpid());

            sigset_t empty, block;
            init_signal_catch(&empty, &block);

        	int a = run_logger(pr);
            kill(getpid(),SIGTERM);
        		
        		//smtp_handler(sock_fd, getpid());
        	default: // процесс - родитель
        		break;
    }
    return pid;
}
int run_logger(struct process *pr) {
	int rc;
	while (pr->state_worked) {
		struct timeval tv; // timeval используется внутри select для выхода из ожидания по таймауту
		tv.tv_sec = 15;
		tv.tv_usec = 0;

		FD_ZERO(&(pr->socket_set));

		if (*(pr->mq) != NULL) {
			FD_SET(*(pr->mq), &(pr->socket_set));
		}

		// now we can use select with timeout
		rc = select(pr->max_fd + 1, &(pr->socket_set), NULL, NULL, &tv);
		if (rc == 0) {
			printf("logger not ready - timeout\n");
		} else {
			// some sockets are ready
			// check mqueue
			if (*(pr->mq) != NULL) {
				if (FD_ISSET(*(pr->mq), &(pr->socket_set))) {
					char msg_buffer[SERVER_BUFFER_SIZE];
					memset(msg_buffer, 0x00, sizeof(msg_buffer));
        			int bytes_read = mq_receive(*(pr->mq), msg_buffer, SERVER_BUFFER_SIZE, NULL);
        			if (bytes_read >= 0) {
						printf("Logger: Received message: %s\n", msg_buffer);
						save_to_logger_file(msg_buffer);
						if (strcmp(msg_buffer,"#") == 0) {
							pr->state_worked = 0;
							continue;
						} 
					} else {
						printf("Logger: None \n");
					}
				}
			}
		}
	}
	return 0;
}

int save_to_logger_file(char *new_text) {


    FILE* logger_file = fopen("server_log", "a");
    if (!logger_file) {
        perror("Error opening logger file");
        perror("server_log");
        return -1;
    }

    time_t current_time = time(NULL);
    char* time = ctime(&current_time);
    time[strlen(time) - 1] = '\0';
    fprintf(logger_file, "[%s]: \"%s\".\n", time,new_text);
    fflush(logger_file);

    fclose(logger_file);
    return 0;
}
int dispose_logger();