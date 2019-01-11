#include <stdio.h>
#include <stdlib.h>
#include <CUnit/Basic.h>

#include "utils.h"

#include "../smtp.h"
#include "../socket.h"

TEST_FUNCT(create_client_socket_no_message) {
	struct client_socket *c_sock = (struct client_socket *)malloc(sizeof(struct client_socket));
	int fd = 0;
	int buffer_size = 2048;
	int state = SOCKET_STATE_INIT;
	int max_recepients = 10;
	int needs_message = 0;

	*c_sock = init_client_socket(fd, buffer_size, state, max_recepients, needs_message);

	CU_ASSERT_EQUAL(c_sock->fd,0);
	CU_ASSERT_EQUAL(sizeof(c_sock->buffer),8);
	CU_ASSERT_EQUAL(c_sock->state,SOCKET_STATE_INIT);
	CU_ASSERT_PTR_EQUAL(c_sock->message,NULL);
	free(c_sock);
}

TEST_FUNCT(create_client_socket_has_message) {
	struct client_socket *c_sock = (struct client_socket *)malloc(sizeof(struct client_socket));
	int fd = 0;
	int buffer_size = 2048;
	int state = SOCKET_STATE_INIT;
	int max_recepients = 10;
	int needs_message = 1;

	*c_sock = init_client_socket(fd, buffer_size, state, max_recepients, needs_message);

	CU_ASSERT_EQUAL(c_sock->fd,0);
	CU_ASSERT_EQUAL(sizeof(c_sock->buffer),8);
	CU_ASSERT_EQUAL(c_sock->state,SOCKET_STATE_INIT);
	CU_ASSERT_PTR_NOT_EQUAL(c_sock->message,NULL);
	CU_ASSERT_EQUAL(c_sock->message->recepients_num,0);
	free(c_sock);
}

TEST_FUNCT(handle_helo_state_init) {
	char *msg_buffer = "HELO myserver.com";
	char buffer_output[SERVER_BUFFER_SIZE];
	struct client_socket *c_sock = (struct client_socket *)malloc(sizeof(struct client_socket));
	int fd = 0;

	int buffer_size = 2048;
	int state = SOCKET_STATE_INIT;
	int max_recepients = 10;
	int needs_message = 1;

	*c_sock = init_client_socket(fd, buffer_size, state, max_recepients, needs_message);

	struct sockaddr_in sock_address;

    sock_address.sin_family = AF_INET;
    sock_address.sin_addr.s_addr = INADDR_ANY;
    sock_address.sin_port = htons( SERVER_PORT );

	int err = handle_HELO(c_sock, msg_buffer, buffer_output, &sock_address) ;
	CU_ASSERT_EQUAL(c_sock->fd,0);

	CU_ASSERT_EQUAL(c_sock->state,SOCKET_STATE_WAIT);
	CU_ASSERT_EQUAL(err,0);
	free(c_sock);
}

TEST_FUNCT(handle_helo_state_incorrect) {
	char *msg_buffer = "HELO myserver.com";
	char buffer_output[SERVER_BUFFER_SIZE];
	struct client_socket *c_sock = (struct client_socket *)malloc(sizeof(struct client_socket));
	int fd = 0;

	int buffer_size = 2048;
	int state = SOCKET_STATE_CLOSED;
	int max_recepients = 10;
	int needs_message = 1;

	*c_sock = init_client_socket(fd, buffer_size, state, max_recepients, needs_message);

	struct sockaddr_in sock_address;

    sock_address.sin_family = AF_INET;
    sock_address.sin_addr.s_addr = INADDR_ANY;
    sock_address.sin_port = htons( SERVER_PORT );

	int err = handle_HELO(c_sock, msg_buffer, buffer_output, &sock_address) ;
	CU_ASSERT_EQUAL(c_sock->fd,0);

	CU_ASSERT_NOT_EQUAL(c_sock->state,SOCKET_STATE_WAIT);
	CU_ASSERT_EQUAL(err,-1);
	free(c_sock);
}

