#ifndef SMTP_H
#define SMTP_H

#include "../common/header.h"
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <time.h>
#include <netdb.h>
#include <sys/time.h>
#include <sys/select.h>
#include "process.h"

// #include <syslog.h>



int check_clients_by_timeout(struct client_socket_list *clients, int max_fd, struct timeval timeout);
int run_process(struct process *pr);
void new_smtp_handler_with_states(struct client_socket *c_sock);
int allowed_commands(struct client_socket *c_sock,char buffer_output[]);
int handle_HELO(struct client_socket *c_sock, char *msg_buffer, char buffer_output[], struct sockaddr_in *address);
int handle_EHLO(struct client_socket *c_sock, char *msg_buffer, char buffer_output[], struct sockaddr_in *address);
int handle_MAIL(struct client_socket *c_sock, char *msg_buffer, char buffer_output[], struct sockaddr_in *address);
int handle_RCPT(struct client_socket *c_sock, char *msg_buffer, char buffer_output[], struct sockaddr_in *address);
int handle_RSET(struct client_socket *c_sock, char *msg_buffer, char buffer_output[], struct sockaddr_in *address);
int handle_DATA(struct client_socket *c_sock, char *msg_buffer, char buffer_output[], struct sockaddr_in *address) ;
int handle_QUIT(struct client_socket *c_sock, char *msg_buffer, char buffer_output[], struct sockaddr_in *address);
int handle_NOOP(struct client_socket *c_sock, char *msg_buffer, char buffer_output[], struct sockaddr_in *address) ;
int handle_NOT_IMPLEMENTED(struct client_socket *c_sock, char *msg_buffer, char buffer_output[], struct sockaddr_in *address) ;
int handle_TEXT(struct client_socket *c_sock, char buffer_output[],char* maildir_path);

void smtp_handler(int *socket_fd, const int pid);

#endif