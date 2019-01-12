#include "../common/header.h"
#include "smtp.h"
#include "process.h"

// -1 - error
// 0 - timeout
// else - amount of clients
int check_clients_by_timeout(struct client_socket_list *clients, int max_fd, struct timeval timeout) {
	fd_set socket_set;
	struct client_socket_list *p;
	int rc;

	FD_ZERO(&socket_set);
	if (clients != NULL) {
		for (p = clients; p != NULL; p = p->next) {
    		FD_SET(p->c_sock.fd, &socket_set);
    		// printf("client_socket%d = %d SOCKET_STATE = %d\n", getpid(), p->c_sock.fd, p->c_sock.state);
    	}

    	return select(max_fd + 1, &socket_set, NULL, NULL, &timeout);
	}
	return -1;
}

int run_process(struct process *pr) {
	char logger_buffer[SERVER_BUFFER_SIZE] = "";

	printf("SMTP run process start\n");
	sprintf(logger_buffer, "SMTP run process start\n");
    if (mq_send(pr->extra, logger_buffer, SERVER_BUFFER_SIZE, 0) < 0) {
        perror(logger_buffer);
    }

	int received_bytes_count;				// число прочитанных байт
	char buffer_output[SERVER_BUFFER_SIZE] = "";	// выходной буфер для записи ответа

	char smtp_stub[SERVER_BUFFER_SIZE] = "Hi, you've come to smtp server";
	int new_socket;								// файловый дескриптор сокета, соединяющегося с сервером

	while (pr->state_worked) {
		struct timeval tv; // timeval используется внутри select для выхода из ожидания по таймауту
		tv.tv_sec = 45;
		tv.tv_usec = 0;
		fd_set temp;

		FD_ZERO(&(pr->socket_set));
		FD_ZERO(&temp);
		// delete all closed sockets
		if (pr->sock_list != NULL) {
			int before = 0;
			int after = 0;
			do {
				before = count_list_elems(pr->sock_list);
				pr->sock_list = delete_elem(pr->sock_list, SOCKET_STATE_CLOSED);
				after = count_list_elems(pr->sock_list);
			} while ((before - after) > 0);
		}

		// add all the sockets to socket list
		struct client_socket_list *p;
		int rc;
		// printf("listeners_list = %p is_null = %d\n",pr->listeners_list, (pr->listeners_list == NULL));
		// printf("sock_list = %p is_null = %d\n",pr->sock_list, (pr->sock_list == NULL));
		sprintf(logger_buffer, "listeners_list = %p is_null = %d\n",pr->listeners_list, (pr->listeners_list == NULL));
    	if (mq_send(pr->extra, logger_buffer, SERVER_BUFFER_SIZE, 0) < 0) {
        	perror(logger_buffer);
    	}
    	sprintf(logger_buffer, "sock_list = %p is_null = %d\n",pr->sock_list, (pr->sock_list == NULL));
    	if (mq_send(pr->extra, logger_buffer, SERVER_BUFFER_SIZE, 0) < 0) {
        	perror(logger_buffer);
    	}

		
		// first add listeners
		if (pr->listeners_list != NULL) {
			for (p = pr->listeners_list; p != NULL; p = p->next) {
				if (p->c_sock.flag == 0)
    				FD_SET(p->c_sock.fd, &(pr->socket_set));
    			else 
    				FD_SET(p->c_sock.fd, &temp);
    			//printf("lisening_socket = %d\n", p->c_sock.fd);
    		}
		}
		// then add client sockets if exist
		if (pr->sock_list != NULL) {
			for (p = pr->sock_list; p != NULL; p = p->next) {
    			if (p->c_sock.flag == 0)
    				FD_SET(p->c_sock.fd, &(pr->socket_set));
    			else 
    				FD_SET(p->c_sock.fd, &temp);
    			printf("client_socket%d = %d SOCKET_STATE = %d\n", getpid(), p->c_sock.fd, p->c_sock.state);
    			sprintf(logger_buffer, "client_socket%d = %d SOCKET_STATE = %d\n", getpid(), p->c_sock.fd, p->c_sock.state);
    			if (mq_send(pr->extra, logger_buffer, SERVER_BUFFER_SIZE, 0) < 0) {
        			perror(logger_buffer);
    			}
    		}
		}

		if (*(pr->mq) != NULL) {
			FD_SET(*(pr->mq), &(pr->socket_set));
		}

		
		// now we can use select with timeout
		rc = select(pr->max_fd + 1, &temp,  &(pr->socket_set), NULL, &tv);
		if (rc == 0) {
			// no sockets are ready - timeout
			printf("no sockets are ready - timeout\n");
			sprintf(logger_buffer, "no sockets are ready - timeout\n");
    		if (mq_send(pr->extra, logger_buffer, SERVER_BUFFER_SIZE, 0) < 0) {
        		perror(logger_buffer);
    		}
			for (p = pr->sock_list; p != NULL; p = p->next) {
				if (!FD_ISSET(p->c_sock.fd, &(pr->socket_set))) {
					p->c_sock.state = SOCKET_STATE_CLOSED;
				}
			}
		} else {
			// some sockets are ready
			// check mqueue
			if (*(pr->mq) != NULL) {
				if (FD_ISSET(*(pr->mq), &(pr->socket_set))) {
					char msg_buffer[SERVER_BUFFER_SIZE];
					memset(msg_buffer, 0x00, sizeof(msg_buffer));
        			int bytes_read = mq_receive(*(pr->mq), msg_buffer, SERVER_BUFFER_SIZE, NULL);
        			if (bytes_read >= 0) {
						printf("Server: Received message: %s\n", msg_buffer);
						if (strcmp(msg_buffer,"#") == 0) {
							pr->state_worked = 0;
							continue;
						}
					} else {
						printf("Server: None \n");
					}
				}
			}
			// check listeners
			if (pr->listeners_list != NULL) {
				// проходим по списку сокетов в поисках установленного соединения
				for (p = pr->listeners_list; p != NULL; p = p->next) {
					if (FD_ISSET(p->c_sock.fd, &(pr->socket_set))) {

						// принимаем соединение
    					new_socket = accept(p->c_sock.fd, (struct sockaddr *) &(pr->serv_address),  (socklen_t*) &(pr->addrlen));
    					printf("new_socket = %d pid = %d\n", new_socket, getpid());

    					sprintf(logger_buffer, "new_socket = %d pid = %d\n", new_socket, getpid());
    					if (mq_send(pr->extra, logger_buffer, SERVER_BUFFER_SIZE, 0) < 0) {
        					perror(logger_buffer);
    					}
    					if (new_socket < 0) 
    					{ 
        					printf("accept() failed\n"); 
        					sprintf(logger_buffer, "accept() failed\n");
    						if (mq_send(pr->extra, logger_buffer, SERVER_BUFFER_SIZE, 0) < 0) {
        						perror(logger_buffer);
    						}
        					continue;
    					} 

    					//FD_CLR(p->c_sock.fd, &(pr->socket_set));
    					int file_flags = fcntl(new_socket, F_GETFL, 0);
    					if (file_flags == -1)
    					{
        					printf("Fail to receive socket flags\n");
        					sprintf(logger_buffer, "Fail to receive socket flags\n");
    						if (mq_send(pr->extra, logger_buffer, SERVER_BUFFER_SIZE, 0) < 0) {
        						perror(logger_buffer);
    						}
        					continue;
    					}

    					if (fcntl(new_socket, F_SETFL, file_flags | O_NONBLOCK))
    					{	
        					printf("Fail to set flag 'O_NONBLOCK' for socket\n");
        					sprintf(logger_buffer, "Fail to set flag 'O_NONBLOCK' for socket\n");
    						if (mq_send(pr->extra, logger_buffer, SERVER_BUFFER_SIZE, 0) < 0) {
        						perror(logger_buffer);
    						}
        					continue;
    					}

    					struct client_socket cl_sock = init_client_socket(new_socket, SERVER_BUFFER_SIZE, SOCKET_STATE_SEND_GREETING, SERVER_MAX_RECIPIENTS, 1);

        				// добавить новый сокет в список сокетов процесса
        				struct client_socket_list *new_scket = malloc(sizeof(struct client_socket_list));
        				new_scket->c_sock = cl_sock;
        				new_scket->next = pr->sock_list;
        				pr->sock_list = new_scket;

        				if (pr->max_fd < cl_sock.fd) {
            				pr->max_fd = cl_sock.fd;
        				}
					}
				}
			}
			// end check listeners

			// check clients
			if (pr->sock_list != NULL) {
				for (p = pr->sock_list; p != NULL; p = p->next) {
					if (FD_ISSET(p->c_sock.fd, &(pr->socket_set))) {
						p->c_sock.flag = 0;
        				// call smtp_handler for socket
        				new_smtp_handler_with_states(&(p->c_sock));
					}

					if (FD_ISSET(p->c_sock.fd, &temp)) {
						p->c_sock.flag = 1;
        				// call smtp_handler for socket
        				new_smtp_handler_with_states(&(p->c_sock));
					}
				}
			}	
		}
	}
	return;
}