TEST_FUNCT(handle_ehlo_state_correct) {
	char *msg_buffer = "EHLO myserver.com";
	char buffer_output[SERVER_BUFFER_SIZE];
	struct client_socket *c_sock = (struct client_socket *)malloc(sizeof(struct client_socket));
	int fd = 0;

	int buffer_size = 2048;
	int state = SOCKET_STATE_INIT;
	int max_recepients = 10;
	int needs_message = 1;

	*c_sock = init_client_socket(fd, buffer_size, state, max_recepients, needs_message);

	struct sockaddr_in sock_address;

    sock_address.sin_family = AF_INET;
    sock_address.sin_addr.s_addr = INADDR_ANY;
    sock_address.sin_port = htons( SERVER_PORT );

	int err = handle_EHLO(c_sock, msg_buffer, buffer_output, &sock_address) ;
	CU_ASSERT_EQUAL(c_sock->fd,0);

	CU_ASSERT_EQUAL(c_sock->state,SOCKET_STATE_WAIT);
	CU_ASSERT_EQUAL(err,0);
	free(c_sock);
}

TEST_FUNCT(handle_ehlo_state_incorrect) {
	char *msg_buffer = "EHLO myserver.com";
	char buffer_output[SERVER_BUFFER_SIZE];
	struct client_socket *c_sock = (struct client_socket *)malloc(sizeof(struct client_socket));
	int fd = 0;

	int buffer_size = 2048;
	int state = SOCKET_STATE_CLOSED;
	int max_recepients = 10;
	int needs_message = 1;

	*c_sock = init_client_socket(fd, buffer_size, state, max_recepients, needs_message);

	struct sockaddr_in sock_address;

    sock_address.sin_family = AF_INET;
    sock_address.sin_addr.s_addr = INADDR_ANY;
    sock_address.sin_port = htons( SERVER_PORT );

	int err = handle_EHLO(c_sock, msg_buffer, buffer_output, &sock_address) ;
	CU_ASSERT_EQUAL(c_sock->fd,0);

	CU_ASSERT_NOT_EQUAL(c_sock->state,SOCKET_STATE_WAIT);
	CU_ASSERT_EQUAL(err,-1);
	free(c_sock);
}

TEST_FUNCT(handle_mail_incorrect_command) {
	char *msg_buffer = "MAIL test@test.ru";
	char *check_mail = "test@test.ru";
	char buffer_output[SERVER_BUFFER_SIZE];
	struct client_socket *c_sock = (struct client_socket *)malloc(sizeof(struct client_socket));
	int fd = 0;

	int buffer_size = 2048;
	int state = SOCKET_STATE_WAIT;
	int max_recepients = 10;
	int needs_message = 1;

	*c_sock = init_client_socket(fd, buffer_size, state, max_recepients, needs_message);

	struct sockaddr_in sock_address;

    sock_address.sin_family = AF_INET;
    sock_address.sin_addr.s_addr = INADDR_ANY;
    sock_address.sin_port = htons( SERVER_PORT );

	int err = handle_MAIL(c_sock, msg_buffer, buffer_output, &sock_address) ;
	CU_ASSERT_EQUAL(c_sock->fd,0);

	CU_ASSERT_NOT_EQUAL(c_sock->state,SOCKET_STATE_MAIL_CREATED_NO_RECEPIENTS);
	CU_ASSERT_EQUAL(err,0);
	CU_ASSERT_EQUAL(strcmp(HEADER_450_MAILBOX_UNAVAILABLE, buffer_output),0);
	CU_ASSERT_PTR_EQUAL( c_sock->message->from,NULL);
	free(c_sock);
}

TEST_FUNCT(handle_mail_correct) {
	char *msg_buffer = "MAIL <test@test.ru>";
	char *check_mail = "test@test.ru";
	char buffer_output[SERVER_BUFFER_SIZE];
	struct client_socket *c_sock = (struct client_socket *)malloc(sizeof(struct client_socket));
	int fd = 0;

	int buffer_size = 2048;
	int state = SOCKET_STATE_WAIT;
	int max_recepients = 10;
	int needs_message = 1;

	*c_sock = init_client_socket(fd, buffer_size, state, max_recepients, needs_message);

	struct sockaddr_in sock_address;

    sock_address.sin_family = AF_INET;
    sock_address.sin_addr.s_addr = INADDR_ANY;
    sock_address.sin_port = htons( SERVER_PORT );

	int err = handle_MAIL(c_sock, msg_buffer, buffer_output, &sock_address) ;
	CU_ASSERT_EQUAL(c_sock->fd,0);

	CU_ASSERT_EQUAL(c_sock->state,SOCKET_STATE_MAIL_CREATED_NO_RECEPIENTS);
	CU_ASSERT_EQUAL(err,0);
	CU_ASSERT_EQUAL(strcmp(check_mail, c_sock->message->from),0);
	free(c_sock);
}

