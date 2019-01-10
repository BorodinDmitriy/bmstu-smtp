#include "process.h"

struct process * init_process(pid_t pid, struct fd_linked_list *socket_fds, struct sockaddr_in serv_address, int logger_pid) {
    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = SERVER_BUFFER_SIZE;
    attr.mq_curmsgs = 0;

    struct process *result = (struct process *) malloc(sizeof(struct process));
	result->pid = pid;
	
    result->sock_list = NULL;
    result->listeners_list = NULL;
    result->max_fd = -1;
    result->serv_address = serv_address;
    result->addrlen = sizeof(serv_address);
    result->state_worked = 1;
    result->extra = 0;

    char queue_name[20];
    char logger_name[20];
    sprintf(queue_name, "/process%d", getpid());
    sprintf(logger_name, "/process%d", logger_pid);
    result->logger_name = logger_name;
    //printf("QUEUE2 = %s\n", queue_name);
    result->queue_name = queue_name;
    result->mq = NULL;
    mqd_t mq = mq_open(result->queue_name, O_CREAT | O_RDONLY | O_NONBLOCK, 0644, &attr);
    if (mq > 0) {
        result->mq = (mqd_t *) malloc(sizeof(mqd_t));
        *(result->mq) = mq;
        if (result->max_fd < mq) {
            result->max_fd = mq;
        }
        printf("mqueue created");
    }

    FD_ZERO(&(result->listener_set));
    FD_ZERO(&(result->socket_set));
    FD_ZERO(&(result->writer_set));
    FD_ZERO(&(result->exception_set));

    printf("serv_sock = %d\n", socket_fds);
    struct fd_linked_list *p;
    for (p = socket_fds; p != NULL; p = p->next) {
        printf("%d\n", p->fd);

        struct client_socket cl_sock = init_client_socket(p->fd, 0, SOCKET_STATE_WAIT, SERVER_MAX_RECIPIENTS, 0);

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
void handle_process_signal(int signum)
{
    if (signum == SIGINT)
    {
        printf("%d: Want to gracefully close\n", getpid());
        mqd_t mq;
        char buffer[SERVER_BUFFER_SIZE] = "#";
        char queue_name[20];

        sprintf(queue_name, "/process%d", getpid());

        /* open the mail queue */
        mq = mq_open(queue_name, O_WRONLY);
        if (mq_send(mq, buffer, SERVER_BUFFER_SIZE, 0) < 0) {
            printf("errno = %d\n", errno);
        }
        
        mq_close(mq);

    }
    return; 
}

void init_signal_catch(sigset_t *empty, sigset_t *block)
{
    /*sigemptyset(empty);
    sigemptyset(block);

    sigaddset(block, SIGINT);
    sigprocmask(SIG_BLOCK, block, empty);*/
    struct sigaction signals;
    signals.sa_handler = handle_process_signal;
    signals.sa_flags = 0;
    //sigemptyset(&signals.sa_mask);
    sigfillset(&signals.sa_mask);
    sigaction(SIGINT, &signals, NULL);

    return;
}

int * init_processes(int count, struct fd_linked_list *socket_fds, struct sockaddr_in serv_address, int logger_pid) {
	int i;
	int *result_pid_array = (int *)malloc(count * sizeof(int));
	for (i = 0; i < count; i++) {
		pid_t pid;
    	switch (pid = fork()) {
    		case -1: {
                printf("fork() failed"); 
                return NULL;
            }
        	case 0:  {
                // процесс - потомок
                struct process *pr = init_process(getpid(),socket_fds, serv_address, logger_pid);
                int logger_mq = mq_open(pr->logger_name, O_WRONLY);
                if (logger_mq > 0) {
                    pr->extra = logger_mq;
                }

                char logger_buffer[SERVER_BUFFER_SIZE];
                sprintf(logger_buffer, "child process forked with pid: %d\n", getpid());
                if (mq_send(pr->extra, logger_buffer, SERVER_BUFFER_SIZE, 0) < 0) {
                    perror(logger_buffer);
                }
                sprintf(logger_buffer, "parent pid: %d\n", getppid());
                if (mq_send(pr->extra, logger_buffer, SERVER_BUFFER_SIZE, 0) < 0) {
                    perror(logger_buffer);
                }

                static int state_worked = 1;

                sigset_t empty, block;
                init_signal_catch(&empty, &block);

                int a = run_process(pr);
                kill(getpid(),SIGTERM);
            }
        	default: {
                // процесс - родитель
                //return init_process(getpid(),socket_fds);
                result_pid_array[i] = pid;
                continue;
            }
    	}
	}
    return result_pid_array;
}