void new_smtp_handler_with_states(struct client_socket *c_sock) {
	printf("SMTP handler begin");
	int received_bytes_count;
	char buffer_output[SERVER_BUFFER_SIZE];
	char *eol;
	struct sockaddr_in address;
	address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( SERVER_PORT );

    if (c_sock->state == SOCKET_STATE_SEND_GREETING) {
        sprintf(buffer_output, "%s %s SMTP CCSMTP\n",HEADER_220,SERVER_DOMAIN);
    	printf("Server: %d, 220: %s", c_sock->fd, buffer_output);
    	if (c_sock->fd > 0){
    		int sc = 0;
    		sc = send(c_sock->fd, buffer_output, strlen(buffer_output), 0);
    		if (sc > 0) {
    			c_sock->state = SOCKET_STATE_INIT;
    			c_sock->flag = 0; // на чтение
    			return;
    		} else {
    			if (errno == EWOULDBLOCK) {
    				return;
    			} else {
    				c_sock->state = SOCKET_STATE_CLOSED;
					return;
    			}
    		}
    	}
    }

	int buffer_left = SERVER_BUFFER_SIZE - c_sock->buffer_offset - 1;
	if (buffer_left == 0) {
		//printf(HEADER_500_TOO_LONG);
		sprintf(buffer_output, HEADER_500_TOO_LONG);
    	printf("Server: %d, TOO_LONG: %s", c_sock->fd, buffer_output);
    	if (send(c_sock->fd, buffer_output, strlen(buffer_output), 0) == -1) {
    		if (errno == EWOULDBLOCK) {
    			return;
    		} else {
    			c_sock->state = SOCKET_STATE_CLOSED;
				return;
    		}
    	} 
		c_sock->buffer_offset = 0;
		c_sock->flag = 0; // на чтение
		return;
	}

	if (c_sock->flag == 0) {
		received_bytes_count = recv(c_sock->fd, c_sock->buffer + c_sock->buffer_offset, buffer_left, 0);

		// считали 0 байт - клиент перестал отправлять данные
		if (received_bytes_count == 0) {
			printf("remote host closed socket %d\n", c_sock->fd);
			c_sock->state = SOCKET_STATE_CLOSED;
			return;
		} else {
			if (received_bytes_count < 0) {
				if (errno == EWOULDBLOCK) {
					return;
				} else {
					printf("problems with socket %d\n", c_sock->fd);
					c_sock->state = SOCKET_STATE_CLOSED;
					return;
				}
			} else {
				if (strstr(c_sock->buffer, "\r\n")) {
					c_sock->flag = 1;
					return;
				}
			}
		}
		
	} else {
		printf("Server: %d - receive ok\n");

		while (strstr(c_sock->buffer, "\r\n")) {
			eol = strstr(c_sock->buffer, "\r\n");
			eol[0] = '\0'; 

			// обработка ключевых слов
			printf("Client: %d, message: %s\n", c_sock->fd, c_sock->buffer);

			if (!c_sock->input_message) {
				char *message_buffer = (char *) malloc (SERVER_BUFFER_SIZE);
				strcpy(message_buffer, c_sock->buffer);
				// конец строки для сравнения (потом переделать под разделение на данные и команды)
				c_sock->buffer[4] = '\0';
				int err_code = 0;

				if (STR_EQUAL(c_sock->buffer, "HELO")) { 
					// начальное приветствие
					err_code = handle_HELO(c_sock,message_buffer,buffer_output,&address);
				} else if (STR_EQUAL(c_sock->buffer, "EHLO")) { 
					// улучшенное начальное приветствие
					err_code = handle_EHLO(c_sock,message_buffer,buffer_output,&address);
				} else if (STR_EQUAL(c_sock->buffer, "MAIL")) { 
					// получено новое письмо от
					err_code = handle_MAIL(c_sock,message_buffer,buffer_output,&address);
				} else if (STR_EQUAL(c_sock->buffer, "RCPT")) { 
					// письмо направлено ... 
					err_code = handle_RCPT(c_sock,message_buffer,buffer_output,&address);
				} else if (STR_EQUAL(c_sock->buffer, "DATA")) { 
					// содержимое письма
					err_code = handle_DATA(c_sock,message_buffer,buffer_output,&address);
				} else if (STR_EQUAL(c_sock->buffer, "RSET")) { 
					// сброс отправителя/получателей
					err_code = handle_RSET(c_sock,message_buffer,buffer_output,&address);
				} else if (STR_EQUAL(c_sock->buffer, "NOOP")) { 
					// ничего не делать
					err_code = handle_NOOP(c_sock,message_buffer,buffer_output,&address);
				} else if (STR_EQUAL(c_sock->buffer, "QUIT")) { 
					// закрыть соединение
					return handle_QUIT(c_sock,message_buffer,buffer_output,&address);
				} else { 
					// метод не был определен
					handle_NOT_IMPLEMENTED(c_sock,message_buffer,buffer_output,&address);
				}
				if (err_code < 0) {
					allowed_commands(c_sock, buffer_output);
				}
				free(message_buffer);
			} else {
			handle_TEXT(c_sock,buffer_output,"../maildir/");
		}

		// смещаем буфер для обработки следующей команды
		memmove(c_sock->buffer, eol + 2, SERVER_BUFFER_SIZE - (eol + 2 - c_sock->buffer));

		}
		c_sock->flag = 0;
	}
}