TEST_FUNCT(handle_mail_incorrect_state) {
	char *msg_buffer = "MAIL <test@test.ru>";
	char *check_mail = "test@test.ru";
	char buffer_output[SERVER_BUFFER_SIZE];
	struct client_socket *c_sock = (struct client_socket *)malloc(sizeof(struct client_socket));
	int fd = 0;

	int buffer_size = 2048;
	int state = SOCKET_STATE_CLOSED;
	int max_recepients = 10;
	int needs_message = 1;

	*c_sock = init_client_socket(fd, buffer_size, state, max_recepients, needs_message);

	struct sockaddr_in sock_address;

    sock_address.sin_family = AF_INET;
    sock_address.sin_addr.s_addr = INADDR_ANY;
    sock_address.sin_port = htons( SERVER_PORT );

	int err = handle_MAIL(c_sock, msg_buffer, buffer_output, &sock_address) ;
	CU_ASSERT_EQUAL(c_sock->fd,0);

	CU_ASSERT_NOT_EQUAL(c_sock->state,SOCKET_STATE_MAIL_CREATED_NO_RECEPIENTS);
	CU_ASSERT_EQUAL(err,-1);
	CU_ASSERT_PTR_EQUAL( c_sock->message->from,NULL);
	free(c_sock);
}

TEST_FUNCT(handle_rcpt_correct) {
	char *msg_buffer = "RCPT <test@test.ru>";
	char *check_mail = "test@test.ru";
	char buffer_output[SERVER_BUFFER_SIZE];
	struct client_socket *c_sock = (struct client_socket *)malloc(sizeof(struct client_socket));
	int fd = 0;

	int buffer_size = 2048;
	int state = SOCKET_STATE_MAIL_CREATED_NO_RECEPIENTS;
	int max_recepients = 10;
	int needs_message = 1;

	*c_sock = init_client_socket(fd, buffer_size, state, max_recepients, needs_message);

	struct sockaddr_in sock_address;

    sock_address.sin_family = AF_INET;
    sock_address.sin_addr.s_addr = INADDR_ANY;
    sock_address.sin_port = htons( SERVER_PORT );

    c_sock->message->from = "testfrom@test.ru";

	int err = handle_RCPT(c_sock, msg_buffer, buffer_output, &sock_address) ;
	CU_ASSERT_EQUAL(c_sock->fd,0);

	CU_ASSERT_EQUAL(strcmp(HEADER_250_OK, buffer_output),0);
	CU_ASSERT_EQUAL(c_sock->state,SOCKET_STATE_RECEPIENTS_SET);
	CU_ASSERT_EQUAL(err,0);
	CU_ASSERT_EQUAL(strcmp(check_mail, c_sock->message->to[0]),0);
	CU_ASSERT_EQUAL(c_sock->message->recepients_num,1);
	free(c_sock);
}

TEST_FUNCT(handle_rcpt_incorrect_command) {
	char *msg_buffer = "RCPT test@test.ru";
	char *check_mail = "test@test.ru";
	char buffer_output[SERVER_BUFFER_SIZE];
	struct client_socket *c_sock = (struct client_socket *)malloc(sizeof(struct client_socket));
	int fd = 0;

	int buffer_size = 2048;
	int state = SOCKET_STATE_MAIL_CREATED_NO_RECEPIENTS;
	int max_recepients = 10;
	int needs_message = 1;

	*c_sock = init_client_socket(fd, buffer_size, state, max_recepients, needs_message);

	struct sockaddr_in sock_address;

    sock_address.sin_family = AF_INET;
    sock_address.sin_addr.s_addr = INADDR_ANY;
    sock_address.sin_port = htons( SERVER_PORT );

    c_sock->message->from = "testfrom@test.ru";

	int err = handle_RCPT(c_sock, msg_buffer, buffer_output, &sock_address) ;
	CU_ASSERT_EQUAL(c_sock->fd,0);

	CU_ASSERT_EQUAL(strcmp(HEADER_450_MAILBOX_UNAVAILABLE, buffer_output),0);
	CU_ASSERT_NOT_EQUAL(c_sock->state,SOCKET_STATE_RECEPIENTS_SET);
	CU_ASSERT_EQUAL(err,0);
	CU_ASSERT_PTR_EQUAL(c_sock->message->to[0],NULL);
	CU_ASSERT_NOT_EQUAL(c_sock->message->recepients_num,1);
	free(c_sock);
}

