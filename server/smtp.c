#include "../common/header.h"
#include "smtp.h"
#include "process.h"

int run_process(struct process *pr) {
	struct mesg_buffer message;

	printf("SMTP run process start");

	int received_bytes_count;				// число прочитанных байт
	char buffer_output[SERVER_BUFFER_SIZE];	// выходной буфер для записи ответа

	char smtp_stub[SERVER_BUFFER_SIZE] = "Hi, you've come to smtp server";

	int new_socket;								// файловый дескриптор сокета, соединяющегося с сервером

	while (1) {
		printf("PROCESS_RUNS\n");
		//sleep(50);
		struct timeval tv; // timeval используется внутри select для выхода из ожидания по таймауту
		tv.tv_sec = 15;
		tv.tv_usec = 0;
		printf("listeners_list = %p is_null = %d\n",pr->listeners_list, (pr->listeners_list == NULL));
		if (pr->listeners_list != NULL) {
			// first add to sets all the sockets in the process' socket list
			struct client_socket_list *p;
    		for (p = pr->listeners_list; p != NULL; p = p->next) {
    			FD_SET(p->c_sock.fd, &(pr->listener_set));
    			//FD_SET(p->c_sock.fd, &(pr->writer_set));
    			//FD_SET(p->c_sock.fd, &(pr->exception_set));
    			printf("socket = %d\n", p->c_sock.fd);
    		}

    		// вечное ожидание соединения по одному из связанных сокетов
			select(pr->max_fd + 1, &(pr->listener_set), NULL, NULL, NULL);

			// проходим по списку сокетов в поисках установленного соединения
			for (p = pr->listeners_list; p != NULL; p = p->next) {
				if (FD_ISSET(p->c_sock.fd, &(pr->listener_set))) {

					// принимаем соединение
    				new_socket = accept(p->c_sock.fd, (struct sockaddr *) &(pr->serv_address),  (socklen_t*) &(pr->addrlen));
    				printf("new_socket = %d pid = %d\n", new_socket, getpid());
    				if (new_socket < 0) 
    				{ 
        				printf("accept() failed"); 
        				continue;
    				} 

    				//FD_CLR(p->c_sock.fd, &(pr->socket_set));
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

    				struct client_socket cl_sock;
        			cl_sock.fd = new_socket;
        			cl_sock.buffer = (char *) malloc(SERVER_BUFFER_SIZE);
        			cl_sock.state = 0;

        			// добавить новый сокет в список сокетов процесса
        			struct client_socket_list *new_scket = malloc(sizeof(struct client_socket_list));
        			new_scket->c_sock = cl_sock;
        			new_scket->next = pr->sock_list;
        			pr->sock_list = new_scket;

        			if (pr->max_fd < cl_sock.fd) {
            			pr->max_fd = cl_sock.fd;
        			}

        			// call smtp_handler for new socket
        			//new_smtp_handler(&new_socket, getpid());
        			//FD_SET(p->c_sock.fd, &(pr->socket_set));

				}
			}
		}

		// end accept_connection
		printf("sock_list = %p is_null = %d\n",pr->sock_list, (pr->sock_list == NULL));
		if (pr->sock_list != NULL) {
			struct client_socket_list *p;
    		for (p = pr->sock_list; p != NULL; p = p->next) {
    			FD_SET(p->c_sock.fd, &(pr->socket_set));
    			//FD_SET(p->c_sock.fd, &(pr->writer_set));
    			//FD_SET(p->c_sock.fd, &(pr->exception_set));
    			printf("client_socket = %d\n", p->c_sock.fd);
    		}

			select(pr->max_fd + 1, &(pr->socket_set), NULL, NULL, &tv);

			for (p = pr->sock_list; p != NULL; p = p->next) {
				if (FD_ISSET(p->c_sock.fd, &(pr->socket_set))) {

        			// call smtp_handler for socket
        			int new_sock = (p->c_sock.fd);
        			smtp_handler(&new_sock, getpid());
        			//FD_SET(p->c_sock.fd, &(pr->socket_set));

				}
			}
		}
	}
	/*
	// old_version
	while (1) {
		printf("PROCESS_RUNS\n");
		//sleep(50);
		struct timeval tv; // timeval используется внутри select для выхода из ожидания по таймауту
		tv.tv_sec = 15;
		tv.tv_usec = 0;
		printf("sock_list = %p is_null = %d\n",pr->sock_list, (pr->sock_list == NULL));
		if (pr->sock_list != NULL) {
			// first add to sets all the sockets in the process' socket list
			struct client_socket_list *p;
    		for (p = pr->sock_list; p != NULL; p = p->next) {
    			FD_SET(p->c_sock.fd, &(pr->socket_set));
    			FD_SET(p->c_sock.fd, &(pr->writer_set));
    			FD_SET(p->c_sock.fd, &(pr->exception_set));
    			printf("socket = %d\n", p->c_sock.fd);
    		}

    		// вечное ожидание соединения по одному из связанных сокетов
			select(pr->max_fd + 1, &(pr->socket_set), NULL, NULL, NULL);

			// проходим по списку сокетов в поисках установленного соединения
			for (p = pr->sock_list; p != NULL; p = p->next) {
				if (FD_ISSET(p->c_sock.fd, &(pr->socket_set))) {

					// принимаем соединение
    				new_socket = accept(p->c_sock.fd, (struct sockaddr *) &(pr->serv_address),  (socklen_t*) &(pr->addrlen));
    				printf("new_socket = %d pid = %d\n", new_socket, getpid());
    				if (new_socket < 0) 
    				{ 
        				printf("accept() failed"); 
        				continue;
    				} 

    				//FD_CLR(p->c_sock.fd, &(pr->socket_set));
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

    				struct client_socket cl_sock;
        			cl_sock.fd = new_socket;
        			cl_sock.buffer = (char *) malloc(SERVER_BUFFER_SIZE);
        			cl_sock.state = 0;

        			// добавить новый сокет в список сокетов процесса
        			struct client_socket_list *new_scket = malloc(sizeof(struct client_socket_list));
        			new_scket->c_sock = cl_sock;
        			new_scket->next = pr->sock_list;
        			pr->sock_list = new_scket;

        			if (pr->max_fd < cl_sock.fd) {
            			pr->max_fd = cl_sock.fd;
        			}

        			// call smtp_handler for new socket
        			new_smtp_handler(&new_socket, getpid());
        			//FD_SET(p->c_sock.fd, &(pr->socket_set));

				}
			}
		}
	}
	// end old_version
	*/

	// TODO: select for sets + smtp_handler (base_version)
	// TODO: states in smtp_handler (mid_version)

	/*while (1) {
    
    	printf("asd\n");
    	printf("%d\n",pr->sock_list->c_sock.fd);
		struct timeval tv; // timeval используется внутри select для выхода из ожидания по таймауту
		tv.tv_sec = 15;
		tv.tv_usec = 0;

		int fd = pr->sock_list->c_sock.fd;
		printf("%d\n", fd);
		printf("%d\n", pr->max_fd);


		if (pr->sock_list != NULL) {
			struct client_socket_list *p;
    		for (p = pr->sock_list; p != NULL; p = p->next) {
    			FD_SET(p->c_sock.fd, &(pr->socket_set));
    			//printf("%d\n", p->c_sock.fd);
    		}

    		// ожидаем select-ом возможности писать в сокет сервером в течение 15с
			int sock_count = select(pr->max_fd + 1,  &(pr->socket_set), NULL, NULL, &tv);

			printf("sock_count = %d\n", sock_count);
			if (sock_count > 1) {
				return 1;
			}
			
			p = pr->sock_list;
			while (p != NULL) {
				// если так и не дождались возможности писать - ошибка
				if (!FD_ISSET(p->c_sock.fd,  &(pr->socket_set))) {
					sprintf(buffer_output, "socket is not available for 15s\n");
					printf("socket is not available for 15s\n");
					send(p->c_sock.fd, buffer_output, strlen(buffer_output), 0);
					struct client_socket_list *temp = p;
					p->next = temp->next;
					free(temp);
					//FD_CLR(p->c_sock.fd, &(pr->socket_set));
					//close(p->c_sock.fd);
				} else {

					switch (p->c_sock.state) {
						case SOCKET_STATE_SEND_STUB: {
							sprintf(buffer_output, "%s\n", smtp_stub);
							if (send(p->c_sock.fd, buffer_output, strlen(buffer_output), 0) < 1) {
								printf("send_stub");
								return 0;
							} else {
								printf("send_stub_ok");
								p->c_sock.state = SOCKET_STATE_RECEIVE_DATA;
							}
						}
						case SOCKET_STATE_RECEIVE_DATA: {
							// читаем, что прислал клиент, во входной буфер
							received_bytes_count = recv(p->c_sock.fd, p->c_sock.buffer, SERVER_BUFFER_SIZE, 0);
							// считали 0 байт - клиент перестал отправлять данные
							if (received_bytes_count == 0) {
								printf("remote host closed socket %d\n", p->c_sock.fd);
								struct client_socket_list *temp = p;
								p->next = temp->next;
								free(temp);
								return 0;
							}
							if (received_bytes_count < 0) {
								printf("problems with socket %d\n", p->c_sock.fd);
								return 0;
							}
							if (received_bytes_count > 0) {
								printf("%s\n",p->c_sock.buffer);
							}
						
						}
					}
					
				}
				p = p->next;
			}
		}
	}*/
}