int allowed_commands(struct client_socket *c_sock,char buffer_output[]) {
	
	sprintf(buffer_output, HEADER_451_COMMAND_NOT_ALLOWED);
    printf("Server: %d, NOT_ALLOWED: %s", c_sock->fd, buffer_output);
    send(c_sock->fd, buffer_output, strlen(buffer_output), 0);
	switch (c_sock->state) {
		case SOCKET_STATE_INIT: {
			sprintf(buffer_output, "Allowed commands are: HELO, EHLO, NOOP, QUIT\n");
			if (send(c_sock->fd, buffer_output, strlen(buffer_output), 0) < 0) {
				if (errno == EWOULDBLOCK) {
					return 1;
				}
			}
			break;
		}
		case SOCKET_STATE_WAIT: {
			sprintf(buffer_output, "Allowed commands are: RSET, MAIL, NOOP, QUIT\n");
			if (send(c_sock->fd, buffer_output, strlen(buffer_output), 0) < 0) {
				if (errno == EWOULDBLOCK) {
					return 1;
				}
			}
			break;
		}
		case SOCKET_STATE_MAIL_CREATED_NO_RECEPIENTS: {
			sprintf(buffer_output, "Allowed commands are: RSET, RCPT, NOOP, QUIT\n");
			if (send(c_sock->fd, buffer_output, strlen(buffer_output), 0) < 0) {
				if (errno == EWOULDBLOCK) {
					return 1;
				}
			}
			break;
		}
		case SOCKET_STATE_RECEPIENTS_SET: {
			sprintf(buffer_output, "Allowed commands are: RSET, RCPT, NOOP, DATA, QUIT\n");
			if (send(c_sock->fd, buffer_output, strlen(buffer_output), 0) < 0) {
				if (errno == EWOULDBLOCK) {
					return 1;
				}
			}
			break;
		}
		case SOCKET_STATE_WRITING_DATA: {
			sprintf(buffer_output, "Allowed commands are: .\n");
			if (send(c_sock->fd, buffer_output, strlen(buffer_output), 0) < 0) {
				if (errno == EWOULDBLOCK) {
					return 1;
				}
			}
			break;
		}
		default: {
			break;
		}
	}
	return 0;
}