TEST_FUNCT(handle_rcpt_incorrect_state) {
	char *msg_buffer = "RCPT <test@test.ru>";
	char *check_mail = "test@test.ru";
	char buffer_output[SERVER_BUFFER_SIZE];
	struct client_socket *c_sock = (struct client_socket *)malloc(sizeof(struct client_socket));
	int fd = 0;

	int buffer_size = 2048;
	int state = SOCKET_STATE_CLOSED;
	int max_recepients = 10;
	int needs_message = 1;

	*c_sock = init_client_socket(fd, buffer_size, state, max_recepients, needs_message);

	struct sockaddr_in sock_address;

    sock_address.sin_family = AF_INET;
    sock_address.sin_addr.s_addr = INADDR_ANY;
    sock_address.sin_port = htons( SERVER_PORT );

    c_sock->message->from = "testfrom@test.ru";

	int err = handle_RCPT(c_sock, msg_buffer, buffer_output, &sock_address) ;
	CU_ASSERT_EQUAL(c_sock->fd,0);

	CU_ASSERT_NOT_EQUAL(strcmp(HEADER_250_OK, buffer_output),0);
	CU_ASSERT_NOT_EQUAL(c_sock->state,SOCKET_STATE_RECEPIENTS_SET);
	CU_ASSERT_EQUAL(err,-1);
	CU_ASSERT_PTR_EQUAL(c_sock->message->to[0],NULL);
	CU_ASSERT_NOT_EQUAL(c_sock->message->recepients_num,1);
	free(c_sock);
}

TEST_FUNCT(handle_rcpt_exceeded_recepients) {
	char *msg_buffer = "RCPT <test@test.ru>";
	char *check_mail = "test@test.ru";
	char buffer_output[SERVER_BUFFER_SIZE];
	struct client_socket *c_sock = (struct client_socket *)malloc(sizeof(struct client_socket));
	int fd = 0;

	int buffer_size = 2048;
	int state = SOCKET_STATE_MAIL_CREATED_NO_RECEPIENTS;
	int max_recepients = SERVER_MAX_RECIPIENTS;
	int needs_message = 1;

	*c_sock = init_client_socket(fd, buffer_size, state, max_recepients, needs_message);

	struct sockaddr_in sock_address;

    sock_address.sin_family = AF_INET;
    sock_address.sin_addr.s_addr = INADDR_ANY;
    sock_address.sin_port = htons( SERVER_PORT );

    c_sock->message->from = "testfrom@test.ru";

    int err = 0;
    int i = 0;
    for (i = 0; i <= max_recepients; i++) {
    	err = handle_RCPT(c_sock, msg_buffer, buffer_output, &sock_address);
    }
	
	CU_ASSERT_EQUAL(c_sock->fd,0);

	CU_ASSERT_EQUAL(strcmp(HEADER_451_EXCEEDED_RECIPIENTS, buffer_output),0);
	CU_ASSERT_EQUAL(c_sock->state,SOCKET_STATE_RECEPIENTS_SET);
	CU_ASSERT_EQUAL(err,0);
	CU_ASSERT_EQUAL(c_sock->message->recepients_num,max_recepients);
	free(c_sock);
}

TEST_FUNCT(handle_rset_correct_no_recepients) {
	char *msg_buffer = "RSET ";
	char buffer_output[SERVER_BUFFER_SIZE];
	struct client_socket *c_sock = (struct client_socket *)malloc(sizeof(struct client_socket));
	int fd = 0;

	int buffer_size = 2048;
	int state = SOCKET_STATE_MAIL_CREATED_NO_RECEPIENTS;
	int max_recepients = 1;
	int needs_message = 1;

	*c_sock = init_client_socket(fd, buffer_size, state, max_recepients, needs_message);

	struct sockaddr_in sock_address;

    sock_address.sin_family = AF_INET;
    sock_address.sin_addr.s_addr = INADDR_ANY;
    sock_address.sin_port = htons( SERVER_PORT );

    c_sock->message->from = get_mail("<testfrom@test.ru>");

    int err = 0;
    err = handle_RSET(c_sock, msg_buffer, buffer_output, &sock_address);
	
	CU_ASSERT_EQUAL(c_sock->fd,0);

	CU_ASSERT_EQUAL(strcmp(HEADER_250_OK_RESET, buffer_output),0);
	CU_ASSERT_EQUAL(c_sock->state,SOCKET_STATE_WAIT);
	CU_ASSERT_EQUAL(err,0);
	CU_ASSERT_EQUAL(c_sock->message->recepients_num,0);
	free(c_sock);
}

