#include "server.h"

static struct server serv;

void handle_server_signal(int signum)
{
    if (signum == SIGINT)
    {
        printf("%d: Want to gracefully close\n", getpid());
        int i = 0;
        for (i = 0; i < serv.process_count; i++) {
        	wait(serv.pids[i]);
        }
        wait(serv.logger_pid);
        serv.state = SERVER_FINISH_WORK;
    }
    return; 
}

void init_signal_catcher(sigset_t *empty, sigset_t *block)
{
    struct sigaction signals;
    signals.sa_handler = handle_server_signal;
    signals.sa_flags = 0;
    sigfillset(&signals.sa_mask);
    sigaction(SIGINT, &signals, NULL);

    return;
}

int server_init() {
	struct fd_linked_list *p;	

	serv.state = SERVER_START_INIT;
	serv.socket_fd_max = -1;
	serv.addrlen = sizeof(serv.address);
	serv.socket_fds = init_sockets();
	//serv.socket_fds = init_sockets_using_clients(10);
	serv.process_count = 1;
	serv.logger_pid = init_logger(serv.address);
	serv.pids = init_processes(serv.process_count, serv.socket_fds, serv.address, serv.logger_pid);


	if ((serv.socket_fds == NULL) || (serv.pids == NULL)) {
		printf("SERVER INIT FAILED\n");
		serv.state = SERVER_FAIL_INIT;
	} else {
		printf("%p\n",serv.socket_fds);
		// узнаем значение файлового дескриптора для select()
		for (p = serv.socket_fds; p != NULL; p = p->next) {
			(p->fd > serv.socket_fd_max) ? (serv.socket_fd_max = p->fd) : 1;
		}

		sigset_t empty, block;
        init_signal_catcher(&empty, &block);
		serv.state = SERVER_FINISH_INIT;
	}
	
	return serv.state;
}

int server_run() {
	int new_socket;								// файловый дескриптор сокета, соединяющегося с сервером
	struct fd_linked_list *p;	
	char queue_name[20];
	sprintf(queue_name, "/process%d", serv.logger_pid);

	int mq = mq_open(queue_name, O_WRONLY);

	serv.state = SERVER_START_WORK;
	// вечно слушающий цикл в поисках новых соединений
	// и создающий по процессу на каждое соединение
	while (serv.state == SERVER_START_WORK) {
		char buffer[SERVER_BUFFER_SIZE] = "SERVER_WORKS\n";
        
        if (mq_send(mq, buffer, SERVER_BUFFER_SIZE, 0)) {
            printf("errno = %d\n", errno);
        }
		sleep(100);
	}
	serv.state = SERVER_FINISH_WORK;
	return serv.state;
}

int main(int argc, char **argv) {
	if (server_init() == SERVER_FINISH_INIT) {
		server_run();
	}
	
	return 0;
}