int handle_HELO(struct client_socket *c_sock, char *msg_buffer, char buffer_output[], struct sockaddr_in *address) {
	if (c_sock->state == SOCKET_STATE_INIT) {
		char* host = get_domain(msg_buffer);
    	int addrlen = sizeof(*address);
    	getpeername(c_sock->fd, (struct sockaddr*)address, (socklen_t*)&addrlen);
    	char* host_ip = ip_to_hostname(inet_ntoa(address->sin_addr));
    	if (strcmp(host, host_ip) == 0) {
        	sprintf(buffer_output, "%s %s\r\n", HEADER_250_OK, host);
        	printf("Server: %d, HELO: %s", c_sock->fd, buffer_output);
    	} else {
        	sprintf(buffer_output, "%s %s\r\n", HEADER_252_OK, host);
        	printf("Server: %d, HELO: %s", c_sock->fd, buffer_output);
    	}
    	free(host);
    	if (c_sock->fd > 0) {
    		if (send(c_sock->fd, buffer_output, strlen(buffer_output), 0) < 0) {
    			if (errno == EWOULDBLOCK) {
					return 1;
				}
    		}
    	}
    	c_sock->state = SOCKET_STATE_WAIT;
    	return 0;
	}
	return -1;
}

int handle_EHLO(struct client_socket *c_sock, char *msg_buffer, char buffer_output[], struct sockaddr_in *address) {
	if (c_sock->state == SOCKET_STATE_INIT) {
		char* host = get_domain(msg_buffer);
    	int addrlen = sizeof(*address);
    	getpeername(c_sock->fd, (struct sockaddr*)address, (socklen_t*)&addrlen);
    
    	/*char* host_ip = ip_to_hostname(inet_ntoa(address->sin_addr));
    	printf("%s\n", host_ip);
    	if (strcmp(host, host_ip) == 0) {
        	sprintf(buffer_output, "%s %s\r\n", HEADER_250_OK_WITH_NAME, host);
        	printf("Server: %d, EHLO: %s", c_sock->fd, buffer_output);
    	} else {
        	sprintf(buffer_output, "%s %s\r\n", HEADER_252_OK, host);
        	printf("Server: %d, EHLO: %s", c_sock->fd, buffer_output);
    	}*/
    	sprintf(buffer_output, "%s %s\r\n", HEADER_250_OK_WITH_NAME, host);
        printf("Server: %d, EHLO: %s", c_sock->fd, buffer_output);
    	if (c_sock->fd > 0) {
    		if (send(c_sock->fd, buffer_output, strlen(buffer_output), 0) < 0) {
    			if (errno == EWOULDBLOCK) {
					return 1;
				}
    		}
    	}
    	free(host);
    	c_sock->state = SOCKET_STATE_WAIT;
    	return 0;
	}
	return -1;
}