TEST_FUNCT(handle_rset_correct_has_recepients) {
	char *msg_buffer = "RSET ";
	char buffer_output[SERVER_BUFFER_SIZE];
	struct client_socket *c_sock = (struct client_socket *)malloc(sizeof(struct client_socket));
	int fd = 0;

	int buffer_size = 2048;
	int state = SOCKET_STATE_RECEPIENTS_SET;
	int max_recepients = 1;
	int needs_message = 1;

	*c_sock = init_client_socket(fd, buffer_size, state, max_recepients, needs_message);

	struct sockaddr_in sock_address;

    sock_address.sin_family = AF_INET;
    sock_address.sin_addr.s_addr = INADDR_ANY;
    sock_address.sin_port = htons( SERVER_PORT );

    c_sock->message->from = get_mail("<testfrom@test.ru>");
    c_sock->message->to[0] = get_mail("<testfrom@test.ru>");

    int err = 0;
    err = handle_RSET(c_sock, msg_buffer, buffer_output, &sock_address);
	
	CU_ASSERT_EQUAL(c_sock->fd,0);

	CU_ASSERT_EQUAL(strcmp(HEADER_250_OK_RESET, buffer_output),0);
	CU_ASSERT_EQUAL(c_sock->state,SOCKET_STATE_WAIT);
	CU_ASSERT_EQUAL(err,0);
	CU_ASSERT_EQUAL(c_sock->message->recepients_num,0);
	free(c_sock);
}

TEST_FUNCT(handle_rset_incorrect_state) {
	char *msg_buffer = "RSET ";
	char buffer_output[SERVER_BUFFER_SIZE];
	struct client_socket *c_sock = (struct client_socket *)malloc(sizeof(struct client_socket));
	int fd = 0;

	int buffer_size = 2048;
	int state = SOCKET_STATE_CLOSED;
	int max_recepients = 1;
	int needs_message = 1;

	*c_sock = init_client_socket(fd, buffer_size, state, max_recepients, needs_message);

	struct sockaddr_in sock_address;

    sock_address.sin_family = AF_INET;
    sock_address.sin_addr.s_addr = INADDR_ANY;
    sock_address.sin_port = htons( SERVER_PORT );

    c_sock->message->from = get_mail("<testfrom@test.ru>");
    c_sock->message->to[0] = get_mail("<testfrom@test.ru>");

    int err = 0;
    err = handle_RSET(c_sock, msg_buffer, buffer_output, &sock_address);
	
	CU_ASSERT_EQUAL(c_sock->fd,0);
	CU_ASSERT_EQUAL(c_sock->state,SOCKET_STATE_CLOSED);
	CU_ASSERT_EQUAL(err,-1);
	free(c_sock);
}

TEST_FUNCT(test_prepare_client_before_getting_data_no_reset) {
	char *msg_buffer = "HELO myserver.com";
	char *msg_buffer2 = "MAIL <test@test.ru>";
	char *msg_buffer3 = "RCPT <test@test.ru>";
	char *check_mail = "test@test.ru";
	char buffer_output[SERVER_BUFFER_SIZE];
	struct client_socket *c_sock = (struct client_socket *)malloc(sizeof(struct client_socket));
	int fd = 0;

	int buffer_size = 2048;
	int state = SOCKET_STATE_INIT;
	int max_recepients = 1;
	int needs_message = 1;

	*c_sock = init_client_socket(fd, buffer_size, state, max_recepients, needs_message);

	struct sockaddr_in sock_address;

    sock_address.sin_family = AF_INET;
    sock_address.sin_addr.s_addr = INADDR_ANY;
    sock_address.sin_port = htons( SERVER_PORT );

    int err = 0;
    err = handle_HELO(c_sock, msg_buffer, buffer_output, &sock_address);
    CU_ASSERT_EQUAL(err,0);
	
	CU_ASSERT_EQUAL(c_sock->state,SOCKET_STATE_WAIT);
	err = handle_MAIL(c_sock, msg_buffer2, buffer_output, &sock_address);
	CU_ASSERT_EQUAL(strcmp(HEADER_250_OK, buffer_output),0);
	CU_ASSERT_EQUAL(err,0);
	
	CU_ASSERT_EQUAL(c_sock->state,SOCKET_STATE_MAIL_CREATED_NO_RECEPIENTS);
	err = handle_RCPT(c_sock, msg_buffer3, buffer_output, &sock_address);
	CU_ASSERT_EQUAL(strcmp(HEADER_250_OK, buffer_output),0);
	CU_ASSERT_EQUAL(strcmp(c_sock->message->from, check_mail),0);
	CU_ASSERT_EQUAL(strcmp(c_sock->message->to[0], check_mail),0);
	
	CU_ASSERT_EQUAL(c_sock->state,SOCKET_STATE_RECEPIENTS_SET);
	CU_ASSERT_EQUAL(err,0);
	free(c_sock);
}