void new_smtp_handler(int *socket_fd, const int pid) {
	printf("SMTP handler start");

	int client_socket_fd = *socket_fd;		// клентский сокет, полученный после select()
	int received_bytes_count;				// число прочитанных байт
	// int i, j;
	char buffer[SERVER_BUFFER_SIZE];		// буфер, в который считываем
	char buffer_output[SERVER_BUFFER_SIZE];	// выходной буфер для записи ответа

	char smtp_stub[SERVER_BUFFER_SIZE] = "Hi, you've come to smtp server";

	sprintf(buffer_output, "%s\n", smtp_stub);
	printf("%s\n", smtp_stub);
	if (send(client_socket_fd, buffer_output, strlen(buffer_output), 0) < 0) {
		return;
	}
}

void smtp_handler(int *socket_fd, const int pid) {
	printf("SMTP handler begin");

	int client_socket_fd = *socket_fd;		// клентский сокет, полученный после select()
	int received_bytes_count;				// число прочитанных байт
	// int i, j;
	char buffer[SERVER_BUFFER_SIZE];		// буфер, в который считываем
	char buffer_output[SERVER_BUFFER_SIZE];	// выходной буфер для записи ответа

	char smtp_stub[SERVER_BUFFER_SIZE] = "Hi, you've come to smtp server";

	sprintf(buffer_output, "%s\n", smtp_stub);
	printf("%s\n", smtp_stub);
	send(client_socket_fd, buffer_output, strlen(buffer_output), 0);

	while (1) {
		fd_set socket_set; // fd_set используется для select - выбор свободного сокета для записи
		struct timeval tv; // timeval используется внутри select для выхода из ожидания по таймауту

		// начальная инициализация socket_set единственным сокетом socket_fd
		FD_ZERO(&socket_set);
		FD_SET(client_socket_fd, &socket_set);
		tv.tv_sec = 15;
		tv.tv_usec = 0;

		// ожидаем select-ом возможности писать в сокет сервером в течение 15с
		int sock_count = select(client_socket_fd + 1, &socket_set, NULL, NULL, &tv);

		printf("%d\n", sock_count);

		// если так и не дождались возможности писать - ошибка
		if (!FD_ISSET(client_socket_fd, &socket_set)) {
			sprintf(buffer_output, "socket is not available for 15s\n");
			printf("socket is not available for 15s\n");
			send(client_socket_fd, buffer_output, strlen(buffer_output), 0);
			break;
		}

		// читаем, что прислал клиент, во входной буфер
		received_bytes_count = recv(client_socket_fd, buffer, SERVER_BUFFER_SIZE, 0);
		// считали 0 байт - клиент перестал отправлять данные
		if (received_bytes_count == 0) {
			printf("remote host closed socket %d\n", client_socket_fd);
			break;
		}
		if (received_bytes_count == -1) {
			printf("problems with socket %d\n", client_socket_fd);
			break;
		}

		char *eol; // признак конца линии обрабатываемой команды
		while (strstr(buffer, "\r\n")) {
			eol = strstr(buffer, "\r\n");
			eol[0] = '\0'; 

			// обработка ключевых слов
			printf("Client: %d, message: %s\n", client_socket_fd, buffer);

			// конец строки для сравнения (потом переделать под разделение на данные и команды)
			buffer[4] = '\0';

			if (STR_EQUAL(buffer, "HELO")) { 
				// начальное приветствие
				sprintf(buffer_output, HEADER_250_OK);
			} else if (STR_EQUAL(buffer, "EHLO")) { 
				// улучшенное начальное приветствие
				sprintf(buffer_output, HEADER_250_OK);
			} else if (STR_EQUAL(buffer, "MAIL")) { 
				// получено новое письмо от
				sprintf(buffer_output, HEADER_250_OK);
			} else if (STR_EQUAL(buffer, "RCPT")) { 
				// письмо направлено ... 
				sprintf(buffer_output, HEADER_250_OK_RECIPIENT);
			} else if (STR_EQUAL(buffer, "DATA")) { 
				// содержимое письма
				sprintf(buffer_output, HEADER_354_CONTINUE);
			} else if (STR_EQUAL(buffer, "RSET")) { 
				// сброс соединения
				sprintf(buffer_output, HEADER_250_OK_RESET);
			} else if (STR_EQUAL(buffer, "NOOP")) { 
				// ничего не делать
				sprintf(buffer_output, HEADER_250_OK_NOOP);
			} else if (STR_EQUAL(buffer, "QUIT")) { 
				// закрыть соединение
				sprintf(buffer_output, HEADER_221_OK);
				printf("Server: %d, message: %s", client_socket_fd, buffer_output);
				send(client_socket_fd, buffer_output, strlen(buffer_output), 0);
				close(client_socket_fd);
				//kill(pid, SIGTERM);
			} else { 
				// метод не был определен
				sprintf(buffer_output, HEADER_502_NOT_IMPLEMENTED);
			}
			printf("Server: %d, message: %s", client_socket_fd, buffer_output);
			send(client_socket_fd, buffer_output, strlen(buffer_output), 0);


			// смещаем буфер для обработки следующей команды
			memmove(buffer, eol + 2, SERVER_BUFFER_SIZE - (eol + 2 - buffer));

		}
	}
	// обработка команды кончилась - закрываем сокет
	close(client_socket_fd);
	//kill(pid, SIGTERM);
}