int handle_MAIL(struct client_socket *c_sock, char *msg_buffer, char buffer_output[], struct sockaddr_in *address) {
	if (c_sock->state == SOCKET_STATE_WAIT) {
		c_sock->message->from = get_mail(msg_buffer);
		if ((c_sock->message->from != NULL) && (strcmp(c_sock->message->from, "") != 0)) {
			sprintf(buffer_output, HEADER_250_OK);
    		printf("Server: %d, MAIL: %s %s", c_sock->fd, c_sock->message->from, buffer_output);
    		c_sock->state = SOCKET_STATE_MAIL_CREATED_NO_RECEPIENTS;
		} else {
			sprintf(buffer_output, HEADER_450_MAILBOX_UNAVAILABLE);
    		printf("Server: %d, MAIL: %s", c_sock->fd, buffer_output);
		}
		if (c_sock->fd > 0) {
			if (send(c_sock->fd, buffer_output, strlen(buffer_output), 0) < 0) {
				if (errno == EWOULDBLOCK) {
					return 1;
				}
			}
		}
    
    	return 0;
	}
	return -1;
}

int handle_RCPT(struct client_socket *c_sock, char *msg_buffer, char buffer_output[], struct sockaddr_in *address) {
	if ((c_sock->state == SOCKET_STATE_MAIL_CREATED_NO_RECEPIENTS) || (c_sock->state == SOCKET_STATE_RECEPIENTS_SET) ){
		if (c_sock->message->recepients_num >= SERVER_MAX_RECIPIENTS) {
			sprintf(buffer_output, HEADER_451_EXCEEDED_RECIPIENTS);
    		printf("Server: %d, RCPT: %s", c_sock->fd, buffer_output);
		} else {
			c_sock->message->to[c_sock->message->recepients_num] = get_mail(msg_buffer);
			if ((c_sock->message->to[c_sock->message->recepients_num] != NULL) && (strcmp(c_sock->message->to[c_sock->message->recepients_num], "") != 0)) {
				c_sock->message->recepients_num++;
				sprintf(buffer_output, HEADER_250_OK);
    			printf("Server: %d, RCPT: %s", c_sock->fd, buffer_output);
    			c_sock->state = SOCKET_STATE_RECEPIENTS_SET;
			} else {
				sprintf(buffer_output, HEADER_450_MAILBOX_UNAVAILABLE);
    			printf("Server: %d, MAIL: %s", c_sock->fd, buffer_output);
			}
		}

		if (c_sock->fd > 0) {
			if (send(c_sock->fd, buffer_output, strlen(buffer_output), 0) < 0) {
				if (errno == EWOULDBLOCK) {
					return 1;
				}
			}
		}
    
    	return 0;
	}
	return -1;
}