TEST_FUNCT(handle_data_correct) {
	char *msg_buffer = "DATA ";
	char buffer_output[SERVER_BUFFER_SIZE];
	struct client_socket *c_sock = (struct client_socket *)malloc(sizeof(struct client_socket));
	int fd = 0;

	int buffer_size = 2048;
	int state = SOCKET_STATE_RECEPIENTS_SET;
	int max_recepients = 1;
	int needs_message = 1;

	*c_sock = init_client_socket(fd, buffer_size, state, max_recepients, needs_message);

	struct sockaddr_in sock_address;

    sock_address.sin_family = AF_INET;
    sock_address.sin_addr.s_addr = INADDR_ANY;
    sock_address.sin_port = htons( SERVER_PORT );

    c_sock->message->from = get_mail("<testfrom@test.ru>");
    c_sock->message->to[0] = get_mail("<testfrom@test.ru>");

    int err = 0;
    err = handle_DATA(c_sock, msg_buffer, buffer_output, &sock_address);
	
	CU_ASSERT_EQUAL(c_sock->fd,0);
	CU_ASSERT_EQUAL(c_sock->state,SOCKET_STATE_WRITING_DATA);
	CU_ASSERT_EQUAL(strcmp(HEADER_354_START, buffer_output),0);
	CU_ASSERT_EQUAL(c_sock->input_message,1);
	CU_ASSERT_EQUAL(err,0);
	free(c_sock);
}

TEST_FUNCT(handle_data_incorrect_state) {
	char *msg_buffer = "DATA ";
	char buffer_output[SERVER_BUFFER_SIZE];
	struct client_socket *c_sock = (struct client_socket *)malloc(sizeof(struct client_socket));
	int fd = 0;

	int buffer_size = 2048;
	int state = SOCKET_STATE_CLOSED;
	int max_recepients = 1;
	int needs_message = 1;

	*c_sock = init_client_socket(fd, buffer_size, state, max_recepients, needs_message);

	struct sockaddr_in sock_address;

    sock_address.sin_family = AF_INET;
    sock_address.sin_addr.s_addr = INADDR_ANY;
    sock_address.sin_port = htons( SERVER_PORT );

    c_sock->message->from = get_mail("<testfrom@test.ru>");
    c_sock->message->to[0] = get_mail("<testfrom@test.ru>");

    int err = 0;
    err = handle_DATA(c_sock, msg_buffer, buffer_output, &sock_address);
	
	CU_ASSERT_EQUAL(c_sock->fd,0);
	CU_ASSERT_NOT_EQUAL(c_sock->state,SOCKET_STATE_WRITING_DATA);
	CU_ASSERT_EQUAL(c_sock->input_message,0);
	CU_ASSERT_EQUAL(err,-1);
	free(c_sock);
}

TEST_FUNCT(handle_quit_correct) {
	char *msg_buffer = "QUIT ";
	char buffer_output[SERVER_BUFFER_SIZE];
	struct client_socket *c_sock = (struct client_socket *)malloc(sizeof(struct client_socket));
	int fd = 0;

	int buffer_size = 2048;
	int state = SOCKET_STATE_INIT;
	int max_recepients = 1;
	int needs_message = 1;

	*c_sock = init_client_socket(fd, buffer_size, state, max_recepients, needs_message);

	struct sockaddr_in sock_address;

    sock_address.sin_family = AF_INET;
    sock_address.sin_addr.s_addr = INADDR_ANY;
    sock_address.sin_port = htons( SERVER_PORT );

    c_sock->message->from = get_mail("<testfrom@test.ru>");
    c_sock->message->to[0] = get_mail("<testfrom@test.ru>");

    int err = 0;
    err = handle_QUIT(c_sock, msg_buffer, buffer_output, &sock_address);
	
	CU_ASSERT_EQUAL(c_sock->fd,0);
	CU_ASSERT_EQUAL(c_sock->state,SOCKET_STATE_CLOSED);
	CU_ASSERT_EQUAL(c_sock->input_message,0);
	CU_ASSERT_EQUAL(err,1);
	free(c_sock);
}