int handle_RSET(struct client_socket *c_sock, char *msg_buffer, char buffer_output[], struct sockaddr_in *address) {
	if ((c_sock->state == SOCKET_STATE_WAIT) || (c_sock->state == SOCKET_STATE_MAIL_CREATED_NO_RECEPIENTS) ||
		(c_sock->state == SOCKET_STATE_RECEPIENTS_SET) || (c_sock->state == SOCKET_STATE_DELIVERING)) {
		sprintf(buffer_output, HEADER_250_OK_RESET);
    	printf("Server: %d, RSET: %s", c_sock->fd, buffer_output);
    	free(c_sock->message->from);
    	c_sock->message->from = NULL;
    	int i = 0;
    	for (i = 0; i < c_sock->message->recepients_num - 1; i++) {
    		free(c_sock->message->to[i]);
    		c_sock->message->to[i] = NULL;
    	}
    	c_sock->message->recepients_num = 0;
    	c_sock->state = SOCKET_STATE_WAIT;
    	if (c_sock->fd > 0) {
    		if (send(c_sock->fd, buffer_output, strlen(buffer_output), 0) < 0) {
    			if (errno == EWOULDBLOCK) {
					return 1;
				}
    		}
    	}
    
    	return 0;
	}
	return -1;
}

int handle_DATA(struct client_socket *c_sock, char *msg_buffer, char buffer_output[], struct sockaddr_in *address) {
	if (c_sock->state == SOCKET_STATE_RECEPIENTS_SET) {
		sprintf(buffer_output, HEADER_354_START);
    	printf("Server: %d, DATA: %s", c_sock->fd, buffer_output);
    	
    	if (c_sock->fd > 0) {
    		if (send(c_sock->fd, buffer_output, strlen(buffer_output), 0) < 0) {
    			if (errno == EWOULDBLOCK) {
					return 1;
				}
    		} else {
    			c_sock->message->body = (char *)malloc(1);
    	c_sock->message->body[0] = '\0';
    	c_sock->message->body_length = 0;
    	c_sock->input_message = 1;
    	c_sock->state = SOCKET_STATE_WRITING_DATA;
    		}
    	}
    
    	return 0;
	}
	return -1;
}