TEST_FUNCT(handle_noop_correct) {
	char *msg_buffer = "NOOP ";
	char buffer_output[SERVER_BUFFER_SIZE];
	struct client_socket *c_sock = (struct client_socket *)malloc(sizeof(struct client_socket));
	int fd = 0;

	int buffer_size = 2048;
	int state = SOCKET_STATE_INIT;
	int max_recepients = 1;
	int needs_message = 0;

	*c_sock = init_client_socket(fd, buffer_size, state, max_recepients, needs_message);

	struct sockaddr_in sock_address;

    sock_address.sin_family = AF_INET;
    sock_address.sin_addr.s_addr = INADDR_ANY;
    sock_address.sin_port = htons( SERVER_PORT );

    int err = 1;
    err = handle_NOOP(c_sock, msg_buffer, buffer_output, &sock_address);
	
	CU_ASSERT_EQUAL(c_sock->fd,0);
	CU_ASSERT_EQUAL(c_sock->state,state);
	CU_ASSERT_EQUAL(strcmp(HEADER_250_OK_NOOP, buffer_output),0);
	CU_ASSERT_EQUAL(err,0);
	free(c_sock);
}

TEST_FUNCT(handle_not_implemented_correct) {
	char *msg_buffer = "sdfsd ";
	char buffer_output[SERVER_BUFFER_SIZE];
	struct client_socket *c_sock = (struct client_socket *)malloc(sizeof(struct client_socket));
	int fd = 0;

	int buffer_size = 2048;
	int state = SOCKET_STATE_INIT;
	int max_recepients = 1;
	int needs_message = 0;

	*c_sock = init_client_socket(fd, buffer_size, state, max_recepients, needs_message);

	struct sockaddr_in sock_address;

    sock_address.sin_family = AF_INET;
    sock_address.sin_addr.s_addr = INADDR_ANY;
    sock_address.sin_port = htons( SERVER_PORT );

    int err = 1;
    err = handle_NOT_IMPLEMENTED(c_sock, msg_buffer, buffer_output, &sock_address);
	
	CU_ASSERT_EQUAL(c_sock->fd,0);
	CU_ASSERT_EQUAL(c_sock->state,state);
	CU_ASSERT_EQUAL(strcmp(HEADER_502_NOT_IMPLEMENTED, buffer_output),0);
	CU_ASSERT_EQUAL(err,0);
	free(c_sock);
}

TEST_FUNCT(handle_text_not_dot) {
	char *msg_buffer = "Test messsage ";
	char buffer_output[SERVER_BUFFER_SIZE];
	struct client_socket *c_sock = (struct client_socket *)malloc(sizeof(struct client_socket));
	int fd = 0;

	int buffer_size = 2048;
	int state = SOCKET_STATE_WRITING_DATA;
	int max_recepients = 1;
	int needs_message = 1;

	*c_sock = init_client_socket(fd, buffer_size, state, max_recepients, needs_message);

	struct sockaddr_in sock_address;

    sock_address.sin_family = AF_INET;
    sock_address.sin_addr.s_addr = INADDR_ANY;
    sock_address.sin_port = htons( SERVER_PORT );

    c_sock->message->from = get_mail("<testfrom@test.ru>");
    c_sock->message->to[0] = get_mail("<testfrom@test.ru>");
    c_sock->message->body = (char *)malloc(1);
    c_sock->message->body[0] = '\0';
    c_sock->message->body_length = 0;
    c_sock->buffer = msg_buffer;

    int err = 0;
    err = handle_TEXT(c_sock,buffer_output,"./test_mail_dir/");
	
	CU_ASSERT_EQUAL(c_sock->fd,0);
	CU_ASSERT_EQUAL(c_sock->state,SOCKET_STATE_WRITING_DATA);
	CU_ASSERT_NOT_EQUAL(strstr(c_sock->message->body, msg_buffer),0);
	CU_ASSERT_EQUAL(err,0);
	free(c_sock);
}

TEST_FUNCT(test_full_session_correct) {
	char *msg_buffer = "HELO myserver.com";
	char *msg_buffer2 = "MAIL <test@test.ru>";
	char *msg_buffer3 = "RCPT <test@test.ru>";
	char *msg_buffer4 = "DATA ";
	char *msg_buffer5 = ".";
	char *msg_buffer6 = "QUIT ";
	char *check_mail = "test@test.ru";
	char buffer_output[SERVER_BUFFER_SIZE];
	struct client_socket *c_sock = (struct client_socket *)malloc(sizeof(struct client_socket));
	int fd = 0;

	int buffer_size = 2048;
	int state = SOCKET_STATE_INIT;
	int max_recepients = 1;
	int needs_message = 1;

	*c_sock = init_client_socket(fd, buffer_size, state, max_recepients, needs_message);

	struct sockaddr_in sock_address;

    sock_address.sin_family = AF_INET;
    sock_address.sin_addr.s_addr = INADDR_ANY;
    sock_address.sin_port = htons( SERVER_PORT );

    int err = 0;
    err = handle_HELO(c_sock, msg_buffer, buffer_output, &sock_address);
    CU_ASSERT_EQUAL(err,0);
	
	CU_ASSERT_EQUAL(c_sock->state,SOCKET_STATE_WAIT);
	err = handle_MAIL(c_sock, msg_buffer2, buffer_output, &sock_address);
	CU_ASSERT_EQUAL(strcmp(HEADER_250_OK, buffer_output),0);
	CU_ASSERT_EQUAL(err,0);
	
	CU_ASSERT_EQUAL(c_sock->state,SOCKET_STATE_MAIL_CREATED_NO_RECEPIENTS);
	err = handle_RCPT(c_sock, msg_buffer3, buffer_output, &sock_address);
	CU_ASSERT_EQUAL(strcmp(HEADER_250_OK, buffer_output),0);
	CU_ASSERT_EQUAL(strcmp(c_sock->message->from, check_mail),0);
	CU_ASSERT_EQUAL(strcmp(c_sock->message->to[0], check_mail),0);
	
	CU_ASSERT_EQUAL(c_sock->state,SOCKET_STATE_RECEPIENTS_SET);
	CU_ASSERT_EQUAL(err,0);

	err = handle_DATA(c_sock, msg_buffer4, buffer_output, &sock_address);
	CU_ASSERT_EQUAL(c_sock->state,SOCKET_STATE_WRITING_DATA);
	CU_ASSERT_EQUAL(strcmp(HEADER_354_START, buffer_output),0);
	CU_ASSERT_EQUAL(c_sock->input_message,1);
	CU_ASSERT_EQUAL(err,0);


	c_sock->buffer = ".";
	err = handle_TEXT(c_sock,buffer_output,"./test_mail_dir/");
	
	CU_ASSERT_EQUAL(c_sock->fd,0);
	CU_ASSERT_EQUAL(c_sock->state,SOCKET_STATE_WAIT);
	CU_ASSERT_EQUAL(err,0);

	err = handle_QUIT(c_sock, msg_buffer, buffer_output, &sock_address);
	
	CU_ASSERT_EQUAL(c_sock->fd,0);
	CU_ASSERT_EQUAL(c_sock->state,SOCKET_STATE_CLOSED);
	CU_ASSERT_EQUAL(c_sock->input_message,0);
	CU_ASSERT_EQUAL(err,1);

	free(c_sock);
}

int add_to_suit_smtp_test(CU_pSuite suite) {
    if (suite) {
        ADD_SUITE_TEST(suite, create_client_socket_no_message)
        ADD_SUITE_TEST(suite, create_client_socket_has_message)
        ADD_SUITE_TEST(suite, handle_helo_state_init)
        ADD_SUITE_TEST(suite, handle_helo_state_incorrect)
        ADD_SUITE_TEST(suite, handle_ehlo_state_correct)
        ADD_SUITE_TEST(suite, handle_ehlo_state_incorrect)
        ADD_SUITE_TEST(suite, handle_mail_correct)
        ADD_SUITE_TEST(suite, handle_mail_incorrect_command)
        ADD_SUITE_TEST(suite, handle_mail_incorrect_state)
        ADD_SUITE_TEST(suite, handle_rcpt_correct)
        ADD_SUITE_TEST(suite, handle_rcpt_incorrect_command)
        ADD_SUITE_TEST(suite, handle_rcpt_incorrect_state)
        ADD_SUITE_TEST(suite, handle_rcpt_exceeded_recepients)
        ADD_SUITE_TEST(suite, handle_rset_correct_no_recepients)
        ADD_SUITE_TEST(suite, handle_rset_correct_has_recepients)
        ADD_SUITE_TEST(suite, handle_rset_incorrect_state)
        ADD_SUITE_TEST(suite, test_prepare_client_before_getting_data_no_reset)
        ADD_SUITE_TEST(suite, handle_data_correct)
        ADD_SUITE_TEST(suite, handle_data_incorrect_state)
        ADD_SUITE_TEST(suite, handle_quit_correct)
        ADD_SUITE_TEST(suite, handle_noop_correct)
        ADD_SUITE_TEST(suite, handle_not_implemented_correct)
        ADD_SUITE_TEST(suite, handle_text_not_dot)
        ADD_SUITE_TEST(suite, test_full_session_correct)
    }
    return CU_get_error();
}