int handle_QUIT(struct client_socket *c_sock, char *msg_buffer, char buffer_output[], struct sockaddr_in *address) {
	sprintf(buffer_output, HEADER_221_OK);
    printf("Server: %d, QUIT: %s", c_sock->fd, buffer_output);
    if (c_sock->fd > 0) {
    	if (send(c_sock->fd, buffer_output, strlen(buffer_output), 0) < 0) {
    		if (errno == EWOULDBLOCK) {
				return 1;
			}
    	}
    }
    c_sock->state = SOCKET_STATE_CLOSED;
    //free(msg_buffer);
    return 1;
}

int handle_NOOP(struct client_socket *c_sock, char *msg_buffer, char buffer_output[], struct sockaddr_in *address) {
	sprintf(buffer_output, HEADER_250_OK_NOOP);
    printf("Server: %d, NOOP: %s", c_sock->fd, buffer_output);
    if (c_sock->fd > 0) {
    	if (send(c_sock->fd, buffer_output, strlen(buffer_output), 0) < 0) {
    		if (errno == EWOULDBLOCK) {
				return 1;
			}
    	}
    }
    return 0;
}

int handle_NOT_IMPLEMENTED(struct client_socket *c_sock, char *msg_buffer, char buffer_output[], struct sockaddr_in *address) {
	sprintf(buffer_output, HEADER_502_NOT_IMPLEMENTED);
    printf("Server: %d, NOT_IMPLEMENTED: %s", c_sock->fd, buffer_output);
    if (c_sock->fd > 0) {
    	if (send(c_sock->fd, buffer_output, strlen(buffer_output), 0) < 0) {
    		if (errno == EWOULDBLOCK) {
				return 1;
			}
    	}
    }
    return 0;
}

int handle_TEXT(struct client_socket *c_sock, char buffer_output[],char* maildir_path) {
	printf("Client: %d, In-MESSAGE: %s", c_sock->fd, c_sock->buffer);

	if (strcmp(c_sock->buffer, ".") == 0) {
    	printf("Server: %d, MESSAGE: %s", c_sock->fd, c_sock->message->body);
    	c_sock->state = SOCKET_STATE_DELIVERING;
    	save_message(c_sock->message, maildir_path);
    	free(c_sock->message->body);
    	c_sock->input_message = 0;
    	if (handle_RSET(c_sock, NULL, buffer_output, NULL) == -1) {
    		allowed_commands(c_sock, buffer_output);
    	}
	} else {
		//printf("MALLOC_SIZE = %d\n", (strlen(c_sock->message->body) + SERVER_BUFFER_SIZE * 2) * sizeof(char));
		if (strlen(c_sock->message->body) + strlen(c_sock->buffer) >= c_sock->message->body_length) {
			int malloc_size = strlen(c_sock->message->body) + SERVER_BUFFER_SIZE * 2;
			c_sock->message->body = (char *) realloc(c_sock->message->body, malloc_size);
			c_sock->message->body_length = malloc_size;	
		}
		strcat(c_sock->message->body, c_sock->buffer);
		int len = strlen(c_sock->message->body);
		c_sock->message->body[len] = '\n';
		c_sock->message->body[len + 1] = '\0';
	}
	return 0